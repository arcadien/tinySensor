# Build the v3.0 PCB from the v2.2 board (base/), applying the review fixes.
# usage: python3.11 make_pcb.py <base.kicad_pcb> <out.kicad_pcb>
import sys, math, pcbnew as P

FP = sys.argv[0].rsplit('/', 2)[0] + '/squashfs-root/usr/share/kicad/footprints/'
b = P.LoadBoard(sys.argv[1])
mm, MM = P.ToMM, P.FromMM
def pt(x, y): return P.VECTOR2I(MM(x), MM(y))
def close(a, x, y, tol=0.03): return abs(mm(a.x) - x) < tol and abs(mm(a.y) - y) < tol

def net(name):
    n = b.FindNet(name)
    if n is None:
        n = P.NETINFO_ITEM(b, name); b.Add(n)
    return n

# shortcut: pcbnew 10 proxies from GetFootprints() break after a track Remove(), so look them up once
FPS = {}
for f in b.GetFootprints(): FPS.setdefault(f.GetReference(), []).append(f)
def fp(ref): return FPS[ref][0]

def seg(x1, y1, x2, y2, netname, layer, w=0.25):
    t = P.PCB_TRACK(b); t.SetStart(pt(x1, y1)); t.SetEnd(pt(x2, y2))
    t.SetWidth(MM(w)); t.SetLayer(layer); t.SetNet(net(netname)); b.Add(t); return t

def via(x, y, netname):
    v = P.PCB_VIA(b); v.SetPosition(pt(x, y)); v.SetWidth(MM(0.8)); v.SetDrill(MM(0.4))
    v.SetNet(net(netname)); b.Add(v)

def find_seg(x1, y1, x2, y2):
    for t in b.GetTracks():
        if t.Type() != P.PCB_TRACE_T: continue
        s, e = t.GetStart(), t.GetEnd()
        if (close(s, x1, y1) and close(e, x2, y2)) or (close(s, x2, y2) and close(e, x1, y1)): return t
    raise SystemExit(f'segment not found {x1},{y1}-{x2},{y2}')

# shortcut: removals are deferred to the end, pcbnew 10 SWIG proxies break after an early Remove()
DOOMED = []
def delete_seg(*c): DOOMED.append(find_seg(*c))

def place(lib, name, ref, value, x, y, rot, layer_back, nets):
    f = P.FootprintLoad(FP + lib + '.pretty', name)
    f.SetReference(ref); f.SetValue(value)
    b.Add(f); FPS.setdefault(ref, []).insert(0, f)
    f.SetPosition(pt(x, y)); f.SetOrientationDegrees(rot)
    if layer_back: f.Flip(f.GetPosition(), P.FLIP_DIRECTION_LEFT_RIGHT)
    for p in f.Pads(): p.SetNet(net(nets[p.GetNumber()]))
    return f

def pad_xy(f, n):
    q = [p for p in f.Pads() if p.GetNumber() == n][0].GetPosition(); return mm(q.x), mm(q.y)

F, B = P.F_Cu, P.B_Cu
def link(f, n, x, y, netname, layer=B, w=0.25):
    px, py = pad_xy(f, n); seg(x, y, px, py, netname, layer, w)

# --- R5/P10: LED D1 reversed. Rotate 180 deg: pads swap places, then K->GND, A->R1
d1 = fp('D1'); d1.Rotate(pt(158.00, 80.29), P.EDA_ANGLE(180, P.DEGREES_T))  # about the pads' midpoint
for p in d1.Pads(): p.SetNet(net('GND' if p.GetNumber() == '1' else 'Net-(D1-A)'))
for t in b.GetTracks():
    if t.GetNetname() == 'Net-(D1-K)': t.SetNet(net('Net-(D1-A)'))
for p in fp('R1').Pads():
    if p.GetNetname() == 'Net-(D1-K)': p.SetNet(net('Net-(D1-A)'))

