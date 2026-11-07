# Debug pas-à-pas (debugWIRE)

L'ancien projet Atmel Studio (`TinySensor/`) a été supprimé. Le debug passe désormais par PlatformIO + AVaRICE + AVR Dragon, en debugWIRE.

> Statut : **non validé sur banc**. Procédure à confirmer au premier essai.

## Chaîne d'outils

| Élément | Rôle |
|---|---|
| AVR Dragon | Sonde (USB) — debugWIRE, ISP, HVSP |
| [AVaRICE](https://avarice.sourceforge.io/) | Serveur GDB pour le Dragon (`--dragon --debugwire`) |
| PlatformIO | `debug_tool = custom`, se connecte au serveur GDB |
| avrdude | Fusibles (activation/désactivation DWEN) |

Bloom n'est pas une option : il cible les sondes EDBG (Atmel-ICE, Snap, PICkit 4), pas le Dragon. `avr-stub` (débogueur natif PlatformIO) ne supporte que les ATmega.

À vérifier : présence de l'ATtiny84 dans la liste des cibles debugWIRE d'AVaRICE (`avarice --known-devices`, marqueur `+`).

## Environnement PlatformIO (proposition)

```ini
[env:debug]
extends = env:devmodule      ; env de base à choisir
build_type = debug
debug_tool = custom
debug_port = :4242
debug_server = avarice --dragon --debugwire --part attiny84 :4242
debug_init_cmds =
  target remote $DEBUG_PORT
  file "$PROG_PATH"
  load
```

Lancer AVaRICE manuellement si besoin : `avarice --dragon --debugwire --part attiny84 :4242`.

## Fusible DWEN

### Activer debugWIRE
Programmer DWEN (= 0) dans le hfuse via ISP (`-c dragon_isp`). Une fois actif, **/RESET n'est plus une broche de reset et l'ISP ne fonctionne plus**.

### Revenir en ISP (sans haute tension)
1. `avrdude -c dragon_dw -P usb -p t84` — avrdude se connecte en debugWIRE et bascule la cible en mode ISP, **sans coupure d'alimentation**.
2. `avrdude -c dragon_isp -P usb -p t84 -U hfuse:w:0xXX:m` — réécrire le hfuse avec DWEN = 1 (partir de la valeur actuelle).

AVaRICE ne sait pas désactiver DWEN lui-même. Séquence avrdude à valider selon la version.

### Recours : haute tension (HVSP)
Si debugWIRE ne répond plus : HVSP à 12 V (tolérance ~11,5–12,5 V), supporté par le Dragon, câblage dédié.

## Limites debugWIRE

- Pas de breakpoint matériel : breakpoints logiciels par réécriture de pages flash (usure).
- Fusibles et lock bits inaccessibles pendant la session.
- Effacement puce (`-e`) impossible en mode debugWIRE.

## Contraintes matérielles — carte de debug dédiée

Les cartes de production ne sont pas prévues pour debugWIRE. Une carte de debug dédiée est à prévoir avec :

- **/RESET sans condensateur** (la ligne porte le signal debugWIRE).
- **Pull-up /RESET ≥ 10 kΩ** (ou aucune résistance externe ; ne pas utiliser de valeur plus faible).
- Connecteur ISP 6 broches accessible (activation/désactivation DWEN).
- Idéalement : accès au câblage HVSP pour récupération.
