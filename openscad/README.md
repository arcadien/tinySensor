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
| `mushroom.scad` | Boîtier « champignon » décoratif (en plus du boîtier mural) |
| `harness.py` | Générateur du plan de câblage imprimable du champignon (`python3 harness.py` → `faisceau_champignon.pdf`, A4) |
| `assembly.scad` | Vue d'ensemble (boîtier + abri clipsé, au mur ou sur pied), sert à générer `rendu.png` et `rendu_pied.png` |

> Statut : **non imprimé, non validé**. Jeux et serrages à confirmer au premier tirage.

## Export

Les pièces prêtes à trancher sont dans `print/`, **un projet OrcaSlicer par modèle**, une plaque par usage et chaque objet nommé :

| Projet | Plaques | Objets |
|---|---|---|
| `box.3mf` | Fond extérieur, Fond intérieur, Couvercle | n'imprimer qu'un des deux fonds : `ext` (presse-étoupe, extérieur) ou `int` (aérations, intérieur) |
| `shield.3mf` | Abri, Assiettes | Chapeau, Fond, Clip (optionnel) ; 4 assiettes |
| `stand.3mf` | Mât, Pied | pied de table, optionnel |
| `mushroom.3mf` | Ailettes, Hat, White buttons, Ground, Divers, Foot, Support | Chapeau, Lamelles, Châssis, Plancher, Pied, Sol, Berceau, Bouchon, Pois Ø6 / Ø8 / Ø10 |

`mushroom_cap.3mf` : chapeau du champignon tel qu'imprimé (réglages Orca de l'impression). Snapshot figé, hors de `export.sh`.

Après une modification d'un `.scad` (environ 30 s, fermer les projets dans Orca avant) :

```bash
./export.sh                  # tous les projets
./export.sh print/box.3mf    # ou un seul
```

`export.sh` lance `update_3mf.py`, qui réexporte chaque pièce et remplace son maillage dans le projet : plaques, positions, rotations et réglages Orca sont gardés, l'ancien projet reste en `print/<projet>.3mf.bak`. Un projet absent est recréé, avec les réglages d'imprimante et de filament de `mushroom.3mf` (Ender 3). Les pièces ne doivent être tournées qu'autour de Z dans Orca, sinon le script refuse de les mettre à jour ; une pièce supprimée dans Orca est signalée, pas remise. Réglages par objet posés s'ils manquent : chapeau du champignon brim extérieur 10 mm, remplissage 10 % ; pied du champignon 6 parois. Les nombres de pois sont ceux du projet : l'écho « Pois » de `mushroom.scad` donne le compte à placer si la graine ou `dot_gap` changent.

Export d'une pièce hors projet (STL, autre format) : `openscad -D 'part="lid"' -o out/lid.3mf tinysensor_box.scad` (voir l'en-tête de chaque `.scad`).

Rendu boîtier ouvert : `openscad --preview -D 'part="open"' --camera=110,-230,420,110,30,0 --imgsize=1400,1000 -o rendu_ouvert.png tinysensor_box.scad`.

Toutes les pièces sont exportées dans leur orientation d'impression et s'impriment **sans support**, y compris le dôme du champignon. `out/` est ignoré par git, pas `print/`.

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

## Champignon (`mushroom.scad`)

Variante décorative, type amanite, qui ne remplace pas le boîtier mural. Le chapeau est un tout (dôme + lamelles) posé sur le pied avec une légère inclinaison ; toute l'électronique est dans le chapeau, le pied ne porte que l'accu.

| Vue d'ensemble | Pied | Dessous | Coupe |
|---|---|---|---|
| ![Champignon](rendu_champignon.png) | ![Champignon, pied](rendu_champignon_pied.png) | ![Champignon, dessous](rendu_champignon_dessous.png) | ![Champignon, coupe](rendu_champignon_coupe.png) |

Pièces :

