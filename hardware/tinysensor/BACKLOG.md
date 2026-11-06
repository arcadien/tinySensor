# Backlog de mise à niveau — carte tinySensor

Revue du 2026-11-06 sur la branche `hw/schematic-upgrade`, faite avec KiCad 10.0.7 (`kicad-cli`) : ERC, netlist, DRC avec parité schéma, et comparaison avec le firmware (`lib/hal/Attiny84aHal*`).
Sévérités : **Critique** / **Majeur** / **Mineur** / **Question** / **Évolution**. Effort : S (≈ 1 h), M (une demi-journée), L (une refonte).

État à l'ERC : 0 erreur et 47 avertissements. 39 sont des symboles embarqués périmés. Deux seulement signalent un vrai défaut, et ce défaut est R1.
La parité schéma ↔ PCB remonte 39 écarts (voir R6).

## Schéma

| ID | Sév. | Zone | Constat | Correctif | Effort |
|---|---|---|---|---|---|
| R1 | Critique | Alim / MCU | PA1 (U1.12, mesure batterie) est relié directement à +BATT, sans résistance ni pont diviseur. Avec une batterie au-dessus de VCC + 0,5 V (Li-ion, 18650, J13 RAW IN au-dessus de 3,8 V), la diode de clamp conduit sans limite de courant : le MCU peut être détruit, ou VCC alimenté par PA1. La lecture sature aussi à 1023. ERC : `pin_to_pin` et `multiple_net_names` +BATT/Battery_Checker. | Résistance série de 10–100 k et 100 nF vers GND. Si la batterie peut dépasser VCC, un pont diviseur, éventuellement commuté par PA2, et adapter `ConvertAnalogValueToMv`. | S |
| R2 | Majeur | Alim | Aucune protection contre l'inversion de polarité. J10 (CR2032), J11 (AAA), J12 (AA), J13 (RAW IN) et le support 18650 du PCB sont tous en parallèle. Un bornier inversé met une tension négative sur le DC-DC et le MCU. Deux piles posées ensemble se rechargent l'une l'autre, ce qui est dangereux pour la CR2032. | P-MOSFET en idéal-diode, ou Schottky sur RAW IN. Marquer les supports comme exclusifs (DNP, mention sérigraphiée). | M |
| R3 | Majeur | Alim / firmware | En hibernation, `sleep()` passe PA2 en entrée flottante (`DDRA = 0`, `Attiny84aHal.cpp:85`). La même fonction active les pull-ups internes de PA4/PA5/PA6 (l.89), et PA3 garde le sien depuis l'init (l.42). SENSOR_VCC est alors alimenté à travers R2/R3/R6 : les capteurs ne sont jamais vraiment coupés, et le BME280 n'a jamais de reset à la mise sous tension propre. | Firmware : en sommeil, PA2 en sortie à 0, PA3/PA4/PA6 en entrée sans pull-up. | S |
| R4 | Majeur | Radio | Le module radio est alimenté en permanence (J1.2 sur VCC), et sa broche DATA (PB0) flotte en sommeil (`DDRB = 0`). Un émetteur OOK peut s'activer tout seul. Le commentaire `Attiny84aHal.h:9` dit le contraire du schéma. | Firmware : PB0 en sortie à 0 en sommeil. Option : un pull-down de 100 k sur TX_RADIO. Corriger le commentaire. | S |
| R5 | Majeur | LED | D1 est montée à l'envers (anode sur GND, cathode vers R1/PB1), et l'empreinte PCB est cohérente avec cette erreur. Le firmware met PB1 à 1 pour l'allumer, donc la LED ne s'allume jamais. | Inverser D1 au schéma et au PCB. | S |
| R6 | Majeur | Synchro | Le schéma n'est plus la référence. Le commit c8df224 a ajouté I2C et I2C_SPI (JST-EH 6 broches), JP5, JP6 et le support BH-18650 dans le PCB seulement. J3, J11, J12 et MH3 manquent au PCB, et MH1 y est en double. Il reste deux pastilles non connectées : JP6 vers MISO, et I2C.6 vers la zone GND. Le pôle − du 18650 semble sans net. | Reporter ces éléments dans le schéma, puis lancer « Update PCB from Schematic » jusqu'à une parité propre. Vérifier le 18650. | M |
| R7 | Mineur | MCU | RESET n'a pas de pull-up externe : seul le pull-up interne, faible, et un bouton. | 10 k vers VCC, sans condensateur, pour rester compatible debugWIRE. | S |
| R8 | Mineur | Alim | Le découplage global (2 × 10 µF) est faible pour une CR2032, dont la résistance interne est de 10–20 Ω, face au pic de courant de la radio. | 100–470 µF à faible fuite pour les versions CR2032. | S |
| R9 | Mineur | Alim | JP2 « DIRECT Vcc » court-circuite l'entrée et la sortie du DC-DC si J2 est monté. | Mention « JP2 fermé ⇒ J2 non monté ». Le firmware doit alors utiliser `BATTERY_IS_VCC`. | S |
| R10 | Question | Alim | Le module DC-DC J2 n'a pas de broche d'activation. Son courant de repos (souvent 10–100 µA) dépasse probablement celui de l'ATtiny en sommeil. | Le mesurer. En v2.3 : boost nano-power avec enable (classe TPS61099). | M |
| R11 | Question | I2C | La broche CSB du BME280 (J3.5) est en l'air ; si elle est vue à 0, le capteur passe en SPI. SDO est à GND, d'où l'adresse 0x76, conforme au firmware. | Relier CSB à SENSOR_VCC, ou vérifier que le breakout la tire vers le haut. | S |
| R12 | Question | I2C | Le BH1750 (`USE_BH1750`, 0x23, env SOLAR_TEST) n'apparaît pas au schéma. | Le documenter sur le second connecteur I2C une fois R6 fait, avec ADDR à 0. | S |
| R13 | Question | Alim | `USE_CHARGE_PUMP` (`platformio.ini:66`, `:81`) n'est utilisé par aucun code et ne correspond à aucun circuit. Le chargeur TP4056 n'est pas au schéma. | Supprimer le flag ou documenter le matériel concerné. | S |
| R14 | Question | MCU | Les fusibles ne sont pas consignés, en particulier BODLEVEL. Le firmware coupe le BOD en sommeil. | Consigner les fusibles voulus dans le dépôt (BOD 1,8 V à 1 MHz). | S |
| R15 | Mineur | Bibliothèques | `fp-lib-table` pointe vers `battery.pretty` et `Chinese breakouts.pretty`, qui sont absents. `sym-lib-table` déclare une bibliothèque Legacy qui n'existe pas. J13 utilise une empreinte retirée de KiCad, et J1/J3/J4 des symboles `Conn_*_Male/Female` retirés. U1 et J6 viennent de `-rescue.lib`. Le fichier est au format KiCad 7. | Nettoyer les tables. Utiliser les symboles et empreintes standards (`ATtiny84A-P`, `AVR-ISP-6`, `Conn_01x0N_Pin/Socket`, `TerminalBlock_Phoenix` 5,08), puis « Update Symbols from Library ». | S–M |
| R16 | Mineur | Connecteurs | J1 est câblé GND/VCC/DATA, alors qu'un FS1000A est DATA/VCC/GND. J5 est câblé VCC/GND/DQ, alors qu'un DS18B20 TO-92 est GND/DQ/VDD. La pastille 1 de l'ISP est ronde, donc rien ne la repère. | Fonctions des broches en sérigraphie, et pastille 1 carrée sur l'ISP. | S |
| R17 | Mineur | Hygiène | Valeurs « 4,7k » et « 4k7 » mélangées. JP3 a la valeur « ~ ». C5, une céramique de 1 nF, utilise une empreinte tantale, et C6 (0,1 µF) un symbole polarisé. La note de R4 (75R ou 18k) n'est pas claire. Un fil pendouille en (54,61 ; 120,65). Le cartouche indique Rev 2.3, la carte v2.2. | Normaliser les valeurs, passer C5 en `C_Disc`, supprimer le fil. | S |
| R18 | Évolution | v2.3 | Pistes pour une v2.3 : ATtiny84A-SSU ou ATtiny1614 (UPDI, TWI matériel, RTC basse conso), émetteur OOK pour 1,8–3,6 V (STX882, SYN115), boost nano-power avec enable, protection idéal-diode, pont diviseur commuté. | — | L |

Points non vérifiés : les pull-ups internes aux breakouts, le courant de repos réel du DC-DC, les valeurs des fusibles.

Ordre suggéré : R3, R4 et R5 d'abord (firmware et LED, peu coûteux), puis R1 et R2 (sécurité), puis R6 avant toute autre modification du schéma.

## PCB

_Revue en cours._
