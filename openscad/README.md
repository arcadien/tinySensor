# Boîtiers imprimés 3D — TinySensor v2.2

Modèles OpenSCAD paramétriques (testés avec OpenSCAD 2021.01). Matière visée : **PETG**.

| Au mur | Sur pied |
|---|---|
| ![Vue d'ensemble au mur](rendu.png) | ![Vue d'ensemble sur pied](rendu_pied.png) |

![Boîtier ouvert : carte et accu 18650](rendu_ouvert.png)

Pièce de 2 € (Ø25,75 mm) dans les rendus pour l'échelle.

| Fichier | Contenu |
|---|---|
| `tinysensor_box.scad` | Boîtier mural principal (base + couvercle) |
| `sensor_shield.scad` | Abri de sonde extérieur pour breakout BMx280 + clip optionnel |
| `stand.scad` | Pied de table : mât + semelle, pour poser boîtier + abri debout |
| `pcb.scad` | Représentation simplifiée de la carte v2.2 et de l'accu 18650 (rendus uniquement) |
| `euro2.scad` | Pièce de 2 € pour l'échelle des rendus |
| `assembly.scad` | Vue d'ensemble (boîtier + abri clipsé, au mur ou sur pied), sert à générer `rendu.png` et `rendu_pied.png` |

> Statut : **non imprimé, non validé**. Jeux et serrages à confirmer au premier tirage.

## Export

OpenSCAD exporte directement en 3MF (ou STL) :

```bash
# Boîtier
openscad -D 'part="base"' -D 'usage="ext"' -o out/base_ext.3mf tinysensor_box.scad
openscad -D 'part="base"' -D 'usage="int"' -o out/base_int.3mf tinysensor_box.scad
openscad -D 'part="lid"'                   -o out/lid.3mf      tinysensor_box.scad
# Abri de sonde
openscad -D 'part="cap"'   -o out/shield_cap.3mf   sensor_shield.scad
openscad -D 'part="plate"' -o out/shield_plate.3mf sensor_shield.scad   # x4
openscad -D 'part="base"'  -o out/shield_base.3mf  sensor_shield.scad
openscad -D 'part="clip"'  -o out/shield_clip.3mf  sensor_shield.scad   # option
# Pied de table (option)
openscad -D 'part="mast"' -o out/stand_mast.3mf stand.scad
openscad -D 'part="foot"' -o out/stand_foot.3mf stand.scad
```

Rendu boîtier ouvert : `openscad --preview -D 'part="open"' --camera=110,-230,420,110,30,0 --imgsize=1400,1000 -o rendu_ouvert.png tinysensor_box.scad`.

Toutes les pièces sont exportées dans leur orientation d'impression et s'impriment **sans support**. `out/` est ignoré par git.

## Boîtier principal (`tinysensor_box.scad`)

- Calé sur le PCB v2.2 (`hardware/tinysensor/tinySensorv1.kicad_pcb`) : 93,98 × 63,75 mm, fixation par les 4 trous M2.5 des coins sur entretoises de 5 mm.
- Dégagement au-dessus du PCB : 25 mm (gros condensateurs), paramètre `comp_h`.
- Baie libre de 14 mm côté bas (côté support 18650) pour câbles et antenne.
- Encombrement : ~117 × 101 × 39 mm (rebord compris, couvercle inclus).
- **Fermeture** : 6 vis M3 auto-taraudeuses, hors de la zone joint.
- **Étanchéité** : gorge 2,2 × 1,5 mm pour cordon Ø2 (TPU imprimé ou EPDM), écrasé par une nervure du couvercle.
- **Fixation murale** : 2 pattes, vis Ø4 tête fraisée. Le fond reste plein.
- Variantes (`usage`) :
  - `ext` : trou Ø12,5 pour presse-étoupe **PG7** (sonde déportée). Par défaut en **paroi latérale gauche**, au niveau de la baie, sous le chanfrein du rebord qui l'abrite de la pluie ; `gland_side` = `left` / `right` / `bottom` (ancienne position, paroi basse). Le câble redescend le long du boîtier puis remonte dans l'abri par le fond : prévoir ~15 cm de plus et une boucle d'égouttement.
  - `int` : fentes d'aération haut et bas (convection).
