"""Presets: knob settings plus the signals to run (VERIFICATION.md §4)."""
from dataclasses import dataclass, field

from . import config, signals


@dataclass(frozen=True)
class Preset:
    id: str
    knobs: dict = field(default_factory=dict)       # engine names; unspecified knobs use the defaults
    description: str = ''

    def all_knobs(self):
        return {**config.DEFAULT_KNOBS, **self.knobs}

    def signals(self, block):
        """Signal list for this preset on the given block."""
        if self.id == 'DEFAULT' or self.id.startswith(('VOL', 'PRES')):
            return signals.standard_set(block.levels)
        if self.id == 'CLEAN':
            return [signals.sine(1000.0, 0.025)]
        if self.id in ('CRUNCH', 'DRIVE'):
            return [signals.sine(220.0, 0.3)] + signals.di_signals()
        if self.id.startswith('TONE-'):
            return [signals.sweep(block.levels.get('small') or block.levels['nominal'], 'SWP-lo')]
        if self.id == 'PIPE':
            return [signals.sine(1000.0, block.levels['nominal'], seconds=0.050)]
        raise KeyError(self.id)


def _tone_presets():
    out = {}
    for t in (0, 0.5, 1):
        for m in (0, 0.5, 1):
            for b in (0, 0.5, 1):
                pid = f'TONE-T{t:g}M{m:g}B{b:g}'
                out[pid] = Preset(pid, {'treble': t, 'middle': m, 'bass': b},
                                  f'Treble {t:g}, Middle {m:g}, Bass {b:g}; SWP-lo')
    return out


PRESETS = {
    'DEFAULT': Preset('DEFAULT', {}, 'Default knobs; standard signal set at the block levels'),
    'VOL0.2': Preset('VOL0.2', {'preampVolume': 0.2}, 'Preamp Volume 0.2; standard signal set (M2)'),
    'VOL1.0': Preset('VOL1.0', {'preampVolume': 1.0}, 'Preamp Volume 1.0; standard signal set (M2)'),
    'CLEAN': Preset('CLEAN', {'preampVolume': 0.2, 'master': 1.0}, '25 mV peak 1 kHz (about 100 W in SPICE)'),
    'CRUNCH': Preset('CRUNCH', {'preampVolume': 0.6, 'master': 0.3}, '300 mV peak 220 Hz, DI clips'),
    'DRIVE': Preset('DRIVE', {'preampVolume': 1.0, 'master': 0.3}, '300 mV peak 220 Hz, DI clips'),
    'PRES0': Preset('PRES0', {'presence': 0.0}, 'Presence 0; standard signal set (M4 J5 study)'),
    'PRES1': Preset('PRES1', {'presence': 1.0}, 'Presence 1; standard signal set (M4 J5 study)'),
    'PIPE': Preset('PIPE', {}, 'Pipeline smoke test: one 50 ms nominal 1 kHz sine'),
    **_tone_presets(),
}
