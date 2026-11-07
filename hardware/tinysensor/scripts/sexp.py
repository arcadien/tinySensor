# Minimal KiCad S-expression reader/writer: lists of atoms; quoted strings kept as Q instances.
import re

class Q(str):
    """A quoted string atom."""

TOK = re.compile(r'\s*(?:(\()|(\))|"((?:[^"\\]|\\.)*)"|([^\s()"]+))', re.S)

def parse(text):
    stack, cur = [], []
    for m in TOK.finditer(text):
        o, c, q, a = m.groups()
        if o: stack.append(cur); cur = []
        elif c: done = cur; cur = stack.pop(); cur.append(done)
        elif q is not None: cur.append(Q(q.replace('\\"', '"').replace('\\\\', '\\')))
        elif a is not None: cur.append(a)
    assert not stack
    return cur[0]

def esc(s): return '"' + s.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n') + '"'

def dump(x, ind=0):
    if not isinstance(x, list): return esc(x) if isinstance(x, Q) else str(x)
    if all(not isinstance(e, list) for e in x): return '(' + ' '.join(dump(e) for e in x) + ')'
    head = []
    i = 0
    while i < len(x) and not isinstance(x[i], list): head.append(dump(x[i])); i += 1
    out = '(' + ' '.join(head)
    for e in x[i:]: out += '\n' + '\t' * (ind + 1) + dump(e, ind + 1)
    return out + '\n' + '\t' * ind + ')'

def kids(x, name): return [e for e in x if isinstance(e, list) and e and e[0] == name]
def kid(x, name):
    k = kids(x, name); return k[0] if k else None
def prop(sym, name):
    for p in kids(sym, 'property'):
        if p[1] == name: return p
def prop_val(sym, name):
    p = prop(sym, name); return p[2] if p else None
