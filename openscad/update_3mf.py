#!/usr/bin/env python3
"""Met à jour le projet Orca du champignon (print/mushroom.3mf) depuis mushroom.scad, sans toucher aux plaques.

Usage : python3 update_3mf.py [projet.3mf]

Chaque objet est reconnu par son nom (PARTS) : son maillage est réexporté par OpenSCAD et remplacé sur place,
recentré comme le fait Orca, en gardant sa position et son orientation sur la plaque (le bas reste sur le plateau).
Le bouchon est ajouté sur la plaque « Divers » s'il manque. Les réglages de SETTINGS ne sont posés que sur
un objet qui ne les a pas encore : ce qui a été changé dans Orca est gardé.
Au premier passage, les objets encore nommés « OpenSCAD Model » sont reconnus par leur fichier d'origine.
"""
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
PROJECT = Path(sys.argv[1]) if len(sys.argv) > 1 else HERE / "print" / "mushroom.3mf"
OPENSCAD = sorted(Path.home().glob("bin/OpenSCAD-*.AppImage"))[-1]

# nom dans Orca -> arguments OpenSCAD (pièce dans son sens d'impression)
PARTS = {
    "Chapeau": ['part="cap"'], "Lamelles": ['part="gills"'], "Châssis": ['part="chassis"'],
    "Plancher": ['part="deck"'], "Pied": ['part="stem"'], "Sol": ['part="ground"'],
    "Berceau": ['part="cradle"'], "Bouchon": ['part="plug"'],
    "Pois Ø6": ['part="dot"', "dot_d=6"], "Pois Ø8": ['part="dot"', "dot_d=8"], "Pois Ø10": ['part="dot"', "dot_d=10"],
}
FROM_FILE = {"mushroom_cap": "Chapeau", "mushroom_gills": "Lamelles", "mushroom_chassis": "Châssis",
             "mushroom_deck": "Plancher", "mushroom_stem": "Pied", "mushroom_ground": "Sol",
             "mushroom_cradle": "Berceau", "mushroom_plug": "Bouchon", "mushroom_dot6": "Pois Ø6",
             "mushroom_dot8": "Pois Ø8", "mushroom_dot10": "Pois Ø10"}
# réglages par objet (README, « Impression ») : chapeau très haut, bord de 6 mm + brim ; sans support (les
# faces des consoles passent en pont en PETG, vérifié à l'impression du 2026-11)
SETTINGS = {
    "Chapeau": {"brim_type": "outer_only", "brim_width": "10", "sparse_infill_density": "10%"},
    "Pied": {"wall_loops": "6"},
}
PLUG = ("Bouchon", "Divers", (300.0, -154.0))   # plaque et position (à gauche du plancher et du berceau)


def export(args, out):
    cmd = [str(OPENSCAD), "--backend=manifold", "-o", str(out)]
    for a in args:
        cmd += ["-D", a]
    r = subprocess.run(cmd + [str(HERE / "mushroom.scad")], capture_output=True, text=True)
    if r.returncode or re.search(r"WARNING|ERROR", r.stderr):
        sys.exit(f"OpenSCAD {args} :\n{r.stderr}")
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
    return {"mesh": mesh, "center": c, "lo_z": lo[2] - c[2]}


