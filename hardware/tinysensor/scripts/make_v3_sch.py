# Build the v3.0 schematic from the v2.2 one (already upgraded to the KiCad 10 format).
# usage: python3 make_sch.py <in.kicad_sch> <out.kicad_sch> <stock symbols dir> <pcb pads json>
import sys, os, json, uuid, copy
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sexp import *
from schbox import items_in

src, dst, SYMS, PADS = sys.argv[1:5]
t = parse(open(src, encoding='utf8').read())
pads = json.load(open(PADS))
ROOT = kid(t, 'uuid')[1]
LIBS = kid(t, 'lib_symbols')
def uid(): return Q(str(uuid.uuid4()))
def at(x, y, a=0): return ['at', f'{x:g}', f'{y:g}', f'{a:g}']
def font(): return ['effects', ['font', ['size', '1.27', '1.27']]]
def syms(): return kids(t, 'symbol')
def sym(ref): return [s for s in syms() if prop_val(s, 'Reference') == ref][0]
def set_prop(s, name, value): prop(s, name)[2] = Q(value)

# --- stock symbols embedded in lib_symbols (extends flattened)
def embed(lib, name):
    full = f'{lib}:{name}'
    if any(s[1] == full for s in kids(LIBS, 'symbol')): return
    lt = parse(open(f'{SYMS}/{lib}.kicad_symdir/{name}.kicad_sym', encoding='utf8').read())
    s = copy.deepcopy([e for e in kids(lt, 'symbol') if e[1] == name][0])
    ext = kid(s, 'extends')
    if ext:
        base = parse(open(f'{SYMS}/{lib}.kicad_symdir/{ext[1]}.kicad_sym', encoding='utf8').read())
        b = [e for e in kids(base, 'symbol') if e[1] == ext[1]][0]
        s.remove(ext)
        for e in b:
            if isinstance(e, list) and e[0] == 'symbol':
                e = copy.deepcopy(e); e[1] = Q(e[1].replace(ext[1], name, 1)); s.append(e)
            elif isinstance(e, list) and e[0] not in ('property',) and not kid(s, e[0]):
                s.insert(2, copy.deepcopy(e))
    s[1] = Q(full)
    LIBS.append(s)

# --- instances
PWR = [200]
def add_symbol(lib_id, ref, value, footprint, x, y, a=0, pins=('1', '2'), hide_ref=False):
    p = lambda n, v, dx, dy, hide=False: (['property', Q(n), Q(v), at(x + dx, y + dy), font()] +
                                           ([['hide', 'yes']] if hide else []))
    s = ['symbol', ['lib_id', Q(lib_id)], at(x, y, a), ['unit', '1'], ['exclude_from_sim', 'no'],
         ['in_bom', 'no' if ref.startswith('#') else 'yes'], ['on_board', 'no' if ref.startswith('#') else 'yes'],
         ['dnp', 'no'], ['uuid', uid()],
         p('Reference', ref, 2.5, -1.3, hide_ref), p('Value', value, 2.5, 1.3), p('Footprint', footprint, 0, 0, True),
         p('Datasheet', '', 0, 0, True), p('Description', '', 0, 0, True)]
    s += [['pin', Q(n), ['uuid', uid()]] for n in pins]
    s.append(['instances', ['project', Q('tinySensorv1'), ['path', Q('/' + ROOT), ['reference', Q(ref)], ['unit', '1']]]])
    t.append(s); return s

def power(kind, x, y, a=0):
    PWR[0] += 1
    add_symbol(f'power:{kind}', f'#PWR0{PWR[0]}', kind, '', x, y, a, pins=('1',), hide_ref=True)

def label(name, x, y, a=0):
    t.append(['label', Q(name), at(x, y, a), ['effects', ['font', ['size', '1.27', '1.27']], ['justify', 'right' if a == 180 else 'left', 'bottom']], ['uuid', uid()]])

def no_connect(x, y): t.append(['no_connect', ['at', f'{x:g}', f'{y:g}'], ['uuid', uid()]])

