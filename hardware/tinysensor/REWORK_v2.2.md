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

### 5. Tension VCC avec accu plein : à mesurer avant de brancher le BME280

Le module DC-DC J2 (« 0,9–3,3 V vers 3,3 V ») est un élévateur. Avec 4,2 V en entrée, beaucoup de modules de ce type laissent passer la tension d'entrée, moins une diode (≈ 3,9 V).
Le rail capteurs suit VCC, alors que le BME280 est donné pour 3,6 V maximum (4,25 V en absolu).

- Mesurer VCC (C2 pad 2) avec un accu chargé à 4,2 V, sans le BME280.
- **VCC ≤ 3,6 V :** rien à faire.
- **VCC > 3,6 V :** utiliser un breakout BME280 avec régulateur 3,3 V intégré, ou remplacer J2 par un module abaisseur-élévateur (buck-boost) 3,3 V.

## Montage adapté

- **J2 (DC-DC) au-dessus de MH5 (P7) :** monter le module sur une barrette femelle 1×3 (carte) et des broches mâles (module), pour pouvoir l'enlever et accéder à la vis de MH5. La place ne manque pas au-dessus de la carte.
- **C6 (P8) :** ne pas le monter. Il ne sert qu'au capteur analogique (J7 / JP4), absent du champignon.
- **JP2 « DIRECT Vcc » (R9) :** le laisser ouvert, puisque le DC-DC est monté.
- **CSB du BME280 (R11) :** la position 5 n'est pas connectée. Vérifier au multimètre que le breakout tire bien CSB vers VIN (≈ 10 kΩ) : sinon le capteur passe en SPI.
- **Polarité de J13 (R2, P9) :** pas de protection contre l'inversion. Respecter le code couleur W5 rouge = borne 1 (+, pastille carrée) et W6 noir = borne 2. Marquer « + » au feutre à côté de J13.

## Recommandé, peu coûteux

- **P13 :** un condensateur de 100 nF côté dessous entre J1 broche 1 (GND, 193,00 ; 56,86) et J1 broche 2 (VCC), au plus près du module radio.
- **R7 :** une résistance de 10 kΩ entre J6 broche 5 (RESET) et J6 broche 2 (VCC), côté dessous. Ne pas ajouter de condensateur, pour rester compatible ISP et debugWIRE.

## Firmware

Le correctif du sommeil (R3, R4) est sur la branche `hw/schematic-upgrade` : flasher cette version.