def main():
    work = Path(tempfile.mkdtemp())
    src = work / "src"
    zipfile.ZipFile(PROJECT).extractall(src)
    cfg_path = src / "Metadata" / "model_settings.config"
    model_path = src / "3D" / "3dmodel.model"
    cfg = ET.parse(cfg_path)
    root = cfg.getroot()
    model = model_path.read_text()

    def meta(el, key, value=None):
        m = el.find(f"metadata[@key='{key}']")
        if value is None:
            return None if m is None else m.get("value")
        if m is None:   # avec les autres métadonnées, avant les <part> et <model_instance>
            m = ET.Element("metadata", key=key)
            el.insert(len(el.findall("metadata")), m)
        m.set("value", value)

    # identification et renommage
    objs = {}
    for o in root.findall("object"):
        part = o.find("part")
        name = meta(o, "name")
        if name not in PARTS:
            stem = Path(meta(part, "source_file") or "").stem
            name = next((v for k, v in FROM_FILE.items() if re.fullmatch(k + r"(_x\d+)?", stem)), None)
            if name is None:
                sys.exit(f"Objet {o.get('id')} non reconnu : {meta(o, 'name')} / {stem}")
        meta(o, "name", name)
        meta(part, "name", name)
        objs[o.get("id")] = (name, o, part)

    # bouchon : copie du plancher, nouveaux identifiants, sur sa plaque
    if all(n != PLUG[0] for n, _, _ in objs.values()):
        ids = [int(i) for i in re.findall(r'<object id="(\d+)"', model)]
        mesh_id, obj_id = max(ids) + 1, max(ids) + 2
        path = f"/3D/Objects/{PLUG[0]}.model"
        (src / path.lstrip("/")).write_text(
            '<?xml version="1.0" encoding="UTF-8"?>\n<model unit="millimeter" xml:lang="en-US" '
            'xmlns="http://schemas.microsoft.com/3dmanufacturing/core/2015/02" '
            'xmlns:BambuStudio="http://schemas.bambulab.com/package/2021" '
            'xmlns:p="http://schemas.microsoft.com/3dmanufacturing/production/2015/06" requiredextensions="p">\n'
            ' <metadata name="BambuStudio:3mfVersion">1</metadata>\n <resources>\n'
            f'  <object id="{mesh_id}" p:UUID="{uuid.uuid4()}" type="model">\n   <mesh/>\n  </object>\n'
            ' </resources>\n <build/>\n</model>\n')
        model = model.replace(" </resources>",
            f'  <object id="{obj_id}" p:UUID="{uuid.uuid4()}" type="model">\n   <components>\n'
            f'    <component p:path="{path}" objectid="{mesh_id}" p:UUID="{uuid.uuid4()}" '
            'transform="1 0 0 0 1 0 0 0 1 0 0 0"/>\n   </components>\n  </object>\n </resources>', 1)
        x, y = PLUG[2]
        model = model.replace(" </build>", f'  <item objectid="{obj_id}" p:UUID="{uuid.uuid4()}" '
                              f'transform="1 0 0 0 1 0 0 0 1 {x} {y} 0" printable="1" auto_drop="1"/>\n </build>', 1)
        rels = src / "3D" / "_rels" / "3dmodel.model.rels"
        rels.write_text(rels.read_text().replace("</Relationships>",
            f' <Relationship Target="{path}" Id="rel-{PLUG[0]}" '
            'Type="http://schemas.microsoft.com/3dmanufacturing/2013/01/3dmodel"/>\n</Relationships>'))
        o = ET.Element("object", id=str(obj_id))
        meta(o, "name", PLUG[0])
        meta(o, "extruder", "1")
        part = ET.SubElement(o, "part", id=str(mesh_id), subtype="normal_part")
        meta(part, "name", PLUG[0])
        meta(part, "matrix", "1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1")
        root.insert(list(root).index(root.find("plate")), o)
        plate = next(p for p in root.findall("plate") if meta(p, "plater_name") == PLUG[1])
        inst = ET.SubElement(plate, "model_instance")
        meta(inst, "object_id", str(obj_id))
        meta(inst, "instance_id", "0")
        meta(inst, "identify_id", str(1 + max(int(meta(i, "identify_id")) for i in root.iter("model_instance")
                                               if meta(i, "identify_id"))))
        objs[str(obj_id)] = (PLUG[0], o, part)

    for name, o, _ in objs.values():
        for k, v in SETTINGS.get(name, {}).items():
            if meta(o, k) is None:
                meta(o, k, v)

    # maillages : un export par pièce, en parallèle
    with ThreadPoolExecutor() as ex:
        new = dict(zip(PARTS, ex.map(lambda ia: export(ia[1], work / f"part{ia[0]}.3mf"), enumerate(PARTS.values()))))

    done = set()
    for oid, (name, o, part) in objs.items():
        comp = re.search(rf'<object id="{oid}"[^>]*>\s*<components>\s*<component p:path="([^"]+)" objectid="(\d+)"', model)
        f, mid = src / comp.group(1).lstrip("/"), comp.group(2)
        t = f.read_text()
        old = re.search(rf'(<object id="{mid}"[^>]*>\s*)(<mesh>.*?</mesh>|<mesh/>)', t, re.S)
        lo_old = min((float(z) for z in re.findall(r'z="([^"]+)"', old.group(2))), default=None)
        if (f, mid) not in done:   # les pois d'une même taille partagent un maillage
            f.write_text(t[:old.start(2)] + new[name]["mesh"] + t[old.end(2):])
            done.add((f, mid))
        # le bas de la pièce reste sur le plateau ; rotations autour de Z seulement
        item = re.search(rf'(<item objectid="{oid}" [^>]*transform=")([^"]+)(")', model)
        m = [float(a) for a in item.group(2).split()]
        if max(abs(m[2]), abs(m[5]), abs(m[6]), abs(m[7])) > 1e-6 or abs(m[8] - 1) > 1e-6:
            sys.exit(f"{name} : pièce basculée dans Orca, mise à jour refusée")
        m[11] = -new[name]["lo_z"] if lo_old is None else m[11] + lo_old - new[name]["lo_z"]   # objet neuf : posé sur le plateau
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
    shutil.copy(PROJECT, PROJECT.with_suffix(".3mf.bak"))
    shutil.move(out, PROJECT)
    shutil.rmtree(work)
    counts = {}
    for n, _, _ in objs.values():
        counts[n] = counts.get(n, 0) + 1
    print(f"{PROJECT.name} : {len(objs)} objets mis à jour {counts} ; ancien fichier : {PROJECT.name}.bak")


main()