def text(s, x, y):
    t.append(['text', Q(s), ['exclude_from_sim', 'no'], at(x, y), ['effects', ['font', ['size', '1.27', '1.27']], ['justify', 'left', 'bottom']], ['uuid', uid()]])

def tie(netname, x, y, a=0):
    """connect a pin end to a net: power symbol for power nets, label otherwise"""
    n = netname.lstrip('/')
    if n in ('GND', 'VCC', '+BATT'): power(n, x, y, 0 if n != 'GND' else 0)
    elif n == '': no_connect(x, y)
    else: label(n, x, y, a)

def two_pin(lib_id, ref, value, footprint, x, y):
    """vertical R/C at rot 0: pin 1 top (y - 3.81), pin 2 bottom; nets from the PCB pads"""
    add_symbol(lib_id, ref, value, footprint, x, y)
    tie(pads[ref]['1'], x, y - 3.81); tie(pads[ref]['2'], x, y + 3.81)

# --- title block
tb = kid(t, 'title_block')
kid(tb, 'title')[1] = Q('TinySensor'); kid(tb, 'date')[1] = Q('2026-11-06'); kid(tb, 'rev')[1] = Q('3.0')

# --- R5: LED D1 was reversed: turn it so the anode faces R1
a = kid(sym('D1'), 'at'); a[3] = '180'

# --- R1: PA1 now reads the divider
for l in kids(t, 'label'):
    if l[1] == 'Battery_Checker' and kid(l, 'at')[1:3] == ['76.2', '38.1']: l[1] = Q('BAT_SENSE')

# --- battery block rebuilt (J10, J11, J12, J13, JP2 and their wiring removed)
doomed = []
for e in t:
    if not isinstance(e, list) or e[0] not in ('wire', 'junction', 'label', 'symbol', 'no_connect'): continue
    if e[0] == 'wire': pts = [(float(p[1]), float(p[2])) for p in kid(e, 'pts')[1:]]
    else: pts = [(float(kid(e, 'at')[1]), float(kid(e, 'at')[2]))]
    if all(198 <= x <= 272 and 12 <= y <= 52 for x, y in pts): doomed.append(e)
for e in doomed: t.remove(e)
left = [i for i in items_in(t, 198, 12, 272, 52) if i[0] != 'polyline']  # frame lines stay
assert not left, left
embed('Transistor_FET', 'AO3401A'); embed('Device', 'Battery_Cell')
ST = 'Connector:Screw_Terminal_01x02'
def terminal(ref, value, footprint, x, y, n1, n2):
    add_symbol(ST, ref, value, footprint, x, y); tie(n1, x - 5.08, y, 180); tie(n2, x - 5.08, y + 2.54)
terminal('J13', 'RAW IN', 'TinySensor lib:TerminalBlock_bornier-2_P5.08mm', 215.9, 21.59, '/RAW_IN', 'GND')
terminal('J10', 'CR2032 socket', 'TinySensor lib:BatteryHolder_Keystone_106_1x21mm', 215.9, 38.1, '+BATT', 'GND')
x, y = 226.06, 24.13
add_symbol('Device:Battery_Cell', 'BT1', '18650', 'TinySensor lib:BatteryHolder_MPD_BH-18650-PC2', x, y)
tie('/RAW_IN', x, y - 5.08); tie('GND', x, y + 2.54)
x, y = 240.03, 24.13
add_symbol('Transistor_FET:AO3401A', 'Q1', 'AO3401A', 'Package_TO_SOT_SMD:SOT-23', x, y, pins=('1', '2', '3'))
tie(pads['Q1']['1'], x - 5.08, y); tie(pads['Q1']['3'], x + 2.54, y - 5.08); tie(pads['Q1']['2'], x + 2.54, y + 5.08)
two_pin('Device:R', 'R7', '1M', 'Resistor_SMD:R_0805_2012Metric', 254.0, 24.13)
two_pin('Device:R', 'R8', '1M', 'Resistor_SMD:R_0805_2012Metric', 261.62, 34.29)
two_pin('Device:C', 'C7', '100n', 'Capacitor_SMD:C_0805_2012Metric', 269.24, 34.29)
x, y = 256.54, 46.99
add_symbol('Jumper:SolderJumper_2_Open', 'JP2', 'DIRECT Vcc', 'Jumper:SolderJumper-2_P1.3mm_Open_TrianglePad1.0x1.5mm', x, y)
tie('VCC', x - 3.81, y); tie('+BATT', x + 3.81, y)
text('Q1 : inversion de polarité (J13, BT1). PA1 = Vbatt / 2.\nUne seule source batterie. JP2 fermé => J2 non monté.', 199.39, 53.34)
for e in [e for e in kids(t, 'text') if e[1] == 'Bypass of DC-DC converter']: t.remove(e)

