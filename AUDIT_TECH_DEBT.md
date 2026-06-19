# Audit de dette technique — TinySensor

**Projet** : [arcadien/tinySensor](https://github.com/arcadien/tinySensor) — firmware AVR bas-consommation pour capteurs d'environnement sans fil (ATtiny84a, 433 MHz, protocoles Oregon v3 / Lacrosse WS7000 / X10).

**Auditeur** : Claude (modèle opus 4.7) — revue statique

**Date** : 19 avril 2026 — mise à jour 19 juin 2026 (voir §1bis)

**Périmètre** : ensemble du dépôt au commit `aa53a83` (branche `master`) — code firmware (`src/`, `lib/`, `include/`), tests (`test/`), projet Atmel Studio legacy (`TinySensor/`), outillage (`scripts/`, `shared/`), web builder (`web/`), hardware (`hardware/`), CI (`.github/workflows/`), Docker/devcontainer.

---

## 1. Synthèse exécutive

TinySensor est un projet firmware embarqué mature sur le plan fonctionnel (protocoles radio testés, HAL propre, tests unitaires natifs avec Unity, portage PlatformIO). Le code "cœur" (`lib/oregon`, `lib/LacrosseWS7000`, `lib/x10`, HAL) est propre, bien abstrait, testé. La dette se concentre dans trois zones qui pèsent sur la maintenabilité et le risque opérationnel :

D'abord, **un secret Coveralls est committé en clair** dans `.coveralls.yml` — il doit être révoqué immédiatement. Ensuite, **la CI ne compile aucun firmware AVR** (seul l'environnement natif est testé), donc un refactor peut casser silencieusement les builds de production `S_02`..`S_05`. Enfin, **un second arbre source** (`TinySensor/`, projet Atmel Studio legacy) duplique le code firmware et contient des binaires de build versionnés ; il est une source de divergence certaine et gonfle l'historique git.

Au-delà de ces trois points, la configuration per-device vit sous forme de blocs presque identiques dans `platformio.ini` (7 environnements avec copier-coller), les dépendances tirent toutes vers des forks personnels non épinglés, l'image Docker `php:7.4-apache` est EOL (novembre 2022), et plusieurs scripts d'outillage contiennent des chemins absolus spécifiques à l'auteur. L'ensemble est traitable en ~3 sprints d'un développeur à temps partiel, avec les gains de risque concentrés sur les deux premières semaines.

**Chiffres clés** : 3 779 lignes de code+tests sur ~20 fichiers. 8 suites de tests Unity natives. 1 workflow GitHub Actions (build + test native uniquement). 6 envs PlatformIO pour cibles physiques distinctes. 1 secret exposé. 2 dépendances critiques EOL.

---

## 1bis. Mises à jour de l'audit

Cette section suit l'évolution de la dette au fil des corrections apportées. Chaque entrée indique l'item résolu, la date, le(s) fichier(s) touché(s) et l'impact sur le score global.

### 2026-04-19 — Workflow GitHub Actions refondé

Fichier modifié : `.github/workflows/main.yml`

Trois items du top-20 traités en une seule PR :

- **T1 (36) — CI compile maintenant toutes les cibles AVR**. Le workflow a été scindé en deux jobs : `test-native` (inchangé) et `build-avr` avec matrice `[S_02, S_03, S_04, S_05, robot, devmodule, SOLAR_TEST]`. `fail-fast: false` pour isoler les pannes par env. → **Résolu**.
- **D2 (30) — Actions et Python mis à jour**. `checkout@v4`, `cache@v4`, `setup-python@v5`, Python `3.12` (remplace 3.9 EOL). Clé de cache désormais invalidée sur `hashFiles('platformio.ini')`. → **Résolu**.
- **I4 (20) — Artefacts firmware publiés**. Chaque build AVR upload `firmware.hex` + `firmware.elf` via `actions/upload-artifact@v4`, rétention 30 j. Addresse la partie "artefact téléchargeable" d'I4 ; la partie "release automation sur tag" reste à faire. → **Partiellement résolu**.

Également : déclenchement étendu aux pull requests (auparavant : `on: push` seul), et cache configuré avec fallback via `restore-keys`.

Impact global : **score cumulé réduit de 86 points** (36 + 30 + 20). Top-20 recomposé au §4.

### 2026-04-19 — `.gitignore` nettoyé et patrons d'artefacts ajoutés

Fichier modifié : `.gitignore`

- **I7 (20)** — Ligne 1 buggée `Debug/ Release/` (un seul pattern littéral avec espace) supprimée. Fichier réorganisé en blocs commentés. Ajout des patrons `*.o`, `*.d`, `*.hex`, `*.eep`, `*.lss`, `*.map`, `*.srec` (`*.elf` était déjà là). Préservation des fins de ligne CRLF. → **Résolu**.
- **I1 (25)** — Audit-revisité. `git ls-files TinySensor/Debug/` retourne vide : **aucun artefact n'était réellement tracké** (les `.o`/`.hex`/`.elf` sur disque proviennent d'une compilation locale Atmel Studio non committée). Le `git rm --cached` prévu par le plan est un no-op. Le nouveau `.gitignore` empêche toute régression : `git check-ignore` confirme que chaque fichier de `TinySensor/Debug/` match la règle `Debug/`. → **Résolu (par prévention ; l'audit initial surestimait l'item).**

Impact cumulé 2026-04-19 (toutes entrées confondues) : **131 points** (86 + 25 + 20).

### 2026-06-16 — Revue MAGI + nettoyage CI/outillage (PR #24)

Branche : `fix/magi-review-findings` — revue indépendante par trois agents MAGI (Gaspard, Melchior, Balthazar) ayant voté REJECT 2/3 sur des bugs firmware, suivie d'une session de corrections.

