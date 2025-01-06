# Firmware Titan pour T-Display-S3
![IMG_20250106_223322](https://github.com/user-attachments/assets/f9da2d12-33a8-4344-a816-5c0d78def7cf)
## Description
Le **firmware Titan** est un projet conçu pour le **T-Display-S3** basé sur le microcontrôleur **ESP32-S3** de **Lilygo**. 

## Fonctionnalités
- **Menu Principal :** Navigation facile pour sélectionner diverses options comme "Wifi Attack", "BLE Attack", "Infos" et "Paramètres".
- **Paramètres :** Modification de la couleur de l'interface et de la luminosité de l'écran.
- **Affichage d'Informations :** Affiche des détails système, tels que le modèle de la puce, la taille de l'écran, la capacité de la mémoire Flash, etc.
- **Interaction avec les boutons :** Utilisation des boutons A et B pour naviguer et ajuster les paramètres.

## Matériel requis
- **Carte :** T-Display-S3 (basée sur l'ESP32-S3)
- **Ordinateur :** Windows ou Linux avec l'IDE Arduino ou PlatformIO.

## Installation

### Prérequis logiciels
- **Arduino IDE** ou **PlatformIO**
- **Bibliothèque TFT_eSPI** pour la gestion de l'écran TFT

### Installation avec Arduino IDE
1. Installez la **bibliothèque TFT_eSPI** via le dépôt officiel du firmware, dans le dossier `lib/TFT_eSPI`.
2. Ajoutez l'URL suivante dans les **préférences de l'IDE Arduino** pour obtenir le support de la carte ESP32 :  
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. Sélectionnez la carte **ESP32S3 Dev Module** dans le menu **Outils** > **Carte**.
4. Configurez les paramètres suivants :
   - **Flash Size :** 16MB (128Mb)
   - **Partition Scheme :** 16M Flash (3M APP/9.9MB FATFS)
   - **PSRAM :** OPI PSRAM
   - **Upload Speed :** 921600
   - **Upload Mode :** UART0/Hardware CDC
5. Connectez la carte via USB et téléchargez le firmware en cliquant sur le bouton **Upload**.

## Fonctionnement du Firmware

### Écran de démarrage
Au démarrage, le firmware affiche un écran de bienvenue avec le logo "Titan Firmware" et une barre de progression animée (à améliorer dans les prochaines versions).

### Menu Principal
Le menu principal propose les options suivantes :
1. **Wifi Attack**
2. **BLE Attack**
3. **Infos** - Affiche les informations système détaillées.
4. **Paramètres** - Permet de modifier la couleur de l'interface et la luminosité de l'écran.

### Navigation
- **Bouton A (GPIO0)** : Accède à la catégorie sélectionnée ou affiche les informations de cette catégorie.
- **Bouton B (GPIO14)** : Permet de naviguer entre les catégories du menu.

### Paramètres
Dans le menu "Paramètres", vous pouvez :
- Modifier la couleur principale de l'interface parmi cinq couleurs prédéfinies.
- Ajuster la luminosité de l'écran par incréments de 20 (de 0 à 255).

## Code

Le code est structuré de manière simple et optimisée, avec des fonctions principales pour la gestion du menu et de l'affichage :

- **setup()** : Initialisation des périphériques et de l'écran.
- **loop()** : Gestion des appuis sur les boutons et mise à jour de l'affichage.
- **Fonctions de gestion des menus :**
  - `displayMenu()`
  - `enterCategory()`
  - `nextCategory()`
  - `displaySettings()`
  - `displayParameters()`
  - `waitForButtonPress()`

## Aide

### Problèmes courants

#### L'écran ne s'allume pas après le téléchargement du firmware
- Vérifiez que le **GPIO15** est bien configuré en sortie et à un niveau **HIGH** pour activer le rétroéclairage de l'écran.

#### La luminosité ne se modifie pas correctement
- Assurez-vous que la fonction `analogWrite()` est utilisée pour ajuster la luminosité via le pin de rétroéclairage.

### Dépannage USB

- Si le téléchargement du firmware échoue, appuyez simultanément sur les boutons **BOOT** et **RST** pour entrer en mode de téléchargement, puis réessayez.
