# DI recordings

Place mono WAV recordings of a dry guitar signal here (straight from the instrument input of your audio
interface, no processing). 44.1 or 48 kHz, 24-bit or float, 5–30 s each. Suggested set:

- `di_chords.wav` — open and power chords, strummed
- `di_palm_mutes.wav` — palm-muted riffing on the low strings
- `di_single_notes.wav` — single-note lines with bends and vibrato
- `di_dynamics.wav` — the same riff played softly, then hard (tests blocking/bias shift)

Level convention: the engine assumes 0 dBFS = +12 dBu at the jack (see DECISIONS D-010). Record with the
interface's instrument gain at its lowest practical setting so recordings stay close to that convention;
the harness sets test levels explicitly, so small offsets do not affect verification.

Free alternative with a CC BY 4.0 licence: the Guitar-TECHS dataset (guitar-techs.github.io) contains DI
recordings (48 kHz, 32-bit float) alongside miked-amp recordings. Player 01 (Les Paul type, bridge
pickup) covers chords and palm mutes; Player 03 has 12 musical excerpts. Credit the dataset if its files
are committed. Its interface gain settings are undocumented, so treat its absolute levels as unknown.

WAV files in this folder are git-ignored by default.
