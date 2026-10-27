#!/usr/bin/env python3
"""Faisceau accu du champignon TinySensor -> faisceau_champignon.pdf (A4 portrait, 300 dpi).

Usage : python3 harness.py [sortie.pdf]
Dépendance : Pillow seulement. Police : DejaVu Sans (fc-list | grep DejaVuSans).
Les données sont dans WIRES (une ligne par fil) : modifier là, puis relancer.
"""
import sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

HERE = Path(__file__).resolve().parent
OUT = Path(sys.argv[1]) if len(sys.argv) > 1 else HERE / "faisceau_champignon.pdf"

# --- Données -----------------------------------------------------------------------------
# id, couleur, signal, de (repère schéma, texte), extrémité de départ, vers (repère, texte),
# extrémité d'arrivée, longueur (mm), section
WIRES = [
    ("W1", "rouge", "B+",    "bat+",  "languette « + » du berceau (haut)", "soudé",
     "jstF1", "JST-XH 2p, broche 1", "JST-XH F (côté berceau)", 40, "≥ 24 AWG"),
    ("W2", "noir",  "B−",    "bat-",  "languette « − » du berceau (bas, passe dans le méplat)", "soudé",
     "jstF2", "JST-XH 2p, broche 2", "JST-XH F (côté berceau)", 110, "≥ 24 AWG"),
    ("W3", "rouge", "B+",    "jstM1", "JST-XH 2p, broche 1", "JST-XH M (côté chapeau)",
     "tpB+",  "TP4056 B+", "soudé", 120, "≥ 24 AWG"),
    ("W4", "noir",  "B−",    "jstM2", "JST-XH 2p, broche 2", "JST-XH M (côté chapeau)",
     "tpB-",  "TP4056 B−", "soudé", 120, "≥ 24 AWG"),
    ("W5", "rouge", "+BATT", "tpO+",  "TP4056 OUT+", "soudé",
     "j13-1", "J13 « RAW IN » borne 1 (+BATT)", "bornier à vis", 100, "≥ 24 AWG"),
    ("W6", "noir",  "GND",   "tpO-",  "TP4056 OUT−", "soudé",
     "j13-2", "J13 borne 2 (GND)", "bornier à vis", 100, "≥ 24 AWG"),
]
COLORS = {"rouge": (200, 20, 20), "noir": (0, 0, 0)}

NOTES = [
    "Capteur BME280 : aucun fil. Sa barrette mâle 6 broches s'enfiche directement dans une barrette "
    "femelle 2,54 mm de 6 positions soudée sous la carte, dans l'empreinte « I2C » (1 SENSOR_VCC, "
    "2 GND, 3 SCL, 4 SDA, 5 non connectée, 6 GND). Le breakout pend parallèle à la carte, 2 mm au-dessus du disque à lamelles.",
    "CSB tombe sur la broche 5 (non connectée : le tirage du breakout garde le mode I2C), SDO sur la 6 "
    "(GND : adresse 0x76, celle du firmware).",
    "Pas de résistance dans le faisceau : les tirages I2C R2/R3 (4,7 kΩ) sont sur la carte.",
    "La paire JST-XH se loge dans le fourreau du pied, juste au-dessus du berceau : on soulève "
    "le chapeau (≈ 20 mm) pour la débrancher.",
    "W3/W4 montent par le trou central du disque (moitié +Y, pour éviter le BME280), sortent de la "
    "chambre du capteur par la fenêtre de la paroi +Y (x = 22) et rejoignent le TP4056 (debout dans "
    "sa glissière à x = 40, micro-USB vers le bas, pastilles en haut). W5/W6 passent au-dessus de la carte jusqu'à J13.",
    "L'antenne passe sous le chapeau.",
    "Longueurs : environ 30 mm de mou compris pour soulever le chapeau ; à ajuster au montage.",
]