# --- R1: PA1 through a 1M/1M divider + 100n instead of raw +BATT.
# R7 goes in series in the B.Cu +BATT track west of U1; the track part on the U1 side becomes BAT_SENSE
delete_seg(174.00, 62.50, 174.00, 62.24)  # F.Cu stub on pin 12
delete_seg(170.19, 66.31, 151.85, 66.31)
find_seg(174.00, 62.50, 170.19, 66.31).SetNet(net('/BAT_SENSE'))
u1 = fp('U1')
for p in u1.Pads():
    if p.GetNumber() == '12': p.SetNet(net('/BAT_SENSE'))
def by_pos(f, axis, low_net, high_net):
    ps = sorted(f.Pads(), key=lambda p: p.GetPosition().x if axis == 'x' else p.GetPosition().y)
    ps[0].SetNet(net(low_net)); ps[1].SetNet(net(high_net))
    return [(mm(p.GetPosition().x), mm(p.GetPosition().y)) for p in ps]
r7 = place('Resistor_SMD', 'R_0805_2012Metric', 'R7', '1M', 157.00, 66.31, 0, True, {'1': '+BATT', '2': '/BAT_SENSE'})
(wx, wy), (ex, ey) = by_pos(r7, 'x', '+BATT', '/BAT_SENSE')
seg(151.85, 66.31, wx, wy, '+BATT', B, 0.3); seg(ex, ey, 170.19, 66.31, '/BAT_SENSE', B, 0.25)
c2gnd = (165.50, 64.00)
for ref, val, x in (('C7', '100n', 163.80), ('R8', '1M', 167.20)):
    lib, name = ('Capacitor_SMD', 'C_0805_2012Metric') if ref[0] == 'C' else ('Resistor_SMD', 'R_0805_2012Metric')
    f = place(lib, name, ref, val, x, 64.60, 90, True, {'1': 'GND', '2': '/BAT_SENSE'})
    (tx, ty), (bx, by) = by_pos(f, 'y', 'GND', '/BAT_SENSE')
    for p in f.Pads(): p.SetLocalZoneConnection(P.ZONE_CONNECTION_FULL)
    seg(tx, ty, *c2gnd, 'GND', B); seg(bx, by, bx, 66.31, '/BAT_SENSE', B)

# --- R2: reverse polarity on RAW IN / 18650 side: P-MOSFET Q1 in the +BATT feed
# upstream (J13, 18650 holder) becomes /RAW_IN; downstream stays +BATT
delete_seg(149.53, 65.09, 149.53, 75.85)
q1 = place('Package_TO_SOT_SMD', 'SOT-23', 'Q1', 'AO3401A', 149.90, 71.00, -90, False,
           {'1': 'GND', '2': '+BATT', '3': '/RAW_IN'})
gx, gy = pad_xy(q1, '1'); sx, sy = pad_xy(q1, '2'); dx, dy = pad_xy(q1, '3')
print('Q1 G', gx, gy, 'S', sx, sy, 'D', dx, dy)
seg(149.53, 65.09, sx, sy, '+BATT', F)
seg(dx, dy, 149.53, 75.85, '/RAW_IN', F)
via(gx, gy + 1.2 if gy > dy else gy - 1.2, 'GND'); seg(gx, gy, gx, gy + 1.2 if gy > dy else gy - 1.2, 'GND', F)
for c in [(149.53, 75.85, 151.67, 77.99), (151.67, 77.99, 148.35, 81.32), (148.35, 81.32, 148.35, 88.16),
          (148.35, 88.16, 135.65, 88.16), (135.65, 88.16, 124.72, 99.08), (123.96, 98.57, 125.96, 98.57)]:
    find_seg(*c).SetNet(net('/RAW_IN'))
for ref, n in [('J13', '1'), ('REF**', '1')]:
    for p in fp(ref).Pads():
        if p.GetNumber() == n: p.SetNet(net('/RAW_IN'))

# --- P2: 18650 holder named BT1, minus on GND
bt = fp('REF**'); bt.SetReference('BT1'); bt.SetValue('18650')
for p in bt.Pads():
    if p.GetNumber() == '2': p.SetNet(net('GND')); p.SetLocalZoneConnection(P.ZONE_CONNECTION_THERMAL)
seg(196.72, 99.08, 196.72, 96.00, 'GND', B, 0.5)  # the pour does not reach the pad by itself