# --- J3 becomes the first sensor connector, J14 the second; pin 6 (SDO) goes to the JP6 selector
j3 = sym('J3')
set_prop(j3, 'Value', 'I2C 1 (BME280)'); set_prop(j3, 'Footprint', 'Connector_JST:JST_EH_B6B-EH-A_1x06_P2.50mm_Vertical')
g = [s for s in syms() if prop_val(s, 'Value') == 'GND' and kid(s, 'at')[1:3] == ['181.61', '125.73']][0]
t.remove(g); label('SDO_SEL', 181.61, 125.73)
add_symbol(kid(j3, 'lib_id')[1], 'J14', 'I2C 2', 'Connector_JST:JST_EH_B6B-EH-A_1x06_P2.50mm_Vertical', 215.9, 118.11,
           pins=tuple('123456'))
for n, y in zip('123456', (113.03, 115.57, 118.11, 120.65, 123.19, 125.73)):
    tie(pads['J14'][n], 210.82, y, 180)
embed('Jumper', 'SolderJumper_3_Bridged12')
x, y = 219.71, 135.89
add_symbol('Jumper:SolderJumper_3_Bridged12', 'JP6', 'SDO', 'Jumper:SolderJumper-3_P1.3mm_Bridged12_RoundedPad1.0x1.5mm', x, y,
           pins=('1', '2', '3'))
tie(pads['JP6']['1'], x - 5.08, y); tie(pads['JP6']['2'], x, y + 3.81); tie(pads['JP6']['3'], x + 5.08, y)
text('JP6 : SDO des capteurs (broche 6)\n1-2 (par défaut) GND = I2C 0x76\n2-3 MISO = SPI\nBroche 5 = CSB, non connectée', 200.66, 151.13)

# --- radio decoupling, RESET pull-up
two_pin('Device:C', 'C8', '100n', 'Capacitor_SMD:C_0805_2012Metric', 160.02, 186.69)
two_pin('Device:R', 'R9', '10k', 'Resistor_SMD:R_0805_2012Metric', 175.26, 186.69)
text('C8 : découplage au plus près de J1 (radio)\nR9 : pull-up RESET, sans condensateur (ISP, debugWIRE)', 185, 186)

# --- values, footprints
c6 = sym('C6'); kid(c6, 'lib_id')[1] = Q('Device:C')
set_prop(c6, 'Value', '100n'); set_prop(c6, 'Footprint', 'Capacitor_SMD:C_0805_2012Metric')
set_prop(sym('C5'), 'Footprint', 'Capacitor_THT:C_Disc_D3.0mm_W1.6mm_P2.50mm')
for r in ('R2', 'R3'): set_prop(sym(r), 'Value', '4k7')
set_prop(sym('JP3'), 'Value', 'Jumper')
for s in syms():
    if (prop_val(s, 'Reference') or '').startswith('MH'): set_prop(s, 'Footprint', 'MountingHole:MountingHole_2.7mm_M2.5')

open(dst, 'w', encoding='utf8').write(dump(t) + '\n')
print('saved', dst)
