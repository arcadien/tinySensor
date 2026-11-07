#!/usr/bin/env python3
"""Projets Orca de print/ (un par modèle) mis à jour depuis les .scad, sans toucher aux plaques.

Usage : python3 update_3mf.py [projet.3mf ...]   (par défaut : tous les projets de PROJECTS)

Projet existant : chaque objet est reconnu par son nom (parts) ; son maillage est réexporté par OpenSCAD et
remplacé sur place, recentré comme le fait Orca, en gardant sa position et son orientation (le bas reste sur
le plateau). Les réglages Orca sont gardés ; ceux de settings ne sont posés que s'ils manquent.
Projet absent : il est créé avec une plaque par entrée de plates, réglages d'imprimante et de filament repris
de TEMPLATE (le projet du champignon, réglé pour l'Ender 3).
"""
import math
import re
import shutil
import subprocess
import sys
import tempfile
import uuid
import xml.etree.ElementTree as ET
import zipfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

HERE = Path(__file__).resolve().parent
PRINT = HERE / "print"
OPENSCAD = sorted(Path.home().glob("bin/OpenSCAD-*.AppImage"))[-1]
TEMPLATE = PRINT / "mushroom.3mf"
BED = 220      # plateau de l'Ender 3 ; Orca espace ses plaques de 1,2 plateau
GAP = 10       # entre deux pièces d'une même plaque

# parts : nom dans Orca -> arguments OpenSCAD (pièce dans son sens d'impression)
# plates : [(nom de plaque, [pièces, une entrée par exemplaire])], pour créer le projet
PROJECTS = {
    "mushroom.3mf": {
        "scad": "mushroom.scad",
        "parts": {
            "Chapeau": ['part="cap"'], "Lamelles": ['part="gills"'], "Châssis": ['part="chassis"'],
            "Plancher": ['part="deck"'], "Pied": ['part="stem"'], "Sol": ['part="ground"'],
            "Berceau": ['part="cradle"'], "Bouchon": ['part="plug"'],
            "Pois Ø6": ['part="dot"', "dot_d=6"], "Pois Ø8": ['part="dot"', "dot_d=8"],
            "Pois Ø10": ['part="dot"', "dot_d=10"],
        },
        # chapeau très haut : bord de 6 mm + brim ; sans support (les faces des consoles passent en pont en PETG,
        # vérifié à l'impression du 2026-11)
        "settings": {"Chapeau": {"brim_type": "outer_only", "brim_width": "10", "sparse_infill_density": "10%"},
                     "Pied": {"wall_loops": "6"}},
        "add": {"Bouchon": ("Divers", (300.0, -154.0))},   # ajouté s'il manque, à gauche du plancher et du berceau
        # objets d'avant le renommage (« OpenSCAD Model »), reconnus par leur fichier d'origine
        "from_file": {"mushroom_cap": "Chapeau", "mushroom_gills": "Lamelles", "mushroom_chassis": "Châssis",
                      "mushroom_deck": "Plancher", "mushroom_stem": "Pied", "mushroom_ground": "Sol",
                      "mushroom_cradle": "Berceau", "mushroom_plug": "Bouchon", "mushroom_dot6": "Pois Ø6",
                      "mushroom_dot8": "Pois Ø8", "mushroom_dot10": "Pois Ø10"},
    },
    "box.3mf": {   # boîtier mural : n'imprimer qu'un des deux fonds
        "scad": "tinysensor_box.scad",
        "parts": {"Fond extérieur": ['part="base"', 'usage="ext"'], "Fond intérieur": ['part="base"', 'usage="int"'],
                  "Couvercle": ['part="lid"']},
        "plates": [("Fond extérieur", ["Fond extérieur"]), ("Fond intérieur", ["Fond intérieur"]),
                   ("Couvercle", ["Couvercle"])],
    },
    "shield.3mf": {   # abri de sonde
        "scad": "sensor_shield.scad",
        "parts": {"Chapeau": ['part="cap"'], "Assiette": ['part="plate"'], "Fond": ['part="base"'],
                  "Clip": ['part="clip"']},
        "plates": [("Abri", ["Chapeau", "Fond", "Clip"]), ("Assiettes", ["Assiette"] * 4)],
    },
    "stand.3mf": {   # pied de table
        "scad": "stand.scad",
        "parts": {"Mât": ['part="mast"'], "Pied": ['part="foot"']},
        "plates": [("Mât", ["Mât"]), ("Pied", ["Pied"])],
    },
}

