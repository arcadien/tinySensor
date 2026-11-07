# Backlog de mise à niveau — carte tinySensor

Revue du 2026-11-06 sur la branche `hw/schematic-upgrade`, faite avec KiCad 10.0.7 (`kicad-cli`) : ERC, netlist, DRC avec parité schéma, et comparaison avec le firmware (`lib/hal/Attiny84aHal*`).
Sévérités : **Critique** / **Majeur** / **Mineur** / **Question** / **Évolution**. Effort : S (≈ 1 h), M (une demi-journée), L (une refonte).

État à l'ERC : 0 erreur et 47 avertissements. 39 sont des symboles embarqués périmés. Deux seulement signalent un vrai défaut, et ce défaut est R1.
La parité schéma ↔ PCB remonte 39 écarts (voir R6).

Rapports complets, ERC/DRC et images : `review/2026-11-06/`. Reprise des cartes v2.2 déjà fabriquées : `REWORK_v2.2.md`.

## Schéma

| ID | Sév. | Zone | Constat | Correctif | Effort |
|---|---|---|---|---|---|
| R1 | ✅ v3.0 (Critique (v2.2 : reprise)) | Alim / MCU | PA1 (U1.12, mesure batterie) est relié directement à +BATT, sans résistance ni pont diviseur. Avec une batterie au-dessus de VCC + 0,5 V (Li-ion, 18650, J13 RAW IN au-dessus de 3,8 V), la diode de clamp conduit sans limite de courant : le MCU peut être détruit, ou VCC alimenté par PA1. La lecture sature aussi à 1023. ERC : `pin_to_pin` et `multiple_net_names` +BATT/Battery_Checker. | Résistance série de 10–100 k et 100 nF vers GND. Si la batterie peut dépasser VCC, un pont diviseur, éventuellement commuté par PA2, et adapter `ConvertAnalogValueToMv`. | S |
| R2 | ◐ v3.0 (Majeur) | Alim | Aucune protection contre l'inversion de polarité. J10 (CR2032), J11 (AAA), J12 (AA), J13 (RAW IN) et le support 18650 du PCB sont tous en parallèle. Un bornier inversé met une tension négative sur le DC-DC et le MCU. Deux piles posées ensemble se rechargent l'une l'autre, ce qui est dangereux pour la CR2032. | P-MOSFET en idéal-diode, ou Schottky sur RAW IN. Marquer les supports comme exclusifs (DNP, mention sérigraphiée). | M |
| R3 | ✅ Corrigé | Alim / firmware | En hibernation, `sleep()` passe PA2 en entrée flottante (`DDRA = 0`, `Attiny84aHal.cpp:85`). La même fonction active les pull-ups internes de PA4/PA5/PA6 (l.89), et PA3 garde le sien depuis l'init (l.42). SENSOR_VCC est alors alimenté à travers R2/R3/R6 : les capteurs ne sont jamais vraiment coupés, et le BME280 n'a jamais de reset à la mise sous tension propre. | Firmware : en sommeil, PA2 en sortie à 0, PA3/PA4/PA6 en entrée sans pull-up. | S |
| R4 | ✅ Corrigé | Radio | Le module radio est alimenté en permanence (J1.2 sur VCC), et sa broche DATA (PB0) flotte en sommeil (`DDRB = 0`). Un émetteur OOK peut s'activer tout seul. Le commentaire `Attiny84aHal.h:9` dit le contraire du schéma. | Firmware : PB0 en sortie à 0 en sommeil. Option : un pull-down de 100 k sur TX_RADIO. Corriger le commentaire. | S |
| R5 | ✅ v3.0 (Majeur) | LED | D1 est montée à l'envers (anode sur GND, cathode vers R1/PB1), et l'empreinte PCB est cohérente avec cette erreur. Le firmware met PB1 à 1 pour l'allumer, donc la LED ne s'allume jamais. | Inverser D1 au schéma et au PCB. | S |
| R6 | ✅ v3.0 (Majeur) | Synchro | Le schéma n'est plus la référence. Le commit c8df224 a ajouté I2C et I2C_SPI (JST-EH 6 broches), JP5, JP6 et le support BH-18650 dans le PCB seulement. J3, J11, J12 et MH3 manquent au PCB, et MH1 y est en double. Il reste deux pastilles non connectées : JP6 vers MISO, et I2C.6 vers la zone GND. Le pôle − du 18650 semble sans net. | Reporter ces éléments dans le schéma, puis lancer « Update PCB from Schematic » jusqu'à une parité propre. Vérifier le 18650. | M |
| R7 | ✅ v3.0 (Mineur) | MCU | RESET n'a pas de pull-up externe : seul le pull-up interne, faible, et un bouton. | 10 k vers VCC, sans condensateur, pour rester compatible debugWIRE. | S |
| R8 | Mineur | Alim | Le découplage global (2 × 10 µF) est faible pour une CR2032, dont la résistance interne est de 10–20 Ω, face au pic de courant de la radio. | 100–470 µF à faible fuite pour les versions CR2032. | S |
| R9 | ✅ v3.0 (Mineur) | Alim | JP2 « DIRECT Vcc » court-circuite l'entrée et la sortie du DC-DC si J2 est monté. | Mention « JP2 fermé ⇒ J2 non monté ». Le firmware doit alors utiliser `BATTERY_IS_VCC`. | S |
| R10 | Question | Alim | Le module DC-DC J2 n'a pas de broche d'activation. Son courant de repos (souvent 10–100 µA) dépasse probablement celui de l'ATtiny en sommeil. | **Décidé (2026-11-07) :** module TPS63020 abaisseur-élévateur (stock Tars), relié par fils aux pastilles de J2 comme dans `REWORK_v2.2.md`. ◐ v3.0 : J2 devient une barrette 1×03 (1 +BATT, 2 GND, 3 VCC) aux mêmes pastilles ; le module (TPS63020, ou une alimentation 5 V) sera choisi quand on fabriquera une v3.0. Mesurer son courant à vide (< 0,1 mA). | M |
| R11 | ✅ v3.0 (Question) | I2C | La broche CSB du BME280 (J3.5) est en l'air ; si elle est vue à 0, le capteur passe en SPI. SDO est à GND, d'où l'adresse 0x76, conforme au firmware. | Relier CSB à SENSOR_VCC, ou vérifier que le breakout la tire vers le haut. SDO sert à choisir l'adresse en I2C (GND = 0x76, celle du firmware), et c'est la sortie de données en SPI : décidé, c'est un choix fait au montage via JP6 (voir P4), avec une note au schéma. Attention : à l'init, le firmware active le pull-up de PA5, donc SDO câblé sur MISO en I2C donnerait 0x77. | S |
| R12 | Question | I2C | Le BH1750 (`USE_BH1750`, 0x23, env SOLAR_TEST) n'apparaît pas au schéma. | Le documenter sur le second connecteur I2C une fois R6 fait, avec ADDR à 0. | S |
| R13 | Question | Alim | `USE_CHARGE_PUMP` (`platformio.ini:66`, `:81`) n'est utilisé par aucun code et ne correspond à aucun circuit. Le chargeur TP4056 n'est pas au schéma. | Supprimer le flag ou documenter le matériel concerné. | S |
| R14 | Question | MCU | Les fusibles ne sont pas consignés, en particulier BODLEVEL. Le firmware coupe le BOD en sommeil. | Consigner les fusibles voulus dans le dépôt (BOD 1,8 V à 1 MHz). | S |
| R15 | ◐ v3.0 (Mineur) | Bibliothèques | `fp-lib-table` pointe vers `battery.pretty` et `Chinese breakouts.pretty`, qui sont absents. `sym-lib-table` déclare une bibliothèque Legacy qui n'existe pas. J13 utilise une empreinte retirée de KiCad, et J1/J3/J4 des symboles `Conn_*_Male/Female` retirés. U1 et J6 viennent de `-rescue.lib`. Le fichier est au format KiCad 7. | Nettoyer les tables. Utiliser les symboles et empreintes standards (`ATtiny84A-P`, `AVR-ISP-6`, `Conn_01x0N_Pin/Socket`, `TerminalBlock_Phoenix` 5,08), puis « Update Symbols from Library ». | S–M |
| R16 | ◐ v3.0 (Mineur) | Connecteurs | J1 est câblé GND/VCC/DATA, alors qu'un FS1000A est DATA/VCC/GND. J5 est câblé VCC/GND/DQ, alors qu'un DS18B20 TO-92 est GND/DQ/VDD. La pastille 1 de l'ISP est ronde, donc rien ne la repère. | Fonctions des broches en sérigraphie, et pastille 1 carrée sur l'ISP. | S |
| R17 | ◐ v3.0 (Mineur) | Hygiène | Valeurs « 4,7k » et « 4k7 » mélangées. JP3 a la valeur « ~ ». C5, une céramique de 1 nF, utilise une empreinte tantale, et C6 (0,1 µF) un symbole polarisé. La note de R4 (75R ou 18k) n'est pas claire. Un fil pendouille en (54,61 ; 120,65). Le cartouche indique Rev 2.3, la carte v2.2. | Normaliser les valeurs, passer C5 en `C_Disc`, supprimer le fil. | S |
| R18 | Évolution | v2.3 | Pistes pour une v2.3 : ATtiny84A-SSU ou ATtiny1614 (UPDI, TWI matériel, RTC basse conso), émetteur OOK pour 1,8–3,6 V (STX882, SYN115), boost nano-power avec enable, protection idéal-diode, pont diviseur commuté. | — | L |

Points non vérifiés : les pull-ups internes aux breakouts, le courant de repos réel du DC-DC, les valeurs des fusibles.

## PCB

Le circuit fait 2 couches, 1,6 mm, 93,98 × 63,75 mm, avec un plan GND en B.Cu. Tous les composants sont traversants, sauf JP2–JP6 (CMS en B.Cu).
La DRC, avec parité et re-remplissage des zones, remonte 42 violations, 2 non-connexions et 39 écarts de parité. Environ 70 de ces messages viennent des bibliothèques ou des champs.
La variante découpée pour le champignon a été vérifiée sur une copie découpée en x 147 / y 87 : la masse reste continue, et MH1, MH5, J1, I2C et J13 tombent exactement aux cotes de `pcb.scad`.
Le TP4056 est un module externe relié à J13 : le courant de charge ne passe pas par la carte.

| ID | Sév. | Zone | Constat | Correctif | Effort |
|---|---|---|---|---|---|
| P1 | ✅ v3.0 (Critique) | Masse | Les pastilles GND du connecteur « I2C » (pads 2 et 6) et de l'ISP J6 (pad 6) sont sur un îlot de cuivre B.Cu isolé, entouré par les pistes VCC, SENSOR_VCC et MISO et par le texte cuivre. DRC : `unconnected_items` I2C.6 ↔ zone GND, `starved_thermal` I2C.2. Le BME280 du champignon se branche justement là : **il n'a pas de masse**. | Piste GND en F.Cu de I2C.6 à J6.6 puis vers le plan, ou un via GND dans l'îlot. Relancer la DRC jusqu'à 0 non-connexion. | S |
| P2 | ✅ v3.0 (Critique) | Alim | Le pôle − du support 18650 (`REF**` BH-18650, pad 2) n'a pas de net, et le support n'existe pas au schéma. La carte entière ne peut pas fonctionner sur 18650. | L'ajouter au schéma (BT1, − sur GND), mettre le PCB à jour, sérigraphier +/−. | S |
| P3 | ✅ v3.0 (Majeur) | Synchro | Mêmes écarts que R6. MH1 est en double, en (203 ; 52) et en (203,75 ; 105,51), ce qui rend ambiguë la désignation des trous utilisée par le boîtier. | Traité avec R6, en ré-annotant aussi les trous de fixation. | M |
| P4 | ✅ v3.0 (Majeur) | Netlist | JP6 relie MISO à MISO, et JP5 GND à GND : ces cavaliers ne servent à rien. La pastille 5 de I2C n'a pas de net. I2C_SPI met MISO en broche 5, alors que le brochage d'un breakout BMx280 est VCC/GND/SCL/SDA/CSB/SDO. | Décidé : JP6 devient un sélecteur à 3 pastilles pour la broche 5 du connecteur capteur (SDO), côté GND pour l'I2C à 0x76, côté MISO pour le SPI. Ajouter une note au schéma pour la personne qui monte la carte. JP5 (GND–GND) est à supprimer ou à justifier. Lié à R11. | S |
| P5 | ◐ v3.0 (Majeur) | Découpe | Les lignes de découpe traversent du cuivre : le plan GND, et deux pistes +BATT en F.Cu qui coupent x = 147 (vers y 59,3) et y = 87. Un bord scié met +BATT et GND à nu, et une bavure peut les court-circuiter en extérieur. | Zones d'exclusion cuivre de 1,5 mm sur les deux couches le long des deux lignes. Faire passer +BATT par un pont ou un cavalier hors de la partie conservée. | S |
| P6 | ✅ v3.0 (Majeur) | Fab | `min_copper_edge_clearance` est à 0,05 mm, et le plan GND arrive à 0,05 mm du bord. Les fabricants demandent au moins 0,2–0,3 mm, et le cuivre à nu s'oxyde dehors. | Régler le dégagement à 0,5 mm, re-remplir, relancer la DRC. | S |
| P7 | Majeur | Méca | MH5 (150,89 ; 51,84) est sous le module DC-DC J2, alors que le champignon visse la carte par MH5. Une fois J2 soudé, le tournevis n'y accède plus. | Déplacer MH5 hors de J2 (et mettre `pcb.scad` à jour) ou déplacer J2. À vérifier : le module a-t-il un trou aligné ? ✅ v3.0 : J2 n'est plus qu'une barrette 1×03, la vis de MH5 reste accessible, `pcb.scad` inchangé. | M |
| P8 | ✅ v3.0 (Majeur) | Méca | C6 (tantale traversant) est en B.Cu, alors que `pcb.scad` le modélise sur le dessus. Debout, il touche le plancher (`pcb_gap` = 5 mm). | Passer C6 en F.Cu, le coucher, ou corriger le modèle. | S |
| P9 | ◐ v3.0 (Majeur) | Montage | Rien n'indique les broches en sérigraphie : pas de +/− sur J13 RAW IN, ni de fonction des broches sur J1, J5 ou J7. Tous les textes sont en cuivre, pas en sérigraphie. | Textes en F.Silkscreen (au moins 1 mm). +/− sur J13, G/V/D sur J1, V/G/Q sur J5. Recoupe R16. | S |
| P10 | ✅ v3.0 (Majeur) | LED | Même constat que R5 : l'anode de D1 est sur GND. | Voir R5. | — |
| P11 | ✅ v3.0 (Mineur) | Méca | Les trous de fixation font 2,5 mm de diamètre, trop justes pour du M2,5. `pcb.scad` les modélise en Ø2,7. | `MountingHole_2.7mm_M2.5`. | S |
| P12 | Mineur | Découpe | J2 (x 147,57) et J13 (≈ 147,7) frôlent la ligne x = 147 : un trait de scie de 1 mm peut les entamer. | Ligne en x ≈ 145,5 (le support CR2032 s'arrête vers 141) et mettre `k_crop` à jour, ou décaler J2/J13. À vérifier : les fils de J13 entrent-ils côté découpe ? | S |
| P13 | ✅ v3.0 (Mineur) | Alim | Le découplage de la radio est loin : C4 (10 µF) est à environ 11 mm de J1, et il n'y a pas de 100 nF. | 100 nF entre J1.1 et J1.2, et C4 à moins de 5 mm. | S |
| P14 | ✅ v3.0 (Mineur) | Masse | La broche 14 de U1 n'est pas reliée au plan (`zone_connect 0`). Sa masse passe seulement par C2 et JP5. | Connexion thermique au plan. | S |
| P15 | Mineur | Alim | Les pistes +BATT font 0,25–0,3 mm, soit environ 0,09 Ω sur 45 mm. Ça suffit pour environ 100 mA. | Les élargir à 0,5–0,8 mm, pour la marge. | S |
| P16 | ✅ v3.0 (Mineur) | Montage | Une barrette femelle au pas de 2,54 dans une empreinte JST-EH au pas de 2,50 accumule 0,2 mm d'erreur sur 6 broches, avec des perçages de 0,95 mm. | Perçages de 1,0–1,1 mm, ou une empreinte double pas. | S |
| P17 | ◐ v3.0 (Mineur) | Hygiène | Références masquées : U1, C2, C3, C6, D2, D3, R6, J2, J7, I2C, I2C_SPI. C5 (1 nF) est sur une empreinte tantale marquée « + ». Le masque rogne la sérigraphie de SW1 et J10, et `min_silk_clearance` est à 0. Il reste des bouts de piste orphelins : GND (202,8 ; 97,0) et VCC (162,7–163,1 ; 61,7). Les bibliothèques sont cassées (voir R15). | Nettoyer. | S |
| P18 | Question | Radio | Quel module va dans J1, et dans quel sens ? Un module de 19 mm debout peut gêner J5. Il n'y a pas de piste RF, donc le plan de masse n'est pas en cause. | — | — |
| P19 | Question | Capteur | Dans la variante boîtier, le connecteur capteur est à 7 mm de U1, au-dessus du plan GND. C'est négligeable vu le rapport cyclique. Dans le champignon, le BME280 pend sous la carte : c'est bien. | — | — |
| P20 | Évolution | v2.3 | Pistes : vraies languettes ou mouse-bites avec exclusion cuivre, trous de fixation hors des modules, une seule entrée batterie protégée (voir R2 : CR2032, 18650 à 4,2 V et entrée boost sont en parallèle), empreinte BH1750, fente d'isolation thermique autour du capteur. | — | L |

Points non vérifiés : rien n'a été ouvert dans l'interface graphique, les Gerbers et le fichier de perçage n'ont pas été exportés, et les collisions 3D entre modules et barrettes n'ont pas été contrôlées. Les dimensions du module radio et le côté d'entrée des fils de J13 sont supposés.

## Ordre suggéré, schéma et PCB ensemble

1. **R3, R4 et R5/P10**, le firmware et la LED : la règle PLAT-POWER-003 est à relire.
2. **P1** : sans masse, le BME280 du champignon ne marche pas. C'est à corriger avant toute fabrication.
3. **R1 et R2/P2**, la sécurité de l'alimentation.
4. **R6/P3/P4** : remettre le schéma en référence avant toute autre modification.
5. **P5, P6, P7, P8, P12**, les points de fabrication et de mécanique du champignon.
6. Le reste.

## Firmware, suite de la revue

| ID | Sév. | Constat | Correctif | Effort |
|---|---|---|---|---|
| F1 | Majeur | Avec le pont diviseur ajouté par la reprise v2.2 (R1), et dans la v3.0, PA1 ne reçoit plus que la moitié de la tension batterie. | Ajouter un réglage de rapport de pont (par exemple `BATTERY_DIVIDER`) dans `main.cpp`, ou dans `ConvertAnalogValueToMv`, via le workflow RBD. | S |

## Révision 3.0 (2026-11-06)

Le schéma et le PCB sont estampillés **Rev 3.0** (cartouche et sérigraphie « TinySensor v3.0 »). Ils ont été générés à partir de la v2.2 par `scripts/make_v3_pcb.py`, `scripts/make_v3_sch.py` et `scripts/sync_paths.py` (KiCad 10, `pcbnew` en Python).
DRC avec parité schéma : 0 erreur d'empreinte, 0 pastille non connectée. La seule erreur restante est la zone d'occupation de J10, mal formée dans la bibliothèque. ERC : aucune erreur, seulement des avertissements de bibliothèque hérités.

**Corrigé :**
- **R5/P10 :** D1 tournée.
- **R1 :** PA1 passe par le pont R7/R8 de 1 MΩ, avec C7 de 100 nF ; la broche lit Vbatt / 2 (voir F1).
- **R2 :** Q1 (AO3401A) protège contre l'inversion sur J13 et BT1.
- **P1 :** la masse des connecteurs capteurs est reliée au plan.
- **P2 :** BT1 18650 est câblé, avec son pôle − à GND.
- **R6/P3 :** J3 et J14 remplacent les connecteurs « I2C » et « I2C_SPI » du PCB. J11, J12 et JP5 sont supprimés, et le MH1 en double devient MH3.
- **R11/P4 :** JP6 sélectionne SDO (broche 6), avec 1-2 = GND par défaut et 2-3 = MISO.
- **R7 :** R9 de 10 kΩ en pull-up sur RESET.
- **P13 :** C8 de 100 nF à la radio.
- **P8 :** C6 passe en 0805 sur le dessus.
- **P11 :** trous M2,5 de 2,7 mm.
- **P16 :** perçages de 1,05 mm.
- **P6 :** 0,5 mm entre le cuivre et le bord.
- **P5 (en partie) :** pas de plan de masse le long des traits de découpe. Deux ponts GND traversent ces bandes, et deux pistes +BATT y passent encore.
- **P9 (en partie) :** les textes sont passés en sérigraphie, avec « + » sur J13, G/V/D sur J1, V/G/Q sur J5 et la mention « 1 seule source batterie ».
- **R15 (en partie) :** les tables de bibliothèques sont nettoyées, et les empreintes de J13 et BT1 sont copiées dans `library/`.

**Reste ouvert pour la v3.0 :**
- **P7 :** ✅ J2 est une barrette 1×03 (`scripts/v3_j2_header.py`), MH5 reste accessible.
- **P12 :** J2 et J13 restent près de x = 147.
- **P15 :** pistes +BATT à élargir.
- **R8, R12, R13, R14, R18/P20 :** non traités.
- **R16 :** pastille 1 carrée sur l'ISP.
- **R17 :** note de R4 et fil pendant.
- **Hygiène :** 28 empreintes ne correspondent plus à leur version en bibliothèque, et les symboles hérités (rescue) sont à remplacer.
- **Fabrication :** relecture dans l'interface graphique, export des Gerbers et du fichier de perçage. Aucun des deux n'a été fait.
- **Schéma :** quelques symboles d'alimentation recouvrent des textes (lisibilité seulement).
- **F1 :** le firmware doit multiplier la lecture de PA1 par 2.