# --- P3: duplicated MH1 -> MH3; JP5 (GND-GND) removed
for f in FPS['MH1']:
    if close(f.GetPosition(), 203.75, 105.51, 0.1): f.SetReference('MH3')
DOOMED.append(fp('JP5'))

# --- P4/R11: SDO selector. Pin 6 of both sensor connectors = SDO -> JP6 (1 GND / 3 MISO, 1-2 bridged)
# pin 5 = CSB, left open (breakout pull-up keeps I2C)
DOOMED.append(fp('JP6'))
delete_seg(189.35, 81.44, 189.35, 83.91)  # MISO to I2C_SPI pin 5 (CSB)
for ref in ('I2C', 'I2C_SPI'):
    for p in fp(ref).Pads():
        if p.GetNumber() == '6': p.SetNet(net('/SDO_SEL'))
        if p.GetNumber() == '5': p.SetNet(net(''))
jp6 = place('Jumper', 'SolderJumper-3_P1.3mm_Bridged12_RoundedPad1.0x1.5mm', 'JP6', 'SDO', 183.50, 81.30, 0, True,
            {'1': 'GND', '2': '/SDO_SEL', '3': '/MISO'})
for n in '123': print('JP6', n, pad_xy(jp6, n))
x3, y3 = pad_xy(jp6, '3'); x2, y2 = pad_xy(jp6, '2')
miso = find_seg(189.35, 79.99, 178.68, 79.99)  # was ending on the old JP6: stop it above pad 3
for get, put in ((miso.GetStart, miso.SetStart), (miso.GetEnd, miso.SetEnd)):
    if close(get(), 189.35, 79.99): put(pt(x3, 79.99))
seg(x3, 79.99, x3, y3, '/MISO', B)
seg(x2, y2, x2, 83.00, '/SDO_SEL', B); seg(x2, 83.00, 186.85, 83.91, '/SDO_SEL', B)
seg(186.85, 77.11, 186.85, 83.91, '/SDO_SEL', F)

# --- P1: GND of the I2C connector (and ISP) island joined to the main pour
seg(196.85, 77.11, 196.85, 83.91, 'GND', F)

# --- P13: 100n at the radio, R7(backlog): 10k RESET pull-up
c8 = place('Capacitor_SMD', 'C_0805_2012Metric', 'C8', '100n', 195.30, 57.95, 90, True, {'1': 'VCC', '2': 'GND'})
link(c8, '1', 193.00, 59.40, 'VCC')
r9 = place('Resistor_SMD', 'R_0805_2012Metric', 'R9', '10k', 162.90, 69.60, 180, True, {'1': '/RESET', '2': 'VCC'})
link(r9, '1', 160.03, 68.09, '/RESET'); link(r9, '2', 165.50, 69.00, 'VCC')

# --- P8: C6 (0.1u under the DC-DC module, was a THT tantalum on the back) -> 0805 on top
DOOMED.append(fp('C6'))
c6 = place('Capacitor_SMD', 'C_0805_2012Metric', 'C6', '100n', 164.09, 56.18, 90, False,
           {'1': 'Net-(J7-Pin_3)', '2': '/SENSOR_VCC'})
for n in '12': print('C6', n, pad_xy(c6, n))
vx, vy = pad_xy(c6, '2'); via(164.09, 54.63, '/SENSOR_VCC'); seg(164.09, 54.63, vx, vy, '/SENSOR_VCC', F)
ox, oy = pad_xy(c6, '1');
if abs(oy - 57.13) > 0.05: seg(ox, oy, 164.09, 57.13, 'Net-(J7-Pin_3)', F)

# --- P17: C5 (1n, unpolarised) on a ceramic disc footprint
c5 = fp('C5'); pos, rot = c5.GetPosition(), c5.GetOrientationDegrees(); nets5 = {p.GetNumber(): p.GetNetname() for p in c5.Pads()}
DOOMED.append(c5)
place('Capacitor_THT', 'C_Disc_D3.0mm_W1.6mm_P2.50mm', 'C5', '1n', mm(pos.x), mm(pos.y), rot, False, nets5)