NS = ('xmlns="http://schemas.microsoft.com/3dmanufacturing/core/2015/02" '
      'xmlns:BambuStudio="http://schemas.bambulab.com/package/2021" '
      'xmlns:p="http://schemas.microsoft.com/3dmanufacturing/production/2015/06" requiredextensions="p"')
REL_3D = "http://schemas.microsoft.com/3dmanufacturing/2013/01/3dmodel"


def export(scad, args, out):
    cmd = [str(OPENSCAD), "--backend=manifold", "-o", str(out)]
    for a in args:
        cmd += ["-D", a]
    r = subprocess.run(cmd + [str(HERE / scad)], capture_output=True, text=True)
    if r.returncode or re.search(r"WARNING|ERROR", r.stderr):
        sys.exit(f"OpenSCAD {scad} {args} :\n{r.stderr}")
    model = next(n for n in zipfile.ZipFile(out).namelist() if n.endswith(".model"))
    t = zipfile.ZipFile(out).read(model).decode()
    v = [tuple(map(float, m)) for m in re.findall(r'<vertex x="([^"]+)" y="([^"]+)" z="([^"]+)"', t)]
    f = re.findall(r'<triangle v1="(\d+)" v2="(\d+)" v3="(\d+)"', t)
    lo, hi = [min(c) for c in zip(*v)], [max(c) for c in zip(*v)]
    c = [(a + b) / 2 for a, b in zip(lo, hi)]   # Orca centre le maillage sur sa boîte
    mesh = "<mesh>\n    <vertices>\n" + "".join(
        f'     <vertex x="{x - c[0]:.6g}" y="{y - c[1]:.6g}" z="{z - c[2]:.6g}"/>\n' for x, y, z in v) + \
        "    </vertices>\n    <triangles>\n" + "".join(
        f'     <triangle v1="{a}" v2="{b}" v3="{d}"/>\n' for a, b, d in f) + "    </triangles>\n   </mesh>"
    return {"mesh": mesh, "center": c, "lo_z": lo[2] - c[2], "size": [b - a for a, b in zip(lo, hi)]}


def meta(el, key, value=None):
    m = el.find(f"metadata[@key='{key}']")
    if value is None:
        return None if m is None else m.get("value")
    if m is None:   # avec les autres métadonnées, avant les <part>, <model_instance>
        m = ET.Element("metadata", key=key)
        el.insert(len(el.findall("metadata")), m)
    m.set("value", value)


def skeleton(src, plate_names):
    """Projet vide : réglages d'imprimante et de filament du TEMPLATE, une plaque par nom."""
    tpl = zipfile.ZipFile(TEMPLATE)
    for n in ("[Content_Types].xml", "Metadata/project_settings.config", "Metadata/slice_info.config"):
        (src / n).parent.mkdir(parents=True, exist_ok=True)
        (src / n).write_bytes(tpl.read(n))
    head = re.match(r".*?<resources>", tpl.read("3D/3dmodel.model").decode(), re.S).group(0)
    (src / "3D" / "_rels").mkdir(parents=True)
    (src / "3D" / "Objects").mkdir()
    (src / "3D" / "3dmodel.model").write_text(
        re.sub(r'(<metadata name="Title">)[^<]*', r"\1", head) + f'\n </resources>\n <build p:UUID="{uuid.uuid4()}">\n </build>\n</model>\n')
    rels = ('<?xml version="1.0" encoding="UTF-8"?>\n<Relationships '
            'xmlns="http://schemas.openxmlformats.org/package/2006/relationships">\n{}</Relationships>\n')
    (src / "_rels").mkdir()
    (src / "_rels" / ".rels").write_text(rels.format(f' <Relationship Target="/3D/3dmodel.model" Id="rel-1" Type="{REL_3D}"/>\n'))
    (src / "3D" / "_rels" / "3dmodel.model.rels").write_text(rels.format(""))
    tpl_plate = ET.fromstring(tpl.read("Metadata/model_settings.config")).find("plate")
    root = ET.Element("config")
    for i, name in enumerate(plate_names, 1):
        p = ET.SubElement(root, "plate")
        meta(p, "plater_id", str(i))
        meta(p, "plater_name", name)
        for k in ("locked", "filament_map_mode", "filament_maps"):
            meta(p, k, meta(tpl_plate, k))
    ET.ElementTree(root).write(src / "Metadata" / "model_settings.config", encoding="UTF-8", xml_declaration=True)