**Nouveaux bugs firmware identifiés par MAGI (non présents dans l'audit initial) :**

- **M1 — `PRR &= ~_BV(PRTIM1/PRADC)` logique inversée dans `UseLessPowerAsPossible()`** (`lib/hal/Attiny84aHal.cpp`). Dans le registre PRR, mettre un bit à 1 éteint le périphérique ; `&= ~` l'allumait (Timer1 et ADC restaient actifs en permanence). Corrigé en `|=`. Score rétroactif : (3+3)×5 = **30**. → **Résolu**.
- **M2 — `SetChannel()` sans validation de bornes** (`lib/oregon/Oregon_v3.cpp`). Channel > 3 décalait des bits dans les nibbles de l'ID capteur, corrompant silencieusement `message[2]`. Guard ajouté. Score rétroactif : (3+3)×5 = **30**. → **Résolu**.
- **M3 — `message[]` variable globale de fichier, non membre de classe** (`lib/oregon/Oregon_v3.cpp`). Singleton accidentel : deux instances `OregonV3` auraient partagé le même buffer. Déplacé en membre privé. Score rétroactif : (2+2)×4 = **16**. → **Résolu**.

**Items de l'audit résolus dans cette PR :**

- **C4 (20) — Dead-code `if (int(count) != count)` dans `OregonV3::Sum`**. Héritage d'une implémentation internet où `count` était `float` ; avec `uint8_t`, la branche est toujours fausse. Supprimée. → **Résolu**.
- **C7 (18) + C10 (15) — Duplication massive `platformio.ini`**. Introduction de `[env:avr_base]` + `extends` ; suppression du `USE_BMP280=1` en double dans `env:S_03`. → **Résolu**.
- **D9 (25) — Pas de Dependabot**. Fichier `.github/dependabot.yml` ajouté : mises à jour hebdomadaires pour `github-actions`, mensuelles pour `docker` et `pip`. → **Résolu**.
- **I4 (12) — Release automation sur tag** (partie restante). Job `release` ajouté au workflow : se déclenche sur `v*`, télécharge tous les artefacts firmware et publie une GitHub Release avec notes auto-générées. → **Résolu (complètement)**.
- **D8/Doc7 (10) — Badge Travis mort**. Remplacé par le badge GitHub Actions dans `README.md`. → **Résolu**.

Impact 2026-06-16 : **176 points** résolus (M1:30 + M2:30 + M3:16 + C4:20 + C7:18 + C10:15 + D9:25 + I4:12 + D8:10).

Impact cumulé toutes dates : **307 points** (131 + 176).

### 2026-06-18 — Révocation du token Coveralls + suppression de `web/` + requalification A1

- **C1 (35) — Token Coveralls révoqué et sorti du repo**. Le token `K1UMmvKnmugffPWriB48kR274KXcAuvuK` présent dans `.coveralls.yml` a été révoqué côté coveralls.io et remplacé par un secret GitHub Actions `COVERALLS_REPO_TOKEN`. → **Résolu**.
- **D1 (36) + A4 (15) + D6 (12) — `web/` supprimé**. Décision : le firmware builder web n'a jamais été finalisé ni déployé. Suppression des deux fichiers (`web/build.php`, `web/index.html`). Élimine d'un coup l'image Docker PHP 7.4 EOL, la vulnérabilité `shell_exec`, et jQuery 1.12.4. → **Résolu (D1, A4, D6)**.
- **A1 — Requalifié (risque accepté)**. `TinySensor/` n'est pas du code mort : il est conservé intentionnellement pour le workflow de debug/flash Atmel Studio (debugWIRE/JTAG), non remplaçable par PlatformIO. Score abaissé de 32 → 20, catégorie "risque de divergence à surveiller". Mitigations recommandées : commentaires croisés dans les deux `main.cpp` + step CI `diff` en warn-only. → **Requalifié, non supprimé**.
- **C2 (30) — Macro `OREGON_DELAY_US` morte supprimée**. La macro n'était jamais appelée (Oregon utilise `_hal->Delay512Us()/Delay1024Us()` directement) et référençait une méthode inexistante. Supprimée de `TestHal.h`. → **Résolu**.
- **C15 (15) — `override` ajouté sur les stubs de délai de `TestHal`**. `Delay400Us`, `Delay1024Us`, `Delay512Us` n'avaient pas `override` ; dérive de signature désormais détectable à la compilation. → **Résolu**.

- **C3 (20) — `Init()` orphelin supprimé**. Fonction libre `void Init(void)` en bas de `Attiny84aHal.cpp`, hors de la classe, jamais appelée — shadow silencieux de `Attiny84aHal::Init`. Supprimée. → **Résolu**.
- **C11 (15) — Branche morte `SetPressure` supprimée**. `if (pressure < 200) pressure = 200;` était immédiatement écrasé par le clamp à 850 ; supprimé. → **Résolu**.
- **D5 (20) — `avrdude.conf` sorti du repo**. Le fichier (534 KB) provient du toolchain PlatformIO (`~/.platformio/packages/tool-avrdude/`) ; supprimé avec `git rm`. → **Résolu**.
- **I3 — Requalifié**. Audit revisité : le ZIP KiCAD n'était pas tracké git (sur disque uniquement) ; `*.zip` déjà couvert par `.gitignore`. Non-issue, comme I1. → **Résolu (par prévention)**.
- **I2 (25) — Chemins absolus paramétrés**. `Makefile` : `PIO ?= $(HOME)/.platformio/penv/bin/pio`, `TARGET ?= S_03`, `cp` remplace `copy`. `scripts/flash_tinySensor` : `PIO_HOME` et `PROJECT_DIR` avec défauts portables (`${HOME}/.platformio` et `$(dirname $0)/..`). → **Résolu**.
- **D3 (24) + D4 (24) — Dépendances PlatformIO épinglées sur SHA**. Les 5 dépendances (BH1750, tiny-i2c, Unity, avr-ds18b20, SparkFun_BME280) épinglées sur leur commit HEAD courant dans `platformio.ini`. Reproductibilité garantie, risque supply-chain éliminé. → **Résolu**.
- **Doc1 (20) — `docs/environments.md` créé**. Convention `S_*` = numéro sur le PCB, guide de calibration `INTERNAL_1v1` par carte, table boards physiques + envs spéciaux (`robot`, `devmodule`, `SOLAR_TEST`), seuils batterie par chimie, procédure d'ajout d'une nouvelle carte. → **Résolu**.

Impact 2026-06-18 : **291 points** résolus (C1:35 + D1:36 + A4:15 + D6:12 + C2:30 + C15:15 + C3:20 + C11:15 + D5:20 + I2:25 + D3:24 + D4:24 + Doc1:20).

Impact cumulé toutes dates : **598 points** (307 + 291).

### 2026-06-19 — RBD setup, requirements reverse-engineering, architectural violations V1-V4

#### Infrastructure RBD

Le projet est passé sous **Requirement-Based Development (RBD)** via le plugin Claude Code `arcadien/rbd@0.12.0`. Cela comprend :

- `.rbd/config.yml` — ID format `FUNC/TECH/CONF/PLAT-DOMAIN-NNN`, framework Unity/C++, linter `clang-format`
- `.clang-format` — style Google, 2 espaces, 80 colonnes, `SortIncludes: false`
- Baseline clang-format appliquée sur tous les `.cpp`/`.h` existants (37 fichiers)
- `CLAUDE.md` mis à jour avec la section RBD (commit prefix obligatoire, `// @req {id}`, etc.)
- Pre-push hook `~/.claude/hooks/rbd-pre-push-check.sh` installé

#### Rétro-ingénierie des exigences

Analyse architecturale complète (`docs/analysis-2026-06-18.md`) suivie de 8 vagues de `requirement-analyst` agents en parallèle. **67 exigences validées et committées** :

| Fichier | Domaines | Nb exigences |
|---------|----------|:------------:|
| `requirements/functional.md` | FUNC-OREGON, LACROSSE, X10, BATTERY, SENSOR, ANALOG | 37 |
| `requirements/technical.md` | TECH-HAL, FILTER, CONVERT, SERIAL, X10 | 14 |
| `requirements/configuration.md` | CONF-BUILD | 11 |
| `requirements/platform.md` | PLAT-POWER | 5 |
| **Total** | | **67** |

`docs/architecture.md` enrichi d'une table de traçabilité complète (67 lignes Requirement → Component).

#### Violations architecturales V1-V4 résolues

- **V1 — `x10rf` contournait le HAL pour tous les timings** : 5 fonctions statiques locales (`x10_preamble_high`, etc.) appelaient `_delay_us()` directement, violant `TECH-HAL-001`. Cycle RBD complet : TECH-X10-001 (5 nouvelles méthodes `Hal` : `DelayX10PreambleHigh` 8960 µs, `DelayX10PreambleLow` 4500 µs, `DelayX10BitShort` 560 µs, `DelayX10BitLong` 1120 µs, `DelayX10Gap` 40 000 µs) + TECH-X10-002 (x10rf remplacé — bloc `#if defined(AVR) _delay_us` supprimé, 5 appels `_hal->DelayX10*()` substitués). 14 nouveaux tests Unity. → **Résolu**. Note : **C6 (SoftSerial) reste ouvert** — même pattern, traitement séparé.

- **V2 — `#include <AnalogFilter.h>` mort dans `Attiny84aHal.cpp`** : inclus ligne 2, jamais référencé — `AdcRead` utilisait des variables locales. Supprimé. → **Résolu**.

- **V3 — `BH1750` absent de `docs/architecture.md`** : ajouté au diagramme Mermaid et à la table des responsabilités. → **Résolu**.

- **V4 — `ds18b20` absent de `docs/architecture.md`** : idem. → **Résolu**.

#### Devcontainer

- **DevContainer** : Node.js 20 LTS + `@anthropic-ai/claude-code` ajoutés à `.devcontainer/Dockerfile` (installation système via NodeSource). Claude Code désormais disponible après `Rebuild Container`.

#### C5 — Accès DS18B20 encapsulé dans la classe `Ds18b20`

- **C5 (20) — `main.cpp` accédait directement aux registres AVR pour DS18B20** (`ds18b20convert(&PORTA, &DDRA, &PINA, …)`). Cycle RBD complet : TECH-SENSOR-001 validé, 4 tests Unity natifs (`test/test_ds18b20/`), classe `include/Ds18b20.h` créée suivant exactement le patron `BMx280` (sections `#if defined(AVR)` / `#else` stub, constructeur `Hal*`). `main.cpp` réduit à `ds18b20sensor.Convert()` + `ds18b20sensor.Read()` — aucune référence résiduelle à `PORTA`/`DDRA`/`PINA`. → **Résolu**.

Impact 2026-06-19 : **20 points** résolus (C5:20). Score cumulé : **618 points** (598 + 20).


---

## 2. Méthodologie

Chaque item identifié est scoré selon la formule du framework tech-debt :

**Priorité = (Impact + Risque) × (6 − Effort)**

- **Impact** (1–5) : dans quelle mesure cela ralentit l'équipe ou dégrade la qualité ?
- **Risque** (1–5) : que se passe-t-il si on ne corrige pas ?
- **Effort** (1–5) : coût de la correction (1 = trivial, 5 = gros chantier).

Le plus haut score théorique est 50 ((5+5)×(6−1)). Je classe les items en quatre bandes :

| Bande | Score | Traitement |
|---|---|---|
| P0 — Critique | ≥ 30 | À traiter sous 1–2 semaines |
| P1 — Élevé | 20–29 | Sprint courant ou suivant |
| P2 — Moyen | 12–19 | Backlog qualité à dérouler sur un trimestre |
| P3 — Faible | < 12 | Opportuniste, quand on touche la zone |

---

## 3. Dette par catégorie

### 3.1 Dette de code

| ID | Item | Localisation | Impact | Risque | Effort | Priorité |
|----|------|--------------|:------:|:------:|:------:|:--------:|
| ~~C1~~ | ~~Token Coveralls commité en clair (`repo_token: K1UMmvKnmugffPWriB48kR274KXcAuvuK`)~~ — ✅ **Résolu 2026-06-18** (token révoqué, déplacé en secret GitHub Actions `COVERALLS_REPO_TOKEN`) | `.coveralls.yml` | 2 | 5 | 1 | ~~35~~ |
| ~~C2~~ | ~~`TestHal` référence un `OREGON_DELAY_US(x) TestHal.Delay(x)` qui n'existe pas.~~ — ✅ **Résolu 2026-06-18** (macro jamais appelée — Oregon utilise `_hal->Delay512Us()/Delay1024Us()` directement ; macro morte supprimée) | `lib/hal/TestHal.h:74` | 3 | 3 | 1 | ~~30~~ |
| ~~C3~~ | ~~Fonction libre `void Init(void)` orpheline en bas de `Attiny84aHal.cpp`, hors de la classe, jamais appelée.~~ — ✅ **Résolu 2026-06-18** (fonction supprimée) | `lib/hal/Attiny84aHal.cpp:199-203` | 2 | 2 | 1 | ~~20~~ |
| ~~C4~~ | ~~Dead-code dans `OregonV3::Sum` : `if (int(count) != count)` est toujours faux pour un `uint8_t`.~~ — ✅ **Résolu 2026-06-16** (branche morte supprimée ; héritage d'une impl. internet avec `float count`) | `lib/oregon/Oregon_v3.cpp:82` | 2 | 2 | 1 | ~~20~~ |
| ~~C5~~ | ~~`main.cpp` fait des accès registres directs (`ds18b20convert(&PORTA, &DDRA, &PINA, (1 << 3), nullptr)`), contournant le HAL.~~ — ✅ **Résolu 2026-06-19** (classe `Ds18b20` créée dans `include/Ds18b20.h` — même patron que `BMx280` ; `main.cpp` appelle `ds18b20sensor.Convert()` / `ds18b20sensor.Read()`, plus aucune référence à `PORTA`/`DDRA`/`PINA` ; TECH-SENSOR-001 + 4 tests Unity) | `src/main.cpp:116,120` | 3 | 2 | 2 | ~~20~~ |
| C6 | `SoftSerial` utilise `_delay_us()` directement au lieu du HAL → impossible à tester en natif et rompt le contrat d'abstraction. | `lib/softSerial/SoftSerial.cpp:3-6,16,24,32,36` | 3 | 2 | 2 | **20** |
| ~~C7~~ | ~~Config per-env avec duplication massive dans `platformio.ini` : S_02, S_03, S_04, S_05, robot, devmodule, SOLAR_TEST partagent 80 % de leurs `build_flags`.~~ — ✅ **Résolu 2026-06-16** (`[env:avr_base]` + `extends` ; `default_envs` pour exclure la base des builds) | `platformio.ini` | 4 | 2 | 3 | ~~18~~ |
| C8 | Magic numbers non documentés : `PRESSURE_SCALING_VALUE = 795`, `MAX_ROLLING_CODE_VALUE = 0xA5` (non utilisée), `SYNC_NIBBLE = 0x0A` (non utilisée), timings X10 (`PREAMBLE_LOW = 4500`, etc.) sans référence datasheet. | `lib/oregon/Oregon_v3.cpp`, `lib/x10/x10rf.cpp` | 2 | 2 | 2 | **16** |
| C9 | `dec16ToHex` déborde silencieusement pour `input > 9999` (le test `conversionToolsTest.cpp` le prouve avec `10000 → 0xA000` marqué en commentaire "overflow"). Pas de `assert` ni de clamp. | `lib/utils/src/conversionTools.cpp:5-22` | 2 | 2 | 2 | **16** |
| ~~C10~~ | ~~`USE_BMP280=1` déclaré **deux fois** dans `env:S_03` (lignes 88 et 90) — copier-coller résiduel.~~ — ✅ **Résolu 2026-06-16** (subsumed par C7 ; la refacto `extends` a éliminé le doublon) | `platformio.ini:88,90` | 2 | 1 | 1 | ~~15~~ |
| ~~C11~~ | ~~Branche morte dans `LacrosseWS7000::SetPressure` : `if (pressure < 200) pressure = 200;` est immédiatement écrasé par `if (pressure < 850) pressure = 850;`.~~ — ✅ **Résolu 2026-06-18** (branche morte supprimée) | `lib/LacrosseWS7000/LacrosseWS7000.cpp:24-25` | 1 | 2 | 1 | ~~15~~ |
| C12 | Macro `BITREAD` redéfinie dans 4 fichiers (`Oregon_v3.cpp`, `LacrosseWS7000.cpp`, `x10rf.cpp`, `x10rf.h`) avec des variantes (`bitRead`, `BITREAD`). | (multiple) | 2 | 1 | 2 | **12** |
| C13 | HAL "leaky" : méthodes nommées `RadioGoHigh/Low`, `SerialGoHigh/Low` — pilotage sémantique d'IO, pas une vraie abstraction de bus radio. Un port sur un MCU sans GPIO configurable de la même façon obligera à retoucher le HAL. | `lib/hal/Hal.h` | 3 | 2 | 4 | **10** |
| C14 | TODO isolé : `// TODO: tests for other modes` | `test/test_x10/x10LibTest.cpp:130` | 2 | 2 | 3 | **12** |
| ~~C15~~ | ~~`TestHal::Delay400Us/Delay1024Us/Delay512Us` manquent `override`.~~ — ✅ **Résolu 2026-06-18** (`override` ajouté sur les trois méthodes) | `lib/hal/TestHal.h:41-44` | 2 | 1 | 1 | ~~15~~ |
| ~~M1~~ | ~~`PRR &= ~_BV(PRTIM1/PRADC)` logique inversée dans `UseLessPowerAsPossible()` : les périphériques restaient actifs (consommation inutile sur batterie).~~ — ✅ **Résolu 2026-06-16** (identifié par revue MAGI, corrigé en `\|=`) | `lib/hal/Attiny84aHal.cpp` | 3 | 3 | 1 | ~~30~~ |
| ~~M2~~ | ~~`OregonV3::SetChannel()` sans validation de bornes : channel > 3 corrompt silencieusement `message[2]` en écrasant les nibbles ID capteur.~~ — ✅ **Résolu 2026-06-16** (guard `if (channel < 1 \|\| channel > 3) return;` ajouté) | `lib/oregon/Oregon_v3.cpp` | 3 | 3 | 1 | ~~30~~ |
| ~~M3~~ | ~~`message[]` variable globale de fichier dans `Oregon_v3.cpp` — singleton accidentel : deux instances `OregonV3` auraient partagé le même buffer.~~ — ✅ **Résolu 2026-06-16** (déplacé en membre privé `uint8_t message[MESSAGE_SIZE_IN_BYTES]`) | `lib/oregon/Oregon_v3.cpp/.h` | 2 | 2 | 2 | ~~16~~ |

### 3.2 Dette d'architecture

| ID | Item | Impact | Risque | Effort | Priorité |
|----|------|:------:|:------:|:------:|:--------:|
| A1 | **Arbre source dupliqué — risque accepté** : `TinySensor/` (projet Atmel Studio) est conservé intentionnellement pour le workflow de debug pas-à-pas et de flashage via debugWIRE/JTAG, que PlatformIO ne remplace pas complètement. Contient `main.cpp`, `bh1750.cpp/h`, `TinyI2CMaster.cpp/h` + `third_party/`. Risque résiduel : divergence silencieuse si un fix appliqué dans `src/` n'est pas reporté dans `TinySensor/main.cpp`. Mitigations recommandées : (1) commentaire croisé dans les deux `main.cpp` pointant l'un vers l'autre, (2) step CI `diff -u src/main.cpp TinySensor/main.cpp` en mode warn-only pour rendre la divergence visible. | 2 | 3 | 2 | **20** |
| A2 | Pas d'interface commune `EnvironmentEncoder` : `main.cpp` dispatche Oregon vs Lacrosse avec des `#if defined(USE_OREGON)/USE_LACROSSE` répétés 4× dans la même fonction. Ajouter un 3e protocole coûtera un passage dans toute la `main()`. | 3 | 2 | 3 | 15 |
| A3 | Le bloc BH1750 dans `main.cpp:155-168` instancie en ligne un `LacrosseWS7000 lightEncoder(&hal)` — logique capteur mélangée avec logique de transmission. | 3 | 2 | 2 | **20** |
| ~~A4~~ | ~~Web firmware builder (`web/build.php`) : `shell_exec` sans sanitisation des inputs.~~ — ✅ **Résolu 2026-06-18** (décision : supprimer `web/` ; builder jamais finalisé, jamais utilisé en production — `web/build.php` + `web/index.html` supprimés) | 2 | 3 | 3 | ~~15~~ |
| A5 | Configuration 100 % build-time : chaque capteur physique = un env PIO + une recompilation. Pas de config runtime, donc aucune mise à jour terrain sans reflashage manuel. Architecture acceptable pour un hobby / production très petite série ; limitante si on veut scaler. | 3 | 2 | 4 | 10 |
| A6 | HAL ATmega328p absent malgré mention README. Branche `feat/328p_hal` existe mais non mergée — code parallèle qui va dériver. | 2 | 1 | 4 | 6 |

### 3.3 Dette de tests

| ID | Item | Impact | Risque | Effort | Priorité |
|----|------|:------:|:------:|:------:|:--------:|
| ~~T1~~ | ~~**La CI ne compile aucune cible AVR**. Le workflow ne lance que `pio test -e native_debug`. Un refactor peut casser `S_02`..`SOLAR_TEST` sans alerte. Le projet "prod" n'est jamais vérifié.~~ — ✅ **Résolu 2026-04-19** (workflow à deux jobs, matrice AVR) | 5 | 4 | 2 | ~~36~~ |
| T2 | Pas de rapport de couverture actif. `.coveralls.yml` existe (et est compromis, cf. C1), mais aucune étape CI ne génère/upload la couverture. | 2 | 1 | 2 | 12 |
| T3 | Pas de test pour le wrapper `BMx280` (le fichier d'en-tête expose `GetTemperature/GetHumidity/GetPressure`, aucun test ne vérifie les passerelles). | 3 | 2 | 3 | 15 |
| T4 | Pas de test pour `SoftSerial` — blocage lié à C6 (dépendance à `_delay_us`). | 2 | 2 | 2 | **16** |
| T5 | Pas de test HW-in-the-loop ni Simavr-based exécuté en CI, malgré la présence de `[env:SOLAR_TEST]` configuré pour simavr. Infrastructure prête mais non exploitée. | 3 | 3 | 4 | 12 |
| T6 | Les tests de timings Oregon/Lacrosse valident une *chaîne de caractères d'ordres* (`"LDH PLD"` = 1, `"HDL PHD"` = 0) via `TestHal.Orders`. C'est élégant mais ne vérifie pas les timings réels ni l'interop avec RTL_433/RFLink que le README cite comme référence. | 3 | 3 | 4 | 12 |
| T7 | `test_busses/bussesTest.cpp` ne teste qu'un booléen `I2CIsConfigured` après `Init()` — valeur de régression nulle. | 2 | 1 | 2 | 12 |

### 3.4 Dette de dépendances

| ID | Item | Impact | Risque | Effort | Priorité |
|----|------|:------:|:------:|:------:|:--------:|
| ~~D1~~ | ~~**Image Docker `php:7.4-apache` EOL depuis novembre 2022.** Utilisée comme base du firmware builder web.~~ — ✅ **Résolu 2026-06-18** (supprimé avec `web/` — plus de Dockerfile PHP dans le repo) | 4 | 5 | 2 | ~~36~~ |
| ~~D2~~ | ~~GitHub Actions obsolètes : `actions/checkout@v3`, `actions/cache@v3`, `actions/setup-python@v4`. Python 3.9 utilisé — EOL octobre 2025 (nous sommes en avril 2026).~~ — ✅ **Résolu 2026-04-19** (v4/v4/v5, Python 3.12) | 3 | 3 | 1 | ~~30~~ |
| ~~D3~~ | ~~Dépendances PlatformIO tirées de forks personnels sans épinglage de commit.~~ — ✅ **Résolu 2026-06-18** (toutes les 5 dépendances épinglées sur leur SHA HEAD courant dans `platformio.ini`) | 4 | 4 | 3 | ~~24~~ |
| ~~D4~~ | ~~`bh1750` épinglé sur la branche `feat/attiny_support` au lieu d'un SHA.~~ — ✅ **Résolu 2026-06-18** (subsumed par D3 — épinglé sur `6d06b87`) | 3 | 3 | 2 | ~~24~~ |
| ~~D5~~ | ~~`avrdude.conf` (534 KB) commité dans le repo.~~ — ✅ **Résolu 2026-06-18** (`git rm avrdude.conf` ; vient du toolchain PlatformIO, pas de la source) | 2 | 2 | 1 | ~~20~~ |
| ~~D6~~ | ~~`web/build.php` charge jQuery 1.12.4 depuis CDN (sortie 2016, EOL).~~ — ✅ **Résolu 2026-06-18** (supprimé avec `web/`) | 1 | 2 | 2 | ~~12~~ |
| D7 | Copies tierces vendored dans `TinySensor/third_party/` (BMX, I2C, X10) figées, risque de divergence vs. upstream. | 2 | 2 | 2 | 16 |
| ~~D8~~ | ~~Badge README pointe vers **Travis CI** (`api.travis-ci.org`) — service fermé aux OSS non payants depuis 2020. Badge cassé = signal négatif pour les contributeurs.~~ — ✅ **Résolu 2026-06-16** (badge GitHub Actions dans `README.md`) | 1 | 1 | 1 | ~~10~~ |
| ~~D9~~ | ~~Pas de configuration Dependabot/Renovate. Dérive des versions non notifiée.~~ — ✅ **Résolu 2026-06-16** (`.github/dependabot.yml` : hebdomadaire pour `github-actions`, mensuel pour `docker` + `pip`) | 2 | 3 | 1 | ~~25~~ |

### 3.5 Dette de documentation

| ID | Item | Impact | Risque | Effort | Priorité |
|----|------|:------:|:------:|:------:|:--------:|
| ~~Doc1~~ | ~~Sémantique des environnements `S_02`..`S_05` non documentée.~~ — ✅ **Résolu 2026-06-18** (`docs/environments.md` créé : convention de nommage PCB, guide de calibration `INTERNAL_1v1`, table des boards physiques et envs spéciaux, seuils batterie) | 3 | 2 | 2 | ~~20~~ |
| Doc2 | Pas de guide d'onboarding (stack requise, setup local, premier build). Le README est orienté utilisateur/hardware, pas contributeur. | 3 | 2 | 2 | **20** |
| Doc3 | Workflow de flashage non documenté : `scripts/flash_tinySensor` + `waitForUpload` supposent une organisation NAS spécifique (`/var/services/homes/aurelien/…`). | 2 | 2 | 2 | 16 |
| Doc4 | Spécifications Oregon et Lacrosse référencées en commentaires (*"A copy of the document should be provided along the implementation"*) mais les docs PDF ne sont pas au repo. Lien externe ou fichier embarqué requis. | 2 | 2 | 2 | 16 |
| Doc5 | Absent : `CONTRIBUTING.md`, `SECURITY.md` (surtout pertinent vu C1), `CODE_OF_CONDUCT.md`, `CHANGELOG.md`. | 2 | 1 | 2 | 12 |
| Doc6 | Hardware : pas de BOM consolidé, pas de guide d'assemblage dans `doc/` (seulement 3 images : `boardv2.png`, `boardv2_copper.png`, `schematic.png`). | 2 | 1 | 3 | 9 |
| ~~Doc7~~ | ~~Badge Travis cassé dans README (doublon avec D8, traité côté deps).~~ — ✅ **Résolu 2026-06-16** (badge remplacé par GitHub Actions dans `README.md`) | 1 | 1 | 1 | ~~10~~ |

### 3.6 Dette d'infrastructure

| ID | Item | Impact | Risque | Effort | Priorité |
|----|------|:------:|:------:|:------:|:--------:|
| ~~I1~~ | ~~**Artefacts de build commités** dans `TinySensor/Debug/` : `.o`, `.d`, `.elf`, `.hex`, `.lss`, `.map`, `.srec`.~~ — ✅ **Résolu 2026-04-19** (audit revisité : aucun artefact n'était réellement tracké ; `TinySensor/Debug/` désormais couvert par `.gitignore`). | 3 | 2 | 1 | ~~25~~ |
| ~~I2~~ | ~~Chemins absolus spécifiques à l'auteur dans `Makefile` et `scripts/flash_tinySensor`.~~ — ✅ **Résolu 2026-06-18** (`PIO ?= $(HOME)/.platformio/penv/bin/pio`, `TARGET ?= S_03` dans `Makefile` ; `PIO_HOME` et `PROJECT_DIR` avec défauts `${HOME}/.platformio` et `$(dirname $0)/..` dans le script) | 3 | 2 | 1 | ~~25~~ |
| ~~I3~~ | ~~Archive de backup KiCAD commitée (`tinySensorv1-2025-05-01_183922.zip`).~~ — ✅ **Résolu 2026-06-18** (audit revisité : le ZIP n'était pas tracké par git — sur disque uniquement ; `*.zip` déjà couvert par `.gitignore`) | 2 | 2 | 1 | ~~20~~ |
| ~~I4~~ | ~~La CI ne produit aucun artefact (firmware `.hex` téléchargeable).~~ — ✅ **Résolu 2026-06-16** (job `release` ajouté sur `v*` tags : télécharge tous les artefacts et publie une GitHub Release avec notes auto-générées — complète la résolution partielle du 2026-04-19) | 2 | 1 | 2 | ~~12~~ |
| I5 | CI single-runner, single-python, single-env. Pas de matrice pour tester différentes versions d'outillage. | 3 | 2 | 2 | **20** |
| I6 | Pas de pre-commit ni de `.editorconfig`. `clang-format` est installé dans le devcontainer mais non appliqué automatiquement (le commit `68cefb8 chore: clang-format` laisse penser à une passe manuelle ponctuelle). | 2 | 1 | 2 | 12 |
| ~~I7~~ | ~~`.gitignore` minimaliste (12 lignes) : n'exclut pas `Debug/` (d'où I1), pas `*.o`, `*.d`, `*.hex` au root.~~ — ✅ **Résolu 2026-04-19** (ligne 1 buggée supprimée, patrons d'artefacts ajoutés, structure commentée). | 2 | 2 | 1 | ~~20~~ |
| I8 | Pas de release / tag git visible sur les 30 derniers commits. Versioning absent. | 2 | 1 | 2 | 12 |

---

## 4. Classement prioritaire (top 20)

> Les items barrés ont été traités depuis la publication initiale (cf. §1bis). Ils restent listés pour traçabilité de l'effort consenti.

| Rang | ID | Item | Cat. | Score |
|:----:|----|------|:----:|:-----:|
| — | ~~T1~~ | ~~CI ne compile aucune cible AVR~~ ✅ | Test | ~~36~~ |
| — | ~~D1~~ | ~~Image Docker PHP 7.4 EOL~~ ✅ | Dep | ~~36~~ |
| — | ~~C1~~ | ~~Token Coveralls en clair dans le repo~~ ✅ | Code | ~~35~~ |
| — | ~~A1~~ | ~~Arbre source dupliqué `TinySensor/`~~ → requalifié, risque accepté (score 20, voir §3.2) | Archi | ~~32~~ |
| — | ~~D2~~ | ~~GitHub Actions obsolètes + Python 3.9 EOL~~ ✅ | Dep | ~~30~~ |
| — | ~~C2~~ | ~~Macro `OREGON_DELAY_US` réfère une méthode inexistante~~ ✅ | Code | ~~30~~ |
| — | ~~M1~~ | ~~PRR bit inversé dans `UseLessPowerAsPossible()`~~ ✅ | Code | ~~30~~ |
| — | ~~M2~~ | ~~`SetChannel()` sans validation de bornes~~ ✅ | Code | ~~30~~ |
| — | ~~D9~~ | ~~Pas de Dependabot/Renovate~~ ✅ | Dep | ~~25~~ |
| — | ~~I1~~ | ~~Artefacts de build commités (`Debug/`)~~ ✅ | Infra | ~~25~~ |
| — | ~~I2~~ | ~~Chemins absolus auteur dans Makefile/scripts~~ ✅ | Infra | ~~25~~ |
| — | ~~D3~~ | ~~Deps pointent vers forks personnels non épinglés~~ ✅ | Dep | ~~24~~ |
| — | ~~D4~~ | ~~`bh1750` épinglé sur une branche au lieu d'un tag/SHA~~ ✅ | Dep | ~~24~~ |
| — | ~~C3~~ | ~~`Init()` orphelin dans `Attiny84aHal.cpp`~~ ✅ | Code | ~~20~~ |
| — | ~~C4~~ | ~~Dead-code `if (int(count) != count)` dans `Sum`~~ ✅ | Code | ~~20~~ |
| 8 | C5 | `main.cpp` accède aux registres sans passer par le HAL | Code | 20 |
| 8 | C6 | `SoftSerial` utilise `_delay_us` directement (non testable) | Code | 20 |
| 8 | A3 | Logique BH1750 mélangée dans `main.cpp` | Archi | 20 |
| — | ~~D5~~ | ~~`avrdude.conf` (534 KB) commité~~ ✅ | Dep | ~~20~~ |
| — | ~~Doc1~~ | ~~Envs `S_0x` non documentés (tribal knowledge)~~ ✅ | Doc | ~~20~~ |
| 8 | Doc2 | Pas de guide d'onboarding | Doc | 20 |
| — | ~~I3~~ | ~~Backup ZIP KiCAD commité~~ → non tracké git ✅ | Infra | ~~20~~ |
| 8 | I5 | Matrice CI inexistante | Infra | 20 |
| — | ~~I7~~ | ~~`.gitignore` minimaliste~~ ✅ | Infra | ~~20~~ |
| — | ~~M3~~ | ~~`message[]` variable globale partagée entre instances~~ ✅ | Code | ~~16~~ |
| — | ~~C7~~ | ~~Duplication massive `platformio.ini`~~ ✅ | Code | ~~18~~ |
| — | ~~C10~~ | ~~`USE_BMP280=1` en double dans `env:S_03`~~ ✅ | Code | ~~15~~ |
| — | ~~I4~~ | ~~Pas de release automation sur tag~~ ✅ | Infra | ~~12~~ |
| — | ~~D8~~ | ~~Badge Travis mort~~ ✅ | Dep | ~~10~~ |

---

## 5. Plan de remédiation phasé

Plan dimensionné pour 1 développeur à ~20 % de son temps. Chaque phase s'intercale avec du travail feature ; aucune ne bloque les autres.

### Phase 0 — Sécurité immédiate (jour 0–1, < 2 h)

Un seul item, non négociable : **révoquer le token Coveralls** (`C1`). Révoquer côté coveralls.io, supprimer le token du fichier, le déplacer dans un secret GitHub Actions `COVERALLS_REPO_TOKEN`, et adapter le workflow. Même si le repo est public et que l'expo a déjà eu lieu, la rotation est obligatoire.

### Phase 1 — Stabilisation CI & outillage (semaine 1–2, ~1 jour) — **partiellement faite 2026-04-19**

Objectif : ne plus pouvoir casser le firmware de prod sans le voir dans la CI, et remettre l'outillage au goût du jour.

Actions, par ordre de valeur :

1. ~~**T1 — Ajouter la compilation AVR à la CI**. Étendre `.github/workflows/main.yml` avec une matrice qui fait `pio run -e S_02 -e S_03 -e S_04 -e S_05 -e robot -e devmodule` en plus du `pio test -e native_debug`. Coût : 30 min.~~ ✅ **Fait 2026-04-19** (matrice sur 7 envs, `fail-fast: false`).
2. ~~**D2 — Mettre à jour les Actions** : `checkout@v4`, `cache@v4`, `setup-python@v5`, Python `3.12`. Coût : 15 min.~~ ✅ **Fait 2026-04-19**.
3. ~~**I1, I7 — Nettoyer les artefacts versionnés**. Ajouter à `.gitignore` : `Debug/`, `Release/`, `*.o`, `*.d`, `*.elf`, `*.hex` (root seulement), `*.lss`, `*.map`, `*.srec`, `TinySensor/Debug/`. Puis `git rm -r --cached TinySensor/Debug/` et commit.~~ ✅ **Fait 2026-04-19** (cf. §1bis ; `git rm --cached` s'est révélé no-op, aucun artefact n'était réellement tracké).
4. **I2 — Paramétrer les scripts**. Remplacer les chemins absolus du `Makefile` et de `scripts/flash_tinySensor` par des variables d'environnement (`PIO_HOME`, `PROJECT_DIR`). Documenter dans le README. Coût : 1 h.
5. **D9 — Activer Dependabot** (`.github/dependabot.yml`) pour `github-actions`, `docker`, `pip` (le python utilisé dans les scripts). Coût : 10 min.
6. **D8 + Doc7 — Retirer le badge Travis**, ajouter le badge GitHub Actions. Coût : 5 min.
7. **I4 (partie restante) — Release automation sur tag** : workflow `on: push: tags: ['v*']` qui rejoue la matrice et publie les `.hex` / `.elf` en GitHub Release. Coût : 1 h. *Déplacé ici depuis la phase 4 car le socle artefacts est désormais en place.*

Livrable de fin de phase : CI verte sur **toutes les cibles AVR + natif** (atteint), plus aucune modification ne peut casser silencieusement un firmware de production (atteint), artefacts téléchargeables par build (atteint), et releases taguées automatiques (à faire).

### Phase 2 — Consolidation de l'arbre source (semaine 3–5, ~3 jours cumulés)

Objectif : une seule source de vérité pour chaque fichier, dépendances reproductibles.

1. **A1 — Décision sur `TinySensor/` (legacy Atmel Studio)**. Deux options :
    - (a) Supprimer intégralement le dossier. C'est ce que je recommande : le projet PlatformIO couvre tout l'usage, Atmel Studio n'est plus maintenu (Microchip Studio 7.0 également en fin de support). Coût : 10 min + audit de ce qui serait perdu.
    - (b) Le conserver en tant que branche ou submodule séparé, hors de `master`.
    Action : ouvrir une PR "remove legacy Atmel Studio tree" avec une fenêtre de confirmation d'une semaine.
2. **D3, D4 — Épingler les dépendances PlatformIO**. Passer de `url.git#branch` à `url.git#<SHA>` (ou mieux, créer des tags `v1.0.0` sur chaque fork et utiliser `#v1.0.0`). Coût : 1 h.
3. **D5 — Sortir `avrdude.conf`** du repo. La config vient du paquet `tool-avrdude` de PlatformIO ; documenter dans le README et ajouter au `.gitignore`. Coût : 15 min.
4. **I3 — Supprimer le backup ZIP KiCAD**. L'historique git suffit. Coût : 5 min.
5. **D1 — Migrer le Dockerfile** de `php:7.4-apache` vers `php:8.3-apache` (ou plus récent). Tester le build. Coût : 2 h + test.

Livrable : un seul arbre source, deps reproductibles, image Docker à jour.

### Phase 3 — Qualité du code firmware (semaine 6–9, ~3 jours cumulés)

Objectif : éliminer la dette de code identifiée, renforcer les tests.

Quick wins (1 h chacun) :

1. **C2** — Corriger la macro `OREGON_DELAY_US` dans `TestHal.h`.
2. **C3** — Supprimer la fonction libre `Init()` orpheline.
3. **C4** — Supprimer le `if` mort dans `OregonV3::Sum`.
4. **C10** — Dédupliquer `USE_BMP280=1` dans `env:S_03`.
5. **C11** — Retirer le `if (pressure < 200)` mort.
6. **C15** — Ajouter `override` sur toutes les méthodes virtuelles de `TestHal`.

Refactorings moyens (1 jour chacun) :

7. **C5 + A3** — Encapsuler les accès DS18B20 et BH1750 derrière des classes capteurs (`Ds18b20Sensor`, `Bh1750Sensor`) utilisant le HAL. Alignerait sur le pattern `BMx280`.
8. **C6 + T4** — Refactorer `SoftSerial` pour passer toutes les temporisations par le HAL (`hal->DelayBitPeriod()` ou similaire). Ajouter un test natif de `SoftSerial` qui vérifie la séquence de `SerialGoHigh/Low` pour un caractère connu.
9. **C7 — Dédupliquer `platformio.ini`** : utiliser `[env:base_avr]` héritée par `[env:S_02]`, etc. (feature native de PlatformIO via `extends = env:base_avr`). Coût : 2 h + test.
10. **C12** — Centraliser `BITREAD` dans `lib/utils/include/bitops.h` et supprimer les redéfinitions.
11. **T3** — Ajouter un test unitaire pour `BMx280` en mode non-AVR (vérifie que les stubs renvoient 0 sans crash).
12. **C8** — Documenter les magic numbers (commentaire avec référence datasheet + lien).

### Phase 4 — Architecture & documentation (trimestre Q2/Q3, au fil de l'eau)

1. **A2** — Introduire une interface `IEnvironmentEncoder` partagée par `OregonV3` et `LacrosseWS7000`, injectée dans `main()`. Simplifie l'ajout de futurs protocoles.
2. **A4** — Statuer sur le sort de `web/` : (a) le finir (formulaire + sanitisation) ou (b) le supprimer et documenter un builder local. Je recommande (b) vu l'état.
3. **Doc1, Doc2** — Écrire `docs/environments.md` (quel env pour quelle carte physique) et `docs/CONTRIBUTING.md` (stack, premier build, architecture HAL/encoders).
4. **Doc5** — Créer `SECURITY.md` (processus de report) et `CHANGELOG.md` (tenir à jour sur les prochaines releases).
5. **I5 — Matrice d'outillage** : étendre la matrice existante à plusieurs Pythons (`3.11`, `3.12`, `3.13`) et/ou plusieurs versions de PlatformIO Core. (La partie release automation d'I4 a été remontée en Phase 1.)
6. **T5, T6 — Élever la barre des tests** : intégrer `simavr` dans la CI pour au moins un env (`SOLAR_TEST` est prêt), et ajouter un test d'intégration qui fait passer la trame Oregon au décodeur RTL_433 (binaire installable en CI) pour valider l'interop réelle.
7. **A5 — Évaluation** : runtime config via EEPROM ? Pertinent seulement si on vise plus de 5 unités en production. Ne pas faire avant d'en avoir le besoin.

---

## 6. Ce qui va bien (à préserver)

Pour équilibrer, voici les forces du projet à ne pas casser lors des refactorings :

**Le HAL est bien pensé** (`Hal.h` → `Attiny84aHal` / `TestHal_`) : abstraction claire, méthode template (`ComputeVccMv`, `ConvertAnalogValueToMv`) dans la classe de base, substituable pour les tests. C'est le patron à généraliser.

**Les encodeurs protocolaires sont testés et déterministes** : `OregonV3` et `LacrosseWS7000` sont vérifiés via une trace d'ordres HAL (`Orders.push_back('H')`), ce qui permet d'asserter la séquence de bits sans matériel. Approche élégante.

**Les commentaires inline** dans les encodeurs documentent le format de trame (structure des nibbles, bits de battery status, checksum) — précieux pour un domaine où les specs sont rares.

**Le `Dockerfile` devcontainer** est complet et autosuffisant pour onboarder un nouveau développeur — le point d'entrée technique est clair.

**Les commits** suivent une convention (`feat:`, `fix:`, `chore:`, `hardware:`, `optim:`) — base pour semver automatique quand les releases seront outillées (phase 4).

---

## 7. Annexes

### 7.1 Métriques

| Métrique | Valeur |
|---|---|
| Lignes de code (lib + src + include) | 1 543 |
| Lignes de tests | 1 835 |
| Ratio tests/code | 1,19 |
| Fichiers `.cpp`/`.h` audités (hors legacy) | 19 |
| TODO/FIXME/HACK restants | 1 |
| Environnements PlatformIO | 8 (1 natif + 7 AVR) |
| Dépendances externes | 5 (toutes forks perso) |
| Workflows CI | 1 (2 jobs : native + matrice AVR 7 envs, maj 2026-04-19) |
| Secrets exposés | 0 (C1 révoqué) |

### 7.2 Items non retenus (hors scope)

Les éléments suivants ont été vus mais ne justifient pas d'action immédiate :

- *Qualité du KiCAD* (hardware) : hors compétence d'une revue de code. À confier à un reviewer hardware.
- *Choix du langage* (C++ sur ATtiny) : adapté au contexte mémoire/ressources. Pas de dette.
- *Format `float` pour la température* (`Oregon_v3.cpp`) : acceptable vu la place disponible ; à reconsidérer seulement si la taille flash devient critique.

### 7.3 Commandes utiles pour la remédiation

```bash
# Phase 1 — vérifier que la CI couvre toutes les cibles AVR
pio run -e S_02 -e S_03 -e S_04 -e S_05 -e robot -e devmodule -e SOLAR_TEST

# Phase 2 — purger l'arbre legacy (après décision)
git rm -r TinySensor/
git rm avrdude.conf
git rm hardware/tinysensor/tinySensorv1-backups/

# Phase 3 — dédupliquer platformio.ini avec extends
# [env:base_avr]
#   platform = atmelavr
#   board = attiny84
#   framework = arduino
#   board_build.f_cpu = 1000000L
#   board_build.mcu = attiny84a
#   build_type = release
# [env:S_02]
#   extends = env:base_avr
#   build_flags = -DUSE_OREGON -DOREGON_RCODE=0x20 …
```

---

*Fin du rapport. Auteur : Claude. Date : 19 avril 2026. Ce rapport reflète l'état du dépôt au commit `aa53a83`. Les scores sont indicatifs et doivent être recalibrés selon la capacité équipe et les priorités produit.*
