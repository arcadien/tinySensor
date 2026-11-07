# Variante LM35 : capteur analogique alimenté en 5 V seulement pendant les mesures

Valable pour la v2.2 et la v3.0. L'ATtiny tourne directement sur 2 piles AA ou AAA. Le LM35 (4 V minimum) est alimenté par un module élévateur 5 V sans broche EN. Ce module n'est mis sous tension que pendant les mesures, par un PNP BC327 commandé par PB2 (J8 « EXT. INT0 ») : sortie à l'état bas pendant les mesures, entrée haute impédance le reste du temps (option de compilation `USE_SWITCHED_5V`).

## Alimentation de la carte

- Piles 2×AA ou 2×AAA sur J13 (borne 1 = +, carrée ; borne 2 = GND).
- **JP2 « DIRECT Vcc » fermé**, rien sur J2 : VCC = tension des piles (2,0–3,2 V). L'ATtiny84A à 1 MHz fonctionne dès 1,8 V.
- Pas de BME280 (3,6 V maximum, mais surtout inutile ici). La portée radio baisse sous 3 V.

## Commutation du module 5 V (câblage en l'air)

```
+BATT ──┬────────── E  BC327 (PNP)
       10k             C ──── VIN module 5 V
        │              B
        └──── 1k ──────┤
                       │
              J8 broche 1 (PB2)
```

| Pièce | Rôle |
|---|---|
| BC327 | interrupteur côté +BATT du module 5 V |
| 10 kΩ base–émetteur | bloque le BC327 quand PB2 est en haute impédance (repos, sommeil, démarrage) |
| 1 kΩ vers PB2 | ≈ 2 mA de base pendant les mesures : le BC327 laisse passer ≈ 40 mA, le module démarre un peu plus lentement, sans conséquence |

PB2 n'est jamais forcée à l'état haut. +BATT ne doit pas dépasser VCC, sinon un courant de fuite passe par la diode de protection de PB2 : c'est garanti avec JP2 fermé (VCC = piles).

Brochage TO-92, face plate vers soi, pattes en bas, de gauche à droite :
- BC327 : C, B, E ;
- LM35 : +Vs, Vout, GND.

Le vérifier sur la fiche du fabricant.

## LM35

| LM35 | Relié à |
|---|---|
| +Vs | VOUT du module 5 V |
| Vout | J7 broche 2 (entrée analogique, PA0 à travers R4 75 Ω) |
| GND | GND de la carte (J2 broche 2, ou J7 broche 3 si JP4 est soudé 2-3) |

GND du module 5 V : GND de la carte.

Pourquoi couper le +BATT et pas la masse : avec la masse coupée, celle du LM35 le serait aussi. La tension de saturation du transistor (10–30 mV) s'ajouterait à la mesure, soit 1 à 3 °C d'erreur. Le LM35 doit rester sur la vraie masse. Mais le module laisse passer VIN vers VOUT quand sa masse est coupée, et le LM35 resterait alors alimenté en sommeil.

## Firmware

- Construire avec `BATTERY_IS_VCC`, `ANALOG1_X10_ID` et `USE_SWITCHED_5V`.
- Le firmware envoie des mV : la température vaut **mV ÷ 10** (°C), à convertir côté réception.
- Entre la mise sous tension des capteurs et la lecture analogique, il s'écoule au moins 30 ms. C'est assez pour que le module démarre (quelques ms) et que le LM35 se stabilise : à confirmer à l'oscilloscope sur VOUT.
- La mesure se fait par rapport à VCC, corrigée par la VCC mesurée : la résolution est d'environ 0,3 °C à 3 V. Pour faire mieux, il faudrait passer à la référence interne de 1,1 V. C'est une évolution firmware, qui passe par une exigence RBD.

## Contrôles

1. Sans LM35, piles en place : en sommeil, VIN du module à 0 V et le courant total de quelques µA.
2. Pendant une mesure, VOUT du module à 5 V. Au multimètre, ça se voit difficilement : pour le test, relier J8 broche 1 à GND avec un fil.
3. Avec le LM35 : comparer à un thermomètre. Une erreur constante de 1 à 2 °C se corrige côté réception.