- La patte basse porte deux encoches latérales pour le clip de l'abri (`shield_notch`).

## Abri de sonde (`sensor_shield.scad`)

Abri à assiettes (type Stevenson) : en extérieur, un BMx280 enfermé dans le boîtier étanche mesurerait l'air du boîtier, pas l'air ambiant.

- Chapeau plein + 4 assiettes ajourées + fond, Ø56 mm, pas de 10 mm, hauteur ~52 mm.
- Assemblage : 3 tiges filetées **M3** traversant les entretoises, écrous sous le fond.
- Fond : glissière verticale pour le breakout **15 × 11 mm, 6 broches** (empreinte `BMx280_breakout`), passe-câble Ø6 dessous, 6 trous de drainage.
- Imprimer en **blanc** (limite l'échauffement solaire). Poser à l'ombre si possible.

### Fixation (au choix)

1. **Murale** : platine à 2 vis Ø4 fraisées côté sonde. Visser le chapeau au mur **avant** d'empiler les assiettes.
2. **Clip sous le boîtier** (`part="clip"`) : adaptateur boulonné derrière la platine (2 vis M4×8 tête fraisée + écrous noyés côté mur). Ses deux branches enserrent la patte murale basse du boîtier (lèvres à 45°) et un bossage s'enclenche dans chaque encoche. Pose : visser le boîtier, glisser l'abri vers le haut jusqu'au clic.
   - Serrage réglable : `bump_r` (accroche), `prong_w` (souplesse).

3. **Sur pied** (`stand.scad`) : voir ci-dessous.

## Pied de table (`stand.scad`)

Pour poser l'ensemble boîtier + abri debout sur une table, sans mur.

- **Mât** 30 × 6 × 212 mm : il tient lieu de mur. Le boîtier s'y visse par ses deux pattes murales (M4 tête fraisée, écrous noyés dans le dos du mât) ; l'abri reste clipsé sous la patte basse, comme au mur.
- **Semelle** 130 × 136 mm avec douille de 25 mm et goussets : le mât s'y emboîte et se bloque par une vis M4 traversante. Elle déborde de 90 mm vers l'avant pour équilibrer l'abri, qui avance jusqu'à ~72 mm du mât.
- 30 mm de dégagement sous l'abri (`under_clr`) pour la boucle du câble qui remonte par le fond.
- Le mât (212 mm) tient sur un plateau 220 × 220.
- Les cotes du boîtier et de l'abri sont recopiées en tête de fichier : à réaligner si on modifie les autres modèles.

### Câblage

- Liaison I2C < 1 m : pull-ups 4,7 kΩ, câble torsadé (SDA/GND, SCL/GND).
- Le câble sort du presse-étoupe et remonte dans l'abri par le fond.

## Visserie

| Usage | Qté | Référence |
|---|---|---|
| PCB → entretoises | 4 | M2.5 × 6 auto-taraudeuse |
| Couvercle | 6 | M3 × 8 auto-taraudeuse |
| Boîtier → mur | 2 | Ø4 tête fraisée + chevilles |
| Presse-étoupe | 1 | PG7 |
| Joint | ~45 cm | cordon Ø2 TPU/EPDM |
| Abri : assemblage | 3 | tige filetée M3 ~60 mm + 6 écrous |
| Abri → mur (option 1) | 2 | Ø4 tête fraisée |
| Clip → abri (option 2) | 2 | M4 × 8 tête fraisée + écrous |
| Boîtier → mât (pied) | 2 | M4 × 10 tête fraisée + écrous |
| Mât → semelle (pied) | 1 | M4 × 20 + écrou |

## Alimentation (à trancher)

- Support 18650 sur le PCB.
- Boîtiers AA/AAA disponibles : à intégrer dans la baie ou dans un compartiment séparé (dimensions à fournir).
- Solaire : le TP4056 (chargeur linéaire, sans MPPT) est peu adapté ; préférer un CN3065 ou un CN3791.