- **Dôme** (`cap`) ~147 × 147 × 190 mm (rayon maxi moyen 72) : **gourde** d'après croquis, méridien `cap_shape` (bas large, taille Ø~55, haut Ø~72), **contour lobé** issu d'un générateur (voir ci-dessous). À partir de la zone qui se resserre (`cap_neck`), le haut penche de 8° vers −X (`cap_lean`, contre l'inclinaison du chapeau), pour ramener la masse vers l'axe du pied : sommet à ~6 mm de l'axe du pied côté +X. La carte est tournée de 180° dans le châssis pour que le sommet reste à l'aplomb de **J1 « RADIO »** (module 433 MHz), selon l'axe de la carte (`cap_top` est calculé), à 126 mm au-dessus de la carte. **Évent** Ø5 (`vent_d`) : de la pointe du plafond, il descend de ~15° vers le flanc arrière (+Y), sans pois autour ; la pluie n'entre pas et l'air chaud sort, à l'opposé de J1. Le gros du bas part à l'opposé, côté évasement du pied. Plafond intérieur conique à 45°, sans support : le haut est plein sous le sommet. Son bord descend de 4 mm sous le disque à lamelles (cache sa tranche) et s'épaissit vers l'intérieur à **6 mm de large** (`rim_w`, raccord à 45°) : ~2,4 fois plus de surface sur le plateau pour une pièce de 190 mm de haut, silhouette inchangée. Fixation : 3 **consoles** sur la paroi intérieure, juste au-dessus du disque (`corbel_*` : 12 mm de large, face droite de 10 mm puis chanfrein à 45° jusqu'à la paroi, trou pilote Ø2,5 × 8) ; le disque monte dans le dôme et y est plaqué par 3 vis M3 × 10 auto-taraudeuses par en dessous, têtes sous le disque entre les lamelles. ![Coupe par une console](rendu_champignon_console.png)
- **Lamelles** (`gills`) : disque qui ferme le dessous du chapeau, au contour du plus étroit de la jupe du dôme moins 0,4 mm (`disc_fit`) pour passer les creux des lobes, 48 lames radiales plus profondes près du pied (`gill_depth`), fentes de ventilation entre les lames sous la chambre du capteur (y compris côté +X, à 0° et ±7,5°, au ras du bord +X du BME280), ouverture de la prise USB, fourreau central qui reçoit le tenon du pied. S'imprime lames vers le haut.
- **Châssis** (`chassis`) vissé sur le disque (4 vis M3 par en dessous), imprimé debout :
  - chambre du bas (48 mm, `chamber_h`), ventilée : **BME280 enfiché sous la carte**, sans fil ni glissière (voir *Câblage*) ; le breakout pend parallèle à la carte, capteur vers le disque, à 42 mm au-dessus du disque ; fenêtres du châssis en haut de la chambre (`win_h`), pour que l'air passe devant le capteur. Le haut du berceau traverse le disque de 30 mm (trou Ø37, `disc_bore`) et finit 12 mm sous le breakout, avec la paire JST-XH ;
  - **plancher percé** (`deck`) sur des appuis, avec une fenêtre de 17,2 × 4,5 mm pour la barrette femelle ;
  - **carte v2.2 découpée** (60 × 39 mm, le long des pointillés de sérigraphie x = 147 / y = 87, sans les supports 18650 et CR2032) : tournée de 180° (J1 côté −X), 2 vis M2.5 (MH5, MH1 du haut), le bord bas repose sur un appui. L'accu arrive sur **J13 « RAW IN »** (`+BATT`/`GND`) ;
  - glissière du **module de charge** TP4056, vertical sur le côté, prise **micro-USB vers le bas** entre deux lamelles.
- **Pied** (`stem`) en **poire d'amanite** d'après croquis : base bulbeuse (~106 × 96 mm au ventre, Ø70 à Ø85 sur la table), col Ø44 à 50 à l'anneau, Ø45 sous le fourreau, creux (Ø29,4) jusqu'en haut : reçoit le berceau de l'accu, inséré par le haut, posé à 72 mm du sol. Dessous, une **chambre à lest** fermée qui épouse la forme du pied (paroi de 2,4 mm, `ballast_wall`, bulbe compris), plafond conique à 45° jusqu'à 70 mm (`ballast_h`) : ~220 cm³, soit ~1 kg de billes d'acier de 1 mm si on la remplit. Remplissage par un trou de Ø15 dans le fond, fermé par un **bouchon** (`plug`, Ø21 × 1,2 mm) collé dans un lamage, à fleur du dessous. Même esprit que le chapeau :
  - fût lobé (`stem_amp`, lobes qui tournent le long du fût, `stem_twist`) ; la déformation n'ajoute que de la matière, la paroi autour de l'accu ne s'amincit pas, et s'annule sous le fourreau ;
  - base bulbeuse dissymétrique (`bulb_r`, `bulb_z`, `bulb_up`, `bulb_dn`, `foot_dir`) : épaule plus marquée d'un côté, dessous arrondi à 44° au plus ;
  - anneau en **coupelle** sous le chapeau (`ring_z`, `ring_ext`, `ring_rise`), bord relevé et ondulé (`ring_amp`), 45° au plus : sans support.
  - En haut, un tenon incliné de 6° (`cap_tilt`) s'emboîte dans le fourreau, 1 vis M3 radiale.
