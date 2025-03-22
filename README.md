# Titan Firmware for T-Display-S3
![IMG_20250208_151310](https://github.com/user-attachments/assets/ce8cb149-f686-414a-9745-d9cae61c9791)
## Description
The **Titan Firmware** is a project designed for the **T-Display-S3**, based on the **ESP32-S3** microcontroller by **Lilygo**. It provides a smooth graphical interface and easy access to settings and system information. This updated version includes new features such as an Evil Portal attack mode, a Snake game, and improved user interaction.

## Features
- **Main Menu:** Intuitive navigation with access to "WiFi Attack", "BLE Attack", "Games", "Info", "Settings", "Battery Info", and "Turn Off".
- **WiFi Attack Modes:**
  - **Deauth Attack:** Continuously sends deauthentication packets to disrupt WiFi networks.
  - **Evil Portal:** Sets up a fake access point to capture credentials from unsuspecting users.
- **Snake Game:** A simple Snake game for entertainment, controlled using buttons A and B.
- **Settings:** Customize the interface (main color and screen brightness).
- **Information Display:** Details about the hardware, such as chip model, screen size, and Flash memory.
- **Battery Management:** Displays voltage via the ESP32's ADC for real-time monitoring.
- **Button Control:** Use buttons A and B to navigate and adjust settings.
- **Custom Startup Screen** with a welcome logo.
- **Battery Percentage Display:** Visual indication of battery charge level.
- **Turn Off Function:** Option to turn off the screen and enter deep sleep mode.
- **Inactivity Screen Off:** Automatically turns off the screen after a period of inactivity.

## Required Hardware
- **Board:** T-Display-S3 (ESP32-S3)
- **Computer:** Windows, Linux, or macOS with **Arduino IDE** or **PlatformIO**.

## Installation

### Software Prerequisites
- **Arduino IDE** or **PlatformIO**
- **TFT_eSPI Library** for TFT display

### Installation with Arduino IDE
1. Install **TFT_eSPI** via the project's lib/TFT_eSPI folder.
2. Add this URL in **Preferences** of the Arduino IDE:
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. Select **ESP32S3 Dev Module** in **Tools** > **Board**.
4. Configure the following settings:
   - **Flash Size:** 16MB (128Mb)
   - **Partition Scheme:** 16M Flash (3M APP/9.9MB FATFS)
   - **PSRAM:** OPI PSRAM
   - **Upload Speed:** 921600
   - **Upload Mode:** UART0/Hardware CDC
5. Connect the board via USB and click **Upload**.

## Firmware Operation

### Startup Screen
On startup, the firmware displays a "Titan Os" logo for 5 seconds before accessing the main menu.

### Main Menu
The menu offers:

1. **WiFi Attack**
   - **Deauth Attack:** Disrupt WiFi networks by sending deauthentication packets.
   - **Evil Portal:** Capture credentials by setting up a fake access point.
2. **BLE Attack**
3. **Games**
   - **Snake Game:** Play a simple Snake game using buttons A and B.
4. **Info** - Display system information
5. **Settings** - Customize display settings
6. **Battery Info** - Monitor battery status
7. **Turn Off** - Turn off the screen and enter deep sleep mode

### Navigation
- **Button A (GPIO0):** Select an option.
- **Button B (GPIO14):** Navigate through the menu.

### Settings
In the "Settings" menu:
- Change the main color (5 options available).
- Adjust brightness (0 to 255 in increments of 20).

### Battery Management
- Reads voltage via **GPIO4**.
- Displays value in millivolts.
- Displays "No battery connected!" if no battery is detected.
- Displays battery charge percentage.

### Turn Off Function
- Turns off the screen and enters deep sleep mode.
- Can be woken up via a configured button.

### Inactivity Screen Off
- Automatically turns off the screen after 15 seconds of inactivity.
- Press any button to wake up the screen.

## Code
The code is structured with main functions for display and menu management:

- **setup():** Initialize peripherals and screen.
- **loop():** Handle user interactions and update display.
- **Menu Functions:**
  - `displayMenu()`
  - `enterCategory()`
  - `displaySettings()`
  - `displayBatteryInfo()`
  - `displayBatteryPercentage()`
  - `turnOffDisplay()`
  - `waitForButtonPress()`
  - `displayWifiAttackSubMenu()`
  - `displayWifiDeauthAttack()`
  - `displayWifiEvilPortal()`
  - `displaySnakeGame()`

## Troubleshooting

### Screen Does Not Turn On
- Ensure **GPIO15** is configured as an output and set to **HIGH** to enable backlight.

### Brightness Does Not Change
- Ensure the `ledcWrite()` function is used on the LEDC channel configured for backlight.

### USB Issues
- If flashing fails, hold **BOOT** and press **RST** to enter download mode.
