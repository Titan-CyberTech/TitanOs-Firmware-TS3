# Firmware Titan pour T-Display-S3

![IMG_20250106_223322](https://github.com/user-attachments/assets/f9da2d12-33a8-4344-a816-5c0d78def7cf)

## Description
Le **firmware Titan** est un projet conçu pour le **T-Display-S3**, basé sur le microcontrôleur **ESP32-S3** de **Lilygo**. Il offre une interface graphique fluide et un accès simplifié aux paramètres et informations système.

## Fonctionnalités
- **Menu Principal :** Navigation intuitive avec accès aux options "WiFi Attack", "BLE Attack", "Infos", "Paramètres" et "Batterie Info".
- **Paramètres :** Personnalisation de l'interface (couleurs et luminosité de l'écran).
- **Affichage d'Informations :** Détails sur le matériel, tels que le modèle de la puce, la taille de l'écran et la mémoire Flash.
- **Gestion de la Batterie :** Affichage de la tension via l'ADC de l'ESP32 pour une surveillance en temps réel.
- **Contrôle via boutons :** Utilisation des boutons A et B pour naviguer et modifier les paramètres.
- **Écran de démarrage personnalisé** avec un logo de bienvenue.

## Matériel requis
- **Carte :** T-Display-S3 (ESP32-S3)
- **Ordinateur :** Windows, Linux ou macOS avec **Arduino IDE** ou **PlatformIO**.

## Installation

### Prérequis logiciels
- **Arduino IDE** ou **PlatformIO**
- **Bibliothèque TFT_eSPI** pour l'affichage TFT

### Installation avec Arduino IDE
1. Installez **TFT_eSPI** via le dossier `lib/TFT_eSPI` du projet.
2. Ajoutez cette URL dans **Préférences** de l'IDE Arduino :  
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. Sélectionnez **ESP32S3 Dev Module** dans **Outils** > **Carte**.
4. Configurez les paramètres suivants :
   - **Flash Size :** 16MB (128Mb)
   - **Partition Scheme :** 16M Flash (3M APP/9.9MB FATFS)
   - **PSRAM :** OPI PSRAM
   - **Upload Speed :** 921600
   - **Upload Mode :** UART0/Hardware CDC
5. Connectez la carte via USB et cliquez sur **Upload**.

## Fonctionnement du Firmware

### Écran de démarrage
Au démarrage, le firmware affiche un logo "Titan Firmware" pendant 5 secondes avant d'accéder au menu principal.

### Menu Principal
Le menu propose :
1. **WiFi Attack**
2. **BLE Attack**
3. **Infos** - Affichage des informations système
4. **Paramètres** - Personnalisation de l'affichage
5. **Batterie Info** - Surveillance de l'état de la batterie

### Navigation
- **Bouton A (GPIO0)** : Sélectionner une option.
- **Bouton B (GPIO14)** : Naviguer dans le menu.

### Paramètres
Dans le menu "Paramètres" :
- Changer la couleur principale (5 options disponibles).
- Régler la luminosité (0 à 255 par paliers de 20).

### Gestion de la Batterie
- Lecture de la tension via **GPIO4**.
- Affichage de la valeur en millivolts.
- Message "No battery connected!" si aucune batterie n'est détectée.

## Code
Le code est structuré avec des fonctions principales pour la gestion de l'affichage et des menus :

- **setup()** : Initialisation des périphériques et de l'écran.
- **loop()** : Gestion des interactions utilisateur et mise à jour de l'affichage.
- **Fonctions du menu :**
  - `displayMenu()`
  - `enterCategory()`
  - `nextCategory()`
  - `displaySettings()`
  - `displayBatteryInfo()`
  - `waitForButtonPress()`

## Dépannage

### L'écran ne s'allume pas
- Vérifiez que **GPIO15** est bien configuré en sortie et mis à **HIGH** pour activer le rétroéclairage.

### La luminosité ne change pas
- Assurez-vous que la fonction `analogWrite()` est bien utilisée sur le pin du rétroéclairage.

### Problèmes USB
- En cas d'échec du flash, maintenez **BOOT** et appuyez sur **RST** pour entrer en mode téléchargement.