# --- Mise en page ------------------------------------------------------------------------
DPI = 300
W, H = 2480, 3508          # A4 à 300 dpi
M = 150                    # marge
FONT_DIR = Path("/usr/share/fonts/truetype/dejavu")


def font(size, bold=False):
    return ImageFont.truetype(str(FONT_DIR / ("DejaVuSans-Bold.ttf" if bold else "DejaVuSans.ttf")), size)


F_TITLE, F_H2, F_TXT, F_SMALL, F_BOLD = font(64, True), font(44, True), font(32), font(30), font(32, True)
# 30 px à 300 dpi = 7,2 pt : plus petite police utilisée


def wrap(d, text, f, width):
    lines, cur = [], ""
    for w in text.split():
        t = (cur + " " + w).strip()
        if d.textlength(t, font=f) <= width:
            cur = t
        else:
            if cur:
                lines.append(cur)
            cur = w
    if cur:
        lines.append(cur)
    return lines


def lh(f):
    return int(f.size * 1.3)


class Doc:
    def __init__(self):
        self.pages = []
        self.new_page()

    def new_page(self):
        self.img = Image.new("RGB", (W, H), "white")
        self.d = ImageDraw.Draw(self.img)
        self.pages.append(self.img)
        self.y = M

    def need(self, h):
        if self.y + h > H - M:
            self.new_page()


