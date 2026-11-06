# Make the PCB follow the schematic: footprint path (= symbol uuid), value, BOM flag, fields, and pad nets
# from the exported netlist. usage: python3.11 sync_paths.py <sch> <netlist> <pcb>
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sexp import *
import pcbnew as P
t = parse(open(sys.argv[1], encoding='utf8').read())
sy = {prop_val(s, 'Reference'): s for s in kids(t, 'symbol') if not prop_val(s, 'Reference').startswith('#')}
nl = parse(open(sys.argv[2], encoding='utf8').read())
padnet = {}
for n in kids(kid(nl, 'nets'), 'net'):
    for node in kids(n, 'node'): padnet[(kid(node, 'ref')[1], kid(node, 'pin')[1])] = kid(n, 'name')[1]
b = P.LoadBoard(sys.argv[3])
def net(name):
    x = b.FindNet(name)
    if x is None: x = P.NETINFO_ITEM(b, name); b.Add(x)
    return x
seen = set()
for f in list(b.GetFootprints()):
    r = f.GetReference(); s = sy.get(r)
    if s is None: print('no symbol for', r); continue
    seen.add(r)
    f.SetPath(P.KIID_PATH('/' + kid(s, 'uuid')[1]))
    f.SetValue(prop_val(s, 'Value'))
    f.SetExcludedFromBOM(kid(s, 'in_bom')[1] == 'no')
    for name in ('Datasheet', 'Description'):
        fld = f.GetField(name)
        if fld is not None: fld.SetText(prop_val(s, name) or '')
    f.SetSheetname('/'); f.SetSheetfile(os.path.basename(sys.argv[1]))
    for p in f.Pads():
        n = padnet.get((r, p.GetNumber()))
        if n is not None and p.GetNetname() != n:
            if not p.GetNetname().startswith('unconnected') and p.GetNetname(): print(f'net {r}.{p.GetNumber()}: {p.GetNetname()} -> {n}')
            p.SetNet(net(n))
for r in sorted(set(sy) - seen): print('no footprint for', r)
P.SaveBoard(sys.argv[3], b)