# --- P11: M2.5 clearance holes; P16: wider drills for 2.54 headers in the EH footprints
for f in [f for l in FPS.values() for f in l]:
    if f.GetReference().startswith('MH'):
        for p in f.Pads(): p.SetDrillSize(pt(2.7, 2.7)); p.SetSize(pt(2.7, 2.7))
    if f.GetReference() in ('I2C', 'I2C_SPI'):
        for p in f.Pads(): p.SetDrillSize(pt(1.05, 1.05))

# --- P14: U1 GND pin onto the pour
for p in u1.Pads():
    if p.GetNumber() == '14': p.SetLocalZoneConnection(P.ZONE_CONNECTION_INHERITED)

# --- P17: junk stubs
for t in list(b.GetTracks()):
    s, e = t.GetStart(), t.GetEnd()
    if t.Type() == P.PCB_TRACE_T and t.GetLength() < MM(0.6) and (
            (t.GetNetname() == 'GND' and close(s, 202.8, 97.0, 0.6)) or (t.GetNetname() == 'VCC' and close(s, 162.86, 61.74, 0.4))):
        print('drop stub', t.GetNetname(), mm(s.x), mm(s.y)); DOOMED.append(t)

# --- P9: copper texts -> silkscreen, version, pin labels
for d in b.GetDrawings():
    if d.GetClass() == 'PCB_TEXT':
        if d.GetLayer() == F: d.SetLayer(P.F_SilkS)
        elif d.GetLayer() == B: d.SetLayer(P.B_SilkS)
        if d.GetText() == 'TinySensor v2.2': d.SetText('TinySensor v3.0')
        if d.GetText().startswith('I²C/SPI'): d.SetText('I²C 2')
        if d.GetTextSize().y < MM(0.8): d.SetTextSize(pt(0.8, 0.8)); d.SetTextThickness(MM(0.12))
def silk(text, x, y, size=1.0):
    t = P.PCB_TEXT(b); t.SetText(text); t.SetPosition(pt(x, y)); t.SetLayer(P.F_SilkS)
    t.SetTextSize(pt(size, size)); t.SetTextThickness(MM(0.15)); b.Add(t)
silk('+', 151.67, 74.40, 1.5)  # above pad 1 of RAW IN
for lab, y in (('G', 56.86), ('V', 59.40), ('D', 61.94)): silk(lab, 190.2, y)
j5 = fp('J5')
for p in j5.Pads():
    x, y = mm(p.GetPosition().x), mm(p.GetPosition().y)
    silk({'1': 'V', '2': 'G', '3': 'Q'}[p.GetNumber()], x, y + 2.4)
silk('1 seule source batterie : CR2032, 18650 ou RAW IN', 160.0, 110.8, 0.9)

# --- P5: no copper pour along the cut lines of the mushroom variant (tracks still allowed)
def keepout(x1, y1, x2, y2):
    z = P.ZONE(b); z.SetIsRuleArea(True); z.SetDoNotAllowZoneFills(True)
    z.SetDoNotAllowTracks(False); z.SetDoNotAllowVias(False); z.SetDoNotAllowPads(False); z.SetDoNotAllowFootprints(False)
    ls = P.LSET(); ls.AddLayer(F); ls.AddLayer(B); z.SetLayerSet(ls)
    o = z.Outline(); o.NewOutline()
    for x, y in ((x1, y1), (x2, y1), (x2, y2), (x1, y2)): o.Append(MM(x), MM(y))
    z.SetZoneName('cut line'); b.Add(z)
keepout(146.25, 47.5, 147.75, 87.75)
keepout(146.25, 86.25, 207.5, 87.75)
# the bands split the pour: GND bridges across them
seg(145.00, 70.00, 149.00, 70.00, 'GND', B, 0.5); seg(165.00, 85.00, 165.00, 89.00, 'GND', B, 0.5)

# --- P6: copper to edge 0.5 mm
ds = b.GetDesignSettings(); ds.m_CopperEdgeClearance = MM(0.5)

tb = b.GetTitleBlock(); tb.SetTitle('TinySensor'); tb.SetRevision('3.0'); tb.SetDate('2026-11-06')

for o in DOOMED: b.Remove(o)
P.ZONE_FILLER(b).Fill(b.Zones())
P.SaveBoard(sys.argv[2], b)
print('saved', sys.argv[2])
