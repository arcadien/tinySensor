# v3.0: J2 becomes a plain 1x03 header (1 +BATT, 2 GND, 3 VCC) on the old module pads; the 3.3 V module
# (TPS63020 buck-boost, or a 5 V supply later) is wired off-board. usage: python3.11 v3_j2_header.py <pcb> <footprints dir>
import sys, pcbnew as P
b = P.LoadBoard(sys.argv[1])
old = b.FindFootprintByReference('J2')
pads = {p.GetNumber(): (p.GetPosition(), p.GetNet()) for p in old.Pads()}
lib, name = 'Connector_PinHeader_2.54mm', 'PinHeader_1x03_P2.54mm_Vertical'
f = P.FootprintLoad(f'{sys.argv[2]}/{lib}.pretty', name)
f.SetFPID(P.LIB_ID(lib, name)); f.SetReference('J2'); f.SetValue(old.GetValue()); f.SetPath(old.GetPath())
f.SetOrientationDegrees(90)  # pads along +X
b.Add(f)
f.SetPosition(f.GetPosition() + pads['1'][0] - [p for p in f.Pads() if p.GetNumber() == '1'][0].GetPosition())
for p in f.Pads():
    assert (p.GetPosition() - pads[p.GetNumber()][0]).EuclideanNorm() < P.FromMM(0.01), p.GetNumber()
    p.SetNet(pads[p.GetNumber()][1])
f.Reference().SetPosition(pads['2'][0] - P.VECTOR2I(0, P.FromMM(2.2)))  # clear of the x = 147 cut line
f.Reference().SetTextAngleDegrees(0)
b.Remove(old)
P.ZONE_FILLER(b).Fill(b.Zones())
P.SaveBoard(sys.argv[1], b)
print('J2 ->', name)