def diagram(doc):
    d = doc.d
    y0 = doc.y
    P1, P2 = y0 + 360, y0 + 650           # lignes des fils + et −
    # zones pied / chapeau
    xs = 1080
    d.rectangle([M, y0, xs - 20, y0 + 1000], outline=(120, 120, 120), width=3)
    d.rectangle([xs + 20, y0, W - M, y0 + 1000], outline=(120, 120, 120), width=3)
    d.text((M + 20, y0 + 15), "PIED (fourreau de l'accu)", font=F_BOLD, fill=(90, 90, 90))
    d.text((xs + 40, y0 + 15), "CHAPEAU (au-dessus du disque à lamelles)", font=F_BOLD, fill=(90, 90, 90))

    pts = {}
    # berceau 18650 (vertical, + en haut)
    cx0, cx1, cy0, cy1 = 230, 390, P1 - 90, P2 + 150
    d.rounded_rectangle([cx0, cy0, cx1, cy1], radius=30, outline="black", width=5, fill=(235, 235, 225))
    d.text(((cx0 + cx1) / 2, (cy0 + cy1) / 2), "18650", font=F_BOLD, fill="black", anchor="mm")
    for y, s in ((P1, "+"), (P2 + 60, "−")):
        d.rectangle([cx1, y - 15, cx1 + 40, y + 15], fill=(190, 190, 190), outline="black", width=3)
        d.text((cx1 - 20, y), s, font=F_H2, fill="black", anchor="rm")
    d.text(((cx0 + cx1) / 2, cy0 - 75), "berceau", font=F_SMALL, fill="black", anchor="mb")
    d.text(((cx0 + cx1) / 2, cy0 - 35), "(languettes)", font=F_SMALL, fill="black", anchor="mb")
    pts["bat+"], pts["bat-"] = (cx1 + 40, P1), (cx1 + 40, P2 + 60)

    # paire JST-XH 2 broches : F (côté berceau) | M (côté chapeau)
    jx = 880
    d.rectangle([jx - 70, P1 - 50, jx, P2 + 50], outline="black", width=4, fill=(250, 250, 250))
    d.rectangle([jx, P1 - 50, jx + 70, P2 + 50], outline="black", width=4, fill=(220, 220, 220))
    for i, y in enumerate((P1, P2), 1):
        d.text((jx, y), str(i), font=F_BOLD, fill="black", anchor="mm")
    d.text((jx, P2 + 120), "JST-XH", font=F_BOLD, fill="black", anchor="mt")
    d.text((jx, P2 + 120 + lh(F_BOLD)), "2p", font=F_BOLD, fill="black", anchor="mt")
    d.text((jx - 35, P2 + 70), "F", font=F_BOLD, fill="black", anchor="mt")
    d.text((jx + 35, P2 + 70), "M", font=F_BOLD, fill="black", anchor="mt")
    pts["jstF1"], pts["jstF2"] = (jx - 70, P1), (jx - 70, P2)
    pts["jstM1"], pts["jstM2"] = (jx + 70, P1), (jx + 70, P2)

    # TP4056
    tx0, tx1 = 1380, 1620
    d.rectangle([tx0, P1 - 100, tx1, P2 + 100], outline="black", width=5, fill=(30, 80, 160))
    d.text(((tx0 + tx1) / 2, (P1 + P2) / 2 - 40), "TP4056", font=F_BOLD, fill="white", anchor="mm")
    d.text(((tx0 + tx1) / 2, (P1 + P2) / 2 + 15), "micro-USB", font=F_SMALL, fill="white", anchor="mm")
    d.text(((tx0 + tx1) / 2, (P1 + P2) / 2 + 55), "vers le bas", font=F_SMALL, fill="white", anchor="mm")
    for x, y, name, key, anc in ((tx0, P1, "B+", "tpB+", "lm"), (tx0, P2, "B−", "tpB-", "lm"),
                                 (tx1, P1, "OUT+", "tpO+", "rm"), (tx1, P2, "OUT−", "tpO-", "rm")):
        d.rectangle([x - 14, y - 14, x + 14, y + 14], fill=(210, 170, 60), outline="black", width=2)
        d.text((x + (22 if anc == "lm" else -22), y), name, font=F_SMALL, fill="white", anchor=anc)
        pts[key] = (x, y)

    # carte v2.2 et bornier J13
    bx0, bx1 = 2020, W - M - 30
    d.rectangle([bx0, P1 - 120, bx1, P2 + 160], outline="black", width=4, fill=(30, 110, 60))
    d.text(((bx0 + bx1) / 2, P2 + 140), "carte v2.2", font=F_BOLD, fill="white", anchor="mb")
    jx0, jx1 = bx0 + 10, bx0 + 110
    d.rectangle([jx0, P1 - 60, jx1, P2 + 60], outline="black", width=4, fill=(40, 120, 200))
    for i, y in enumerate((P1, P2), 1):
        d.ellipse([jx0 + 25, y - 25, jx0 + 75, y + 25], fill=(200, 200, 200), outline="black", width=3)
        d.line([jx0 + 33, y - 17, jx0 + 67, y + 17], fill="black", width=4)
        d.text((jx1 + 15, y), str(i), font=F_BOLD, fill="white", anchor="lm")
        pts["j13-%d" % i] = (jx0, y)
    d.text(((bx0 + bx1) / 2, P1 - 70), "J13 RAW IN", font=F_BOLD, fill="white", anchor="mb")
    d.text(((bx0 + bx1) / 2, P2 + 175), "J13 : bornier à vis 5,08", font=F_SMALL, fill="black", anchor="mt")

    # fils + étiquettes
    for wid, col, sig, a, _, a_end, b, _, b_end, length, awg in WIRES:
        (xa, ya), (xb, yb) = pts[a], pts[b]
        rgb = COLORS[col]
        if ya == yb:
            d.line([xa, ya, xb, yb], fill=rgb, width=10)
            xm, ym = (xa + xb) / 2, ya
        else:                                   # W2 : horizontal en bas, puis remonte à la broche
            xk = xb - 60
            d.line([(xa, ya), (xk, ya), (xk, yb), (xb, yb)], fill=rgb, width=10, joint="curve")
            xm, ym = (xa + xk) / 2, ya
        below = sig in ("B−", "GND")
        lines = ["%s  %s %s" % (wid, col, sig), "%d mm · %s" % (length, awg), a_end + " →", b_end]
        fl = [F_BOLD, F_SMALL, F_SMALL, F_SMALL]
        tw = max(d.textlength(t, font=f) for t, f in zip(lines, fl))
        th = sum(lh(f) for f in fl)
        ty = ym + 22 if below else ym - 22 - th
        d.rectangle([xm - tw / 2 - 12, ty - 6, xm + tw / 2 + 12, ty + th], fill="white", outline=rgb, width=3)
        for t, f in zip(lines, fl):
            d.text((xm, ty), t, font=f, fill=rgb if f is F_BOLD else "black", anchor="mt")
            ty += lh(f)
    doc.y = y0 + 1000 + 60