- **Berceau** (`cradle`) 74,8 × Ø28,6 mm : gouttière amovible pour une **18650 nue** (65 mm, téton plat ; `cell_l` pour une autre longueur), sans soudure sur l'accu. Deux **butées à languette** (plaque 13 × 11 mm, languette 3 × 7 percée) glissées dans des rainures : ressort côté « − » en bas, bossage côté « + » en haut (polarité gravée sur les bouts). Les languettes traversent le fond et sortent dans le méplat, où passent les fils jusqu'au fourreau (W1 rouge 40 mm, W2 noir 110 mm, soudé → JST-XH F, voir *Câblage*) ; on soude les fils sur les languettes, pas sur l'accu. L'accu entre par la fente du dessus en comprimant le ressort (≈ 4 mm, `spring_c`). Trou de tirage Ø2,5 en haut (ficelle, nœud à l'intérieur). Imprimé à plat, méplat sur le plateau, sans support. ![Berceau : monté et nu](rendu_champignon_berceau.png)
- **Sol** (`ground`) : disque de 1 mm au contour lobé (Ø~145, `ground_R`), frange d'herbe en zigzag autour du pied, touffes de grands brins et brins isolés (`grass_*`, `fringe_h`). Pièce séparée à imprimer en vert ; le pied se colle dessus. Élargit l'appui au sol.
- **Pois** (`dot`) rapportés, collés dans des logements de 0,8 mm, répartis par tirage sur la surface réelle (écart mini `dot_gap`) : 99 (Ø6 × 45, Ø8 × 26, Ø10 × 28), l'écho OpenSCAD donne le nombre par taille.

Hauteur totale ~300 mm.

### Contour du chapeau

Le contour vu de dessus est une somme d'harmoniques (2 à 7) à phases aléatoires, pondérées autour de `lobe_n` lobes et normalisées à `lobe_amp` (écart maxi au rayon moyen, ±22 % par défaut). La déformation est pleine au bord et s'atténue vers le sommet (`lobe_top` = part qui reste en haut). Le bord reste plat (impression bord sur le plateau).

- `lobe_seed` change la forme. Planche des graines 1 à 8 (vue de dessus) : ![Graines](champignon_graines.png)
- `lobe_rot` : orientation du contour (165° pour la graine 6) ; `-1` : OpenSCAD choisit l'orientation qui laisse le plus de jeu autour de l'électronique (creux face aux petits côtés du châssis, lobe sur le chargeur). Les 3 vis du dôme se placent d'elles-mêmes dans les lobes.
- La console affiche l'**indice de jeu** : ≤ 1 = châssis, carte (composants ≤ 14 mm) et module de charge à ≥ 3 mm du dôme partout ; sinon un avertissement (augmenter `cap_R` / `cap_H` ou changer de graine). La console affiche aussi la marge des fils au-dessus du berceau ; `stem_h` et `ballast_h` ne descendent pas sous 5 mm de marge (assert).

### Impression

- Toutes les pièces sortent dans leur sens d'impression. Le **dôme** (190 mm de haut, tient sur Ender 3) s'imprime bord sur le plateau, **sans support** : imprimé en PETG (~20 h), les faces d'appui des consoles, à 7 mm du plateau, passent en pont sur plus de 20 mm avec deux angles droits. Remplissage faible (~10 %) : le haut est plein dans le modèle (~60 cm³, 76 g à 100 %, ~8 g à 10 %), juste pour se passer de supports derrière le col, et il pèse au mauvais endroit. Les autres pièces s'impriment sans support (pied : 113 mm de haut).
- Pied : **6 parois** (buse 0,4) : la paroi de la chambre à lest fait 2,4 mm (`ballast_wall`), elle est ainsi pleine sans remplissage d'interstices. 4 parois suffisent pour les autres pièces.
- Dôme en rouge (ou brun : panthère), pois en blanc, sol en vert, le reste en blanc / ivoire.
- Sol : brins et frange de 0,9 mm d'épaisseur, pointes à 0,6 mm (buse 0,4) ; le slicer peut tronquer les pointes les plus fines.

### Montage, accu

![Vue éclatée : sol et pied, accu, disque à lamelles, châssis et chargeur, plancher percé, carte + BME280, dôme](rendu_champignon_eclate.png)

0. Pied à l'envers : grenaille par le trou du fond (quelques gouttes de colle pour la figer et éviter qu'elle sonne), bouchon collé dans son lamage (époxy), puis pied collé sur le sol.
1. Carte : souder une **barrette femelle 2,54 1×6** sur le **dessous** de la carte, dans l'empreinte « I2C » ; les JST « I2C » et « I2C_SPI » ne sont pas posés (voir *Câblage*). Côté breakout, souder sa barrette mâle de façon que la puce regarde le disque une fois enfiché (corps du breakout vers l'intérieur du châssis, à vérifier selon le modèle de breakout).
2. Module de charge : souder W5 / W6 (rouge / noir, 70 mm, OUT+ / OUT− soudé → bornier à vis J13). W3 / W4 se soudent à l'étape 3 : le boîtier JST-XH (~7,4 × 5,8 mm) passe de justesse dans la fenêtre de la paroi +Y (8 mm de large), on passe donc l'extrémité nue.
3. Plancher posé dans le châssis, carte vissée par-dessus (la barrette femelle traverse la fenêtre du plancher) ; châssis retourné, BME280 enfiché par le dessous ouvert du châssis. Module de charge dans sa glissière ; W5 / W6 (rouge / noir, 70 mm, TP4056 soudé → bornier à vis) vissés sur J13 ; W3 / W4 (rouge / noir, 100 mm, TP4056 soudé → JST-XH M) : JST-XH M déjà serti, côté chambre ; pousser l'autre extrémité, nue, de l'intérieur de la chambre vers l'extérieur par la fenêtre de la paroi +Y (x = 22), puis la souder sur B+ / B− du TP4056 ; le côté JST reste dans la chambre, au-dessus du berceau. Enfin châssis sur le disque à lamelles (4 vis par en dessous).
4. Disque à lamelles poussé dans le dôme jusqu'aux 3 consoles, 3 vis M3 × 10 auto-taraudeuses par en dessous (têtes entre les lamelles). L'antenne du module 433 MHz monte droit de J1 dans le haut de la gourde.
5. Berceau : butées dans les rainures, W1 rouge 40 mm / W2 noir 110 mm soudés sur les languettes (W2 dans le méplat) → JST-XH F ; accu clipsé ; berceau dans le pied par le haut (ressort en bas).
6. Chapeau tenu ~50 mm au-dessus du pied, brancher la paire JST-XH, la loger dans la chambre juste au-dessus du berceau (trou central du disque), chapeau emboîté sur le tenon, vis radiale.
7. Changer l'accu : retirer la vis radiale, soulever le chapeau (~50 mm) : la paire JST-XH apparaît, la débrancher, sortir le berceau par la ficelle.

