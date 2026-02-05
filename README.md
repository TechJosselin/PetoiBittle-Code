# PetoiBittle-Mod
Refonte complète de l’électronique et du firmware du robot chien Bittle (Alloy) de Petoi autour d’un ESP32, avec architecture modulaire (servos, capteurs, caméra) et documentation du projet.
Refonte complète de l’électronique et du firmware du robot chien **Bittle (Alloy)** de :contentReference[oaicite:0]{index=0} autour d’un **ESP32**, avec une architecture modulaire pour ajouter des fonctionnalités (capteurs, caméra, audio, etc.) et une documentation technique exploitable.

> Objectif principal : reprendre la maîtrise totale de la chaîne **servo → contrôle → comportements**, en remplaçant la carte d’origine par une plateforme ESP32 robuste, maintenable et extensible.

---

## Sommaire
- [Objectifs](#objectifs)
- [Périmètre](#périmètre)
- [Architecture logicielle](#architecture-logicielle)
- [Architecture matérielle](#architecture-matérielle)
- [Structure du dépôt](#structure-du-dépôt)
- [Démarrage rapide](#démarrage-rapide)
- [Configuration](#configuration)
- [Tests & validation](#tests--validation)
- [Roadmap](#roadmap)
- [Contribution](#contribution)
- [Sécurité & avertissements](#sécurité--avertissements)
- [Licence](#licence)

---

## Objectifs
- Remplacer l’électronique d’origine par un contrôleur **ESP32** (bring-up, drivers, intégration).
- Refaire la **gestion des servomoteurs** (calibration, limites, sécurité, mouvements).
- Mettre en place une architecture **modulaire** : capteurs (IMU, ToF/ultrasons), caméra, audio, LED, etc.
- Documenter l’ensemble (schémas, pinout, BOM, procédures, décisions techniques).
- Faciliter le travail à deux : conventions, branches, issues, journal de décisions.

---

## Périmètre
### Inclus
- Firmware (PlatformIO + ESP-IDF) : drivers, contrôle servos, bus (I2C/SPI/UART), logs, sécurité.
- Matériel : câblage, intégration, schéma, éventuellement PCB (si prévu).
- Modules optionnels : caméra, capteurs variés, etc. (selon itérations).

### Non-inclus (pour l’instant)
- Refabrication mécanique lourde (hors supports simples).
- Autonomie avancée (SLAM, navigation complexe) tant que la base servo/IMU n’est pas stable.

---

## Architecture logicielle
Architecture en couches, pour limiter l’entropie quand on ajoute des modules :

- **HAL / Drivers** : GPIO, I2C, PWM/servo, UART, IMU, capteurs.
- **Services** : gestion servo (calibration, mapping, limites), bus manager, logger, config.
- **Control** : cinématique (si nécessaire), gait engine, posture, stabilisation (IMU).
- **Application** : modes (demo, télécommande, autonome), commandes, sécurité (failsafe).

Principes :
- Modules découplés, interfaces claires, dépendances orientées “vers le bas”.
- Logs structurés + niveaux (INFO/WARN/ERROR) pour debug terrain.
- Sécurité : watchdog, limites servo, arrêt d’urgence, comportement en cas de brown-out.

---

## Architecture matérielle
Cible initiale :
- ESP32 (ex: XIAO ESP32C6 ou autre carte ESP32 selon contraintes)
- Alimentation (à préciser : reprise existante vs régulation dédiée)
- Bus :
  - I2C (IMU / capteurs)
  - PWM/servo (direct ou driver type PCA9685 selon choix)
  - UART/SPI (option modules)