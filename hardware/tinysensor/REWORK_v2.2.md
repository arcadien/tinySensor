# Reprise des cartes v2.2 déjà fabriquées — montage champignon

Les cartes v2.2 sont déjà fabriquées. La version 3.0 corrigera les défauts à la source (voir `BACKLOG.md`).
Cette fiche dit comment rendre une v2.2 utilisable dans le champignon. Il s'agit de la carte découpée en x = 147 / y = 87, alimentée par une 18650 à travers un TP4056 relié à J13, avec le BME280 enfiché dans l'empreinte « I2C ».
Les coordonnées sont celles de KiCad, en mm, carte non tournée, vue de dessus (côté composants).

## À faire absolument

### 1. Masse du BME280 (P1) : un strap

Les pastilles GND de l'empreinte « I2C » (pads 2 et 6) et la broche GND de l'ISP (J6.6) sont sur un îlot de cuivre isolé.
Sans correction, le BME280 n'a pas de masse. Sa broche SDO, câblée au GND, flotte elle aussi, et l'adresse I2C du capteur n'est plus fixée.

- Le strap : un fil de 7 mm entre **I2C pad 6** (186,85 ; 77,11) et **I2C_SPI pad 6** (186,85 ; 83,91), juste en dessous, côté dessus de la carte. Les trous de l'I2C_SPI sont libres puisqu'on ne monte pas son connecteur, et son pad 6 est relié au plan de masse.
- Ensuite, au multimètre : bip entre I2C pad 2, I2C pad 6, J6 pad 6 et J13 borne 2.

### 2. LED D1 (R5) : la monter à l'envers de la sérigraphie

L'empreinte est inversée. Il faut mettre l'**anode (patte longue) dans la pastille carrée** (pad 1, côté R1) et la cathode dans la ronde (pad 2, GND). Aucune coupe n'est nécessaire.

### 3. Mesure batterie sur PA1 (R1) : une coupe et un pont diviseur

PA1 (U1 broche 12) est relié directement à +BATT. Avec une 18650 pleine (4,2 V), la broche reçoit plus que VCC : la mesure sature à 1023, et la diode de protection du MCU risque de conduire.

1. **Couper** la piste B.Cu de 0,3 mm qui part en diagonale de la broche 12 de U1 (174,00 ; 62,50) vers (170,19 ; 66,31), à 1–2 mm de la broche. Faire deux traits de cutter et retirer le cuivre entre les deux. Vérifier qu'il n'y a plus de continuité entre U1.12 et J13 borne 1.
2. **Gratter** le vernis de la piste sur 2 mm, juste après la coupe côté +BATT.
3. **R haute, 1 MΩ :** de U1.12 au cuivre gratté, par-dessus la coupe, côté dessous.
4. **R basse, 1 MΩ, et 100 nF :** de U1.12 à U1.14 (GND), côté dessous. Les pattes enjambent la broche 13 (PA0) : les gainer.
5. Le pont consomme environ 2 µA en continu. La valeur lue est la moitié de la tension batterie, et le firmware doit la multiplier par 2. Ce réglage n'existe pas encore : je l'ajoute au backlog firmware. En attendant, ne pas activer `BATTERY_VOLTAGE_X10_ID`, ou bien compiler avec `BATTERY_IS_VCC` (qui ne mesure que VCC).

### 4. Découpe de la carte (P5) : ne pas laisser +BATT contre la masse

Les traits x = 147 et y = 87 coupent le plan de masse (B.Cu) et deux pistes +BATT (F.Cu) : la première vers y ≈ 59,3, la seconde vers x = 148,35.

- Scier un peu à l'extérieur du trait, puis limer jusqu'au trait.
- Sur les deux bouts de piste +BATT, enlever au cutter le dernier millimètre de cuivre côté bord.
- Chanfreiner légèrement le bord côté B.Cu à la lime pour casser la bavure du plan de masse.
- Contrôle : J13 borne 1 et J13 borne 2 ne doivent **pas** biper ensemble.
- Passer un vernis sur les bords (vernis à ongles ou vernis de tropicalisation) : la carte vit dehors.

### 5. Alimentation 3,3 V : un TPS63020 à la place du module J2

Le module prévu pour J2 (« 0,9–3,3 V vers 3,3 V ») est un élévateur. Avec une 18650 à 4,2 V, ces modules laissent en général passer la tension d'entrée (≈ 3,9 V), alors que le BME280 est donné pour 3,6 V maximum.
On le remplace par le module **TPS63020** abaisseur-élévateur 3,3 V disponible du projet Tars (26,7 × 17,7 × 1,6 mm, trop grand pour l'emplacement J2). Il est relié par fils :

| Fil | Couleur | Module TPS63020 | Carte | Longueur |
|---|---|---|---|---|
| W7 | rouge | VIN | J2 pad 1, +BATT (149,53 ; 57,25) | ~40 mm |
| W8 | noir | GND | J2 pad 2, GND (152,07 ; 57,25) | ~40 mm |
| W9 | blanc | VOUT 3,3 V | J2 pad 3, VCC (154,61 ; 57,25) | ~40 mm |

- Sur le module : **EN relié à VIN**, et **PS/SYNC relié à GND** (mode économie). Sinon il reste en PWM et consomme quelques mA.
- Le module est posé à plat sur le dessus de la carte, sur de l'adhésif mousse double face, en vérifiant qu'aucune soudure ne passe dessous. Le volume sous le chapeau est largement suffisant. Le modèle du champignon ne prévoit pas encore son emplacement : c'est à ajouter.
- Avantage : la vis de MH5 n'est plus masquée (P7).
- Contrôle, sans BME280 ni ATtiny, en alimentant J13 par une alim de labo limitée à 100 mA :
  - à 3,7 V et à 4,2 V, VCC (C2 pad 2) doit être entre **3,20 et 3,40 V** ;
  - le courant à vide doit être **inférieur à 0,1 mA**.
  Noter le résultat aussi dans le banc de test Tars `tests/bench/PLAT-POWER-REG-001.md`, qui n'a pas encore été passé.
- Consommation : l'ATtiny en sommeil consomme quelques µA ; le module en ajoute environ 25–50 µA à vide. Cela reste plusieurs années sur une 18650 de 2500 mAh.

## Montage adapté

- **J2 (P7) :** ne rien monter dans l'empreinte, seulement les fils W7–W9 (voir point 5).
- **C6 (P8) :** ne pas le monter. Il ne sert qu'au capteur analogique (J7 / JP4), absent du champignon.
- **JP2 « DIRECT Vcc » (R9) :** le laisser ouvert : sinon il court-circuite l'entrée et la sortie du TPS63020.
- **CSB du BME280 (R11) :** la position 5 n'est pas connectée. Vérifier au multimètre que le breakout tire bien CSB vers VIN (≈ 10 kΩ) : sinon le capteur passe en SPI.
- **Polarité de J13 (R2, P9) :** pas de protection contre l'inversion. Respecter le code couleur W5 rouge = borne 1 (+, pastille carrée) et W6 noir = borne 2. Marquer « + » au feutre à côté de J13.

## Recommandé, peu coûteux

- **P13 :** un condensateur de 100 nF côté dessous entre J1 broche 1 (GND, 193,00 ; 56,86) et J1 broche 2 (VCC), au plus près du module radio.
- **R7 :** une résistance de 10 kΩ entre J6 broche 5 (RESET) et J6 broche 2 (VCC), côté dessous. Ne pas ajouter de condensateur, pour rester compatible ISP et debugWIRE.

## Firmware

Le correctif du sommeil (R3, R4) est sur la branche `hw/schematic-upgrade` : flasher cette version.
