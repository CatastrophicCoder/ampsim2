#include "amp2203/Supply.h"

#include "solver/MultiportNewton.h"

namespace amp2203 {

void Supply::continuous(Mat& A, std::array<Vec, kNodes>& B, Vec& c) const noexcept {
    // dx/dt = A·x + Σ B_k·I_k + c (WDF_DESIGN §7), x = (v_ct, i_L, v_scr, v_X, v_A)
    A = {};
    B = {};
    c = {};
    A[0][0] = -1.0 / (Rsrc * C23);
    A[0][1] = -1.0 / C23;
    c[0] = Vht / (Rsrc * C23);
    A[1][0] = 1.0 / Lch;
    A[1][1] = -Rch / Lch;
    A[1][2] = -1.0 / Lch;
    A[2][1] = 1.0 / C22;
    A[2][2] = -1.0 / (R29 * C22);
    A[2][3] = 1.0 / (R29 * C22);
    A[3][2] = 1.0 / (R29 * C21b);
    A[3][3] = -1.0 / (R29 * C21b) - 1.0 / (R8 * C21b);
    A[3][4] = 1.0 / (R8 * C21b);
    A[4][3] = 1.0 / (R8 * C21a);
    A[4][4] = -1.0 / (R8 * C21a);
    B[iCt][0] = -1.0 / C23;
    B[iScreen][2] = -1.0 / C22;
    B[iBpX][3] = -1.0 / C21b;
    B[iBpA][4] = -1.0 / C21a;
}

void Supply::prepare(double sampleRate) {
    const double T = 1.0 / sampleRate;
    Mat A;
    std::array<Vec, kNodes> B;
    Vec c;
    continuous(A, B, c);
    // (I − T/2·A)·x[n+1] = (I + T/2·A)·x[n] + T·(B·u + c), u held over the step
    for (int col = 0; col < kStates + kNodes + 1; ++col) {
        solver::Mat<kStates> M;
        for (int i = 0; i < kStates; ++i)
            for (int j = 0; j < kStates; ++j)
                M[i][j] = (i == j ? 1.0 : 0.0) - 0.5 * T * A[i][j];
        solver::Vec<kStates> r;
        for (int i = 0; i < kStates; ++i)
            r[i] = col < kStates ? (i == col ? 1.0 : 0.0) + 0.5 * T * A[i][col]
                                 : T * (col < kStates + kNodes ? B[col - kStates][i] : c[i]);
        solver::solveLinear<kStates>(M, r);
        for (int i = 0; i < kStates; ++i) {
            if (col < kStates)
                Ad_[i][col] = r[i];
            else if (col < kStates + kNodes)
                Bd_[col - kStates][i] = r[i];
            else
                cd_[i] = r[i];
        }
    }
}

void Supply::reset(const Currents& load) noexcept {
    // A·x = −(B·u + c)
    Mat A;
    std::array<Vec, kNodes> B;
    Vec c;
    continuous(A, B, c);
    solver::Mat<kStates> M;
    solver::Vec<kStates> r;
    for (int i = 0; i < kStates; ++i) {
        for (int j = 0; j < kStates; ++j)
            M[i][j] = A[i][j];
        r[i] = -c[i];
        for (int k = 0; k < kNodes; ++k)
            r[i] -= B[k][i] * load[k];
    }
    solver::solveLinear<kStates>(M, r);
    for (int i = 0; i < kStates; ++i)
        x_[i] = r[i];
    load_ = load;
}

void Supply::process(const Currents& load) noexcept {
    Vec next{};
    for (int i = 0; i < kStates; ++i) {
        double acc = cd_[i];
        for (int j = 0; j < kStates; ++j)
            acc += Ad_[i][j] * x_[j];
        for (int k = 0; k < kNodes; ++k)
            acc += Bd_[k][i] * load[k];
        next[i] = acc;
    }
    x_ = next;
    load_ = load;
}

}  // namespace amp2203