def table(doc):
    d = doc.d
    cols = [("#", 80), ("Couleur", 160), ("Signal", 150), ("De (extrémité)", 660),
            ("Vers (extrémité)", 660), ("Longueur", 210), ("Section", 0)]
    cols[-1] = (cols[-1][0], W - 2 * M - sum(w for _, w in cols[:-1]))
    rows = [[wid, col, sig, "%s — %s" % (a_txt, a_end), "%s — %s" % (b_txt, b_end), "%d mm" % length, awg]
            for wid, col, sig, _, a_txt, a_end, _, b_txt, b_end, length, awg in WIRES]
    pad = 14

    def row(cells, f, fill=None, colour=None):
        wrapped = [wrap(d, c, f, w - 2 * pad) for c, (_, w) in zip(cells, cols)]
        h = max(len(w) for w in wrapped) * lh(f) + 2 * pad
        doc.need(h)
        x = M
        for i, (lines, (_, w)) in enumerate(zip(wrapped, cols)):
            doc.d.rectangle([x, doc.y, x + w, doc.y + h], outline="black", width=2, fill=fill)
            for k, t in enumerate(lines):
                c = colour if (colour and i == 1) else "black"
                doc.d.text((x + pad, doc.y + pad + k * lh(f)), t, font=f, fill=c)
            x += w
        doc.y += h

    doc.need(lh(F_H2) + 200)
    doc.d.text((M, doc.y), "Liste des fils", font=F_H2, fill="black")
    doc.y += lh(F_H2) + 10
    row([c for c, _ in cols], F_BOLD, fill=(225, 225, 225))
    for r, w in zip(rows, WIRES):
        row(r, F_SMALL, colour=COLORS[w[1]])
    doc.y += 50


def notes(doc):
    doc.need(lh(F_H2) + 3 * lh(F_TXT))
    doc.d.text((M, doc.y), "Notes", font=F_H2, fill="black")
    doc.y += lh(F_H2) + 10
    for n in NOTES:
        lines = wrap(doc.d, n, F_TXT, W - 2 * M - 50)
        doc.need(len(lines) * lh(F_TXT) + 15)
        doc.d.text((M, doc.y), "•", font=F_TXT, fill="black")
        for t in lines:
            doc.d.text((M + 50, doc.y), t, font=F_TXT, fill="black")
            doc.y += lh(F_TXT)
        doc.y += 15


def main():
    doc = Doc()
    d = doc.d
    d.text((M, doc.y), "Faisceau accu — TinySensor « champignon »", font=F_TITLE, fill="black")
    doc.y += lh(F_TITLE)
    d.text((M, doc.y), "18650 (berceau) → JST-XH → TP4056 → J13 « RAW IN » de la carte v2.2 ; "
           "rouge = +, noir = −", font=F_TXT, fill=(70, 70, 70))
    doc.y += lh(F_TXT) + 40
    diagram(doc)
    table(doc)
    notes(doc)
    doc.pages[0].save(OUT, "PDF", resolution=DPI, save_all=True, append_images=doc.pages[1:],
                      title="Faisceau accu — TinySensor champignon",
                      creationDate=None, modDate=None)   # pas de date : sortie reproductible
    print("%s : %d page(s)" % (OUT, len(doc.pages)))


if __name__ == "__main__":
    main()
