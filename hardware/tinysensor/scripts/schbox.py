import sys, os; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sexp import *
def items_in(t, x1, y1, x2, y2):
    out = []
    def inb(x, y): return x1 <= x <= x2 and y1 <= y <= y2
    for e in t:
        if not isinstance(e, list): continue
        if e[0] in ('wire', 'polyline'):
            pts = [(float(p[1]), float(p[2])) for p in kid(e, 'pts')[1:]]
            if any(inb(*p) for p in pts) or (len(pts)==2 and min(p[0] for p in pts)<=x2 and max(p[0] for p in pts)>=x1 and min(p[1] for p in pts)<=y2 and max(p[1] for p in pts)>=y1): out.append((e[0], pts))
        elif e[0] in ('symbol', 'label', 'global_label', 'junction', 'no_connect', 'text'):
            a = kid(e, 'at')
            if a and inb(float(a[1]), float(a[2])): out.append((e[0], prop_val(e, 'Reference') if e[0] == 'symbol' else (e[1] if e[0] in ('label', 'text') else ''), a[1:3]))
    return out
if __name__ == '__main__':
    t = parse(open(sys.argv[1], encoding='utf8').read())
    for b in sys.argv[2:]:
        x1, y1, x2, y2 = map(float, b.split(','))
        r = items_in(t, x1, y1, x2, y2); print(b, 'FREE' if not r else r[:6])