def plate_origin(k, n):
    cols = math.ceil(math.sqrt(n))
    return (k % cols) * BED * 1.2, -(k // cols) * BED * 1.2


def layout(sizes):
    """Pièces en rangées, bloc centré sur la plaque ; centres [(x, y)] relatifs au coin de la plaque."""
    rows, row, w = [], [], 0
    for s in sizes:
        if row and w + GAP + s[0] > BED - 2 * GAP:
            rows.append(row)
            row, w = [], 0
        row.append(s)
        w += (GAP if len(row) > 1 else 0) + s[0]
    rows.append(row)
    heights = [max(s[1] for s in r) for r in rows]
    total = sum(heights) + GAP * (len(rows) - 1)
    width = max(sum(s[0] for s in r) + GAP * (len(r) - 1) for r in rows)
    if total > BED or width > BED:
        sys.exit(f"Pièces trop grandes pour une plaque de {BED} mm : {sizes}")
    out, y = [], (BED + total) / 2
    for r, h in zip(rows, heights):
        x = (BED - (sum(s[0] for s in r) + GAP * (len(r) - 1))) / 2
        for s in r:
            out.append((x + s[0] / 2, y - h / 2))
            x += s[0] + GAP
        y -= h + GAP
    return out


def update(path, spec):
    work = Path(tempfile.mkdtemp())
    src = work / "src"
    created = not path.exists()
    if created:
        skeleton(src, [p for p, _ in spec["plates"]])
    else:
        zipfile.ZipFile(path).extractall(src)
    cfg_path = src / "Metadata" / "model_settings.config"
    model_path = src / "3D" / "3dmodel.model"
    cfg = ET.parse(cfg_path)
    root = cfg.getroot()
    model = model_path.read_text()
    parts = spec["parts"]

    # maillages : un export par pièce, en parallèle
    with ThreadPoolExecutor() as ex:
        new = dict(zip(parts, ex.map(lambda ia: export(spec["scad"], ia[1], work / f"part{ia[0]}.3mf"),
                                     enumerate(parts.values()))))

    # identification et renommage
    objs = {}
    for o in root.findall("object"):
        part = o.find("part")
        name = meta(o, "name")
        if name not in parts:
            stem = Path(meta(part, "source_file") or "").stem
            name = next((v for k, v in spec.get("from_file", {}).items() if re.fullmatch(k + r"(_x\d+)?", stem)), None)
            if name is None:
                sys.exit(f"{path.name} : objet {o.get('id')} non reconnu ({meta(o, 'name')})")
        meta(o, "name", name)
        meta(part, "name", name)
        objs[o.get("id")] = (name, o, part)

    def add_object(name, plate_name, xy):
        nonlocal model
        ids = [int(i) for i in re.findall(r'<object id="(\d+)"', model)] or [0]
        mesh_id, obj_id = max(ids) + 1, max(ids) + 2
        mesh_path = f"/3D/Objects/object_{obj_id}.model"
        (src / mesh_path.lstrip("/")).write_text(
            f'<?xml version="1.0" encoding="UTF-8"?>\n<model unit="millimeter" xml:lang="en-US" {NS}>\n'
            ' <metadata name="BambuStudio:3mfVersion">1</metadata>\n <resources>\n'
            f'  <object id="{mesh_id}" p:UUID="{uuid.uuid4()}" type="model">\n   <mesh/>\n  </object>\n'
            ' </resources>\n <build/>\n</model>\n')
        model = model.replace(" </resources>",
            f'  <object id="{obj_id}" p:UUID="{uuid.uuid4()}" type="model">\n   <components>\n'
            f'    <component p:path="{mesh_path}" objectid="{mesh_id}" p:UUID="{uuid.uuid4()}" '
            'transform="1 0 0 0 1 0 0 0 1 0 0 0"/>\n   </components>\n  </object>\n </resources>', 1)
        model = model.replace(" </build>", f'  <item objectid="{obj_id}" p:UUID="{uuid.uuid4()}" '
                              f'transform="1 0 0 0 1 0 0 0 1 {xy[0]:.4f} {xy[1]:.4f} 0" printable="1"/>\n </build>', 1)
        rels = src / "3D" / "_rels" / "3dmodel.model.rels"
        rels.write_text(rels.read_text().replace("</Relationships>",
            f' <Relationship Target="{mesh_path}" Id="rel-{obj_id}" Type="{REL_3D}"/>\n</Relationships>'))
        o = ET.Element("object", id=str(obj_id))
        meta(o, "name", name)
        meta(o, "extruder", "1")
        part = ET.SubElement(o, "part", id=str(mesh_id), subtype="normal_part")
        meta(part, "name", name)
        meta(part, "matrix", "1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1")
        plates = root.findall("plate")
        root.insert(list(root).index(plates[0]), o)
        plate = next(p for p in plates if meta(p, "plater_name") == plate_name)
        inst = ET.SubElement(plate, "model_instance")
        meta(inst, "object_id", str(obj_id))
        meta(inst, "instance_id", "0")
        meta(inst, "identify_id", str(1 + max([int(meta(i, "identify_id")) for i in root.iter("model_instance")
                                               if meta(i, "identify_id")] or [100])))
        objs[str(obj_id)] = (name, o, part)

    if created:
        n = len(spec["plates"])
        for k, (plate_name, names) in enumerate(spec["plates"]):
            ox, oy = plate_origin(k, n)
            for name, (x, y) in zip(names, layout([new[nm]["size"][:2] for nm in names])):
                add_object(name, plate_name, (ox + x, oy + y))
    for name, (plate_name, xy) in spec.get("add", {}).items():
        if all(nm != name for nm, _, _ in objs.values()):
            add_object(name, plate_name, xy)
    missing = set(parts) - {nm for nm, _, _ in objs.values()}
    if missing:
        print(f"{path.name} : absent du projet, à ajouter dans Orca : {', '.join(sorted(missing))}")

    for name, o, _ in objs.values():
        for k, v in spec.get("settings", {}).get(name, {}).items():
            if meta(o, k) is None:
                meta(o, k, v)

    done = set()
    for oid, (name, o, part) in objs.items():
        comp = re.search(rf'<object id="{oid}"[^>]*>\s*<components>\s*<component p:path="([^"]+)" objectid="(\d+)"', model)
        f, mid = src / comp.group(1).lstrip("/"), comp.group(2)
        t = f.read_text()
        old = re.search(rf'(<object id="{mid}"[^>]*>\s*)(<mesh>.*?</mesh>|<mesh/>)', t, re.S)
        lo_old = min((float(z) for z in re.findall(r'z="([^"]+)"', old.group(2))), default=None)
        if (f, mid) not in done:   # des exemplaires peuvent partager un maillage (pois du champignon)
            f.write_text(t[:old.start(2)] + new[name]["mesh"] + t[old.end(2):])
            done.add((f, mid))
        # le bas de la pièce reste sur le plateau ; rotations autour de Z seulement
        item = re.search(rf'(<item objectid="{oid}" [^>]*transform=")([^"]+)(")', model)
        m = [float(a) for a in item.group(2).split()]
        if max(abs(m[2]), abs(m[5]), abs(m[6]), abs(m[7])) > 1e-6 or abs(m[8] - 1) > 1e-6:
            sys.exit(f"{path.name} : {name} basculé dans Orca, mise à jour refusée")
        m[11] = -new[name]["lo_z"] if lo_old is None else m[11] + lo_old - new[name]["lo_z"]   # objet neuf : posé
        model = model[:item.start(2)] + " ".join(f"{a:.9g}" for a in m) + model[item.end(2):]
        for k, v in zip("xyz", new[name]["center"]):
            meta(part, "source_offset_" + k, repr(v))

    model_path.write_text(model)
    cfg.write(cfg_path, encoding="UTF-8", xml_declaration=True)
    out = work / "out.3mf"
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
        for p in sorted(src.rglob("*"), key=lambda p: p.name != "[Content_Types].xml"):   # en tête de l'archive
            if p.is_file():
                z.write(p, p.relative_to(src).as_posix())
    if not created:
        shutil.copy(path, path.with_suffix(".3mf.bak"))
    shutil.move(out, path)
    shutil.rmtree(work)
    counts = {}
    for nm, _, _ in objs.values():
        counts[nm] = counts.get(nm, 0) + 1
    print(f"{path.name} : {'créé' if created else 'mis à jour'}, {len(objs)} objets {counts}"
          + ("" if created else f" ; ancien fichier : {path.name}.bak"))


targets = [Path(a) for a in sys.argv[1:]] or [PRINT / n for n in PROJECTS]
for p in targets:
    update(p, PROJECTS[p.name])