### Câblage (champignon)

Plan imprimable A4 : [`faisceau_champignon.pdf`](faisceau_champignon.pdf) (généré par `python3 harness.py`).

**Capteur, sans fil.** Le breakout BME280 (6 broches VIN GND SCL SDA CSB SDO, ~15 × 11 mm) s'enfiche directement par sa barrette mâle dans une **barrette femelle 2,54 1×6** soudée sous la carte, dans l'empreinte « I2C » (JST-EH 6, pas 2,50 : décalage cumulé de 0,2 mm sur 6 positions, absorbé par le jeu des trous Ø0,95).

- Positions 1-4 = SENSOR_VCC, GND, SCL, SDA = VIN, GND, SCL, SDA.
- Position 5 non connectée = CSB : le tirage du breakout garde le mode I2C.
- Position 6 = GND = SDO : adresse **0x76**, celle qu'attend le firmware.
- Pull-ups I2C R2 / R3 4,7 kΩ sur la carte (côté carte) ; le breakout porte aussi les siennes.
- Hauteur : femelle 8,5 mm + plastique mâle 2,5 mm ; le breakout est au-dessus du berceau, à 42 mm du disque ; l'air monte par les fentes du disque, passe devant le capteur, sort par les fenêtres hautes du châssis et l'évent du sommet.

![BME280 enfiché sous la carte](rendu_champignon_capteur.png)

**Accu** (fils 22–24 AWG : plus petit numéro = plus gros fil, W1-W4 portent le courant de charge de 1 A ; longueurs avec 15 à 25 mm de marge, à ajuster au montage) :

