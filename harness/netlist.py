"""Minimal parser for the reference netlist: element lines, their nodes, and directives (VERIFICATION §2).
Only the constructs used in reference/spice/jcm800_2203.cir are supported; anything else raises."""
from dataclasses import dataclass

TWO_NODE = set('RCLVIDBE')


@dataclass(frozen=True)
class Element:
    name: str          # as written, e.g. 'RVR1a'
    line: str          # original line (continuations joined)
    nodes: tuple       # lowercase node names

    @property
    def key(self):
        return self.name.lower()


@dataclass(frozen=True)
class Netlist:
    title: str
    directives: tuple  # '.include', '.param', '.func' lines in order
    elements: tuple

    def by_name(self):
        return {e.key: e for e in self.elements}

    def nodes(self):
        return sorted({n for e in self.elements for n in e.nodes} - {'0'})


def _logical_lines(text):
    out = []
    for raw in text.splitlines():
        if raw.startswith('+') and out:
            out[-1] += ' ' + raw[1:].strip()
        else:
            out.append(raw.rstrip())
    return out


def parse(text):
    lines = _logical_lines(text)
    title, body = lines[0], lines[1:]
    directives, elements = [], []
    for line in body:
        s = line.strip()
        if not s or s.startswith('*'):
            continue
        if s.startswith('.'):
            word = s.split()[0].lower()
            if word in ('.include', '.param', '.func'):
                directives.append(s)
                continue
            if word == '.end':
                break
            raise ValueError(f'unsupported directive: {s}')
        tok = s.split()
        kind = tok[0][0].upper()
        if kind in TWO_NODE:
            nodes = tok[1:3]
        elif kind == 'X':
            nodes = [t for t in tok[1:-1] if '=' not in t]
        elif kind == 'K':
            nodes = []
        else:
            raise ValueError(f'unsupported element: {s}')
        elements.append(Element(tok[0], s, tuple(n.lower() for n in nodes)))
    return Netlist(title, tuple(directives), tuple(elements))