| # | Couleur | Signal | De (extrémité) | Vers (extrémité) | Longueur | Section |
|---|---|---|---|---|---|---|
| W1 | rouge | B+ | languette « + » du berceau (haut) — soudé | JST-XH 2p, broche 1 — JST-XH F (côté berceau) | 40 mm | 22–24 AWG |
| W2 | noir | B− | languette « − » du berceau (bas, passe dans le méplat) — soudé | JST-XH 2p, broche 2 — JST-XH F | 110 mm | 22–24 AWG |
| W3 | rouge | B+ | JST-XH 2p broche 1 — JST-XH M (côté chapeau) | TP4056 B+ — soudé | 100 mm | 22–24 AWG |
| W4 | noir | B− | JST-XH 2p broche 2 — JST-XH M | TP4056 B− — soudé | 100 mm | 22–24 AWG |
| W5 | rouge | +BATT | TP4056 OUT+ — soudé | J13 « RAW IN » borne 1 (+BATT) — bornier à vis | 70 mm | 22–24 AWG |
| W6 | noir | GND | TP4056 OUT− — soudé | J13 borne 2 (GND) — bornier à vis | 70 mm | 22–24 AWG |

Cheminement :

- Paire JST-XH (W1 rouge 40 mm / W2 noir 110 mm, languettes soudé → JST-XH F ; W3 / W4 rouge / noir 100 mm, JST-XH M → TP4056 soudé) dans la chambre, juste au-dessus du berceau : soulever le chapeau d'environ 50 mm suffit pour la débrancher par le trou central du disque.
- W3 / W4 (rouge / noir, 100 mm, JST-XH M → TP4056 soudé) sortent de la chambre du capteur par la fenêtre de la paroi +Y (x = 22) et rejoignent le TP4056 (module **avec protection** : pastilles B+ / B− / OUT+ / OUT−, micro-USB vers le bas).
- W5 / W6 (rouge / noir, 70 mm, soudé → bornier à vis) passent par-dessus la paroi +X du châssis jusqu'à J13, tout proche.
- Repérage : W3 / W4 et W5 / W6 sont deux paires rouge / noir soudées côte à côte sur le TP4056. **1 bague** de gaine thermo à chaque bout de W3 / W4, **2 bagues** sur W5 / W6 : une inversion B± / OUT± contourne la protection DW01.
- JST-XH pré-sertis : vérifier rouge = broche 1 des deux côtés (M et F) avant de souder.
- Antenne 433 MHz : droite, de J1 « RADIO » dans le haut de la gourde (sommet à l'aplomb de J1).

### Points ouverts

- Cotes à mesurer : butées (épaisseur de tôle `bt_t`, ressort comprimé `spring_c`), position de la prise du module de charge (`usb_cut`), breakout BME (`bme_w/d`, `bme_edge`, hauteur de la barrette femelle `fh_h`).
- Pas de lame d'air entre le dôme et la carte : le capteur est sous le plancher, ventilé par en dessous ; échauffement au soleil à vérifier par mesure.
- Stabilité : réglée par le lest, à vérifier une fois imprimé. Estimation (coque PETG pleine, appui r ~38 mm) : basculement vers ~15° sans lest, ~21° avec 350 g, ~28° avec la chambre pleine (~1 kg, ~1,7 kg au total, centre de gravité à ~69 mm). Un remplissage partiel suffit : à figer à la colle, pied à l'endroit.

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
| Champignon : carte → châssis | 2 | M2.5 × 6 auto-taraudeuse |
| Champignon : châssis → disque | 4 | M3 × 8 auto-taraudeuse |
| Champignon : dôme → disque | 3 | M3 × 10 auto-taraudeuse (dans les consoles) |
| Champignon : chapeau → pied | 1 | M3 × 8 auto-taraudeuse (radiale) |

## Connectique et électronique (champignon)

| Usage | Qté | Référence |
|---|---|---|
| Capteur → carte (dessous du PCB, empreinte I2C) | 1 | barrette femelle 2,54 1×6 |
| Accu ↔ chapeau | 1 | paire JST-XH 2 broches mâle / femelle (fils pré-sertis ou à sertir) |
| Charge | 1 | module TP4056 micro-USB avec protection (DW01), 28,7 × 17,15 × 0,9 mm |
| Berceau | 1 jeu | butées à languette (ressort « − » + bossage « + », plaque 13 × 11 mm) |
| Faisceau W1-W6 | ~0,5 m | fil 22–24 AWG rouge et noir |
| Soudures | — | gaine thermo |
| Accu | 1 | 18650 nu, téton plat, 65 mm |
| Capteur | 1 | breakout BME280 6 broches (adresse 0x76) |

## Alimentation (à trancher)

- Support 18650 sur le PCB.
- Boîtiers AA/AAA disponibles : à intégrer dans la baie ou dans un compartiment séparé (dimensions à fournir).
- Solaire : le TP4056 (chargeur linéaire, sans MPPT) est peu adapté ; préférer un CN3065 ou un CN3791.
