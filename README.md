# Titan Firmware TS3 for T-Display-S3

![IMG_0081](https://github.com/user-attachments/assets/48c1a630-4036-4e47-841f-b060a08f3809)

## Description

**Titan Firmware** is a complete firmware for the **T-Display-S3** board (ESP32-S3 Lilygo).  
It offers a modern graphical interface, WiFi/BLE attacks, a Snake game, battery management, advanced settings, and a captive portal (Evil Portal).

## Features

- **Graphical menu**: intuitive navigation with A/B buttons
- **WiFi attacks**:
  - **Deauth Attack**: disconnect WiFi clients
  - **Evil Portal**: fake access point to capture credentials
- **BLE Attack** (placeholder)
- **Built-in Snake game**
- **Settings**: main color and screen brightness
- **Battery display**: voltage, percentage, battery detection
- **System info**: version, hardware, etc.
- **Custom splash screen**
- **Auto sleep** after 15s of inactivity
- **Full shutdown** (deep sleep) via menu

## Required Hardware

- **Board**: Lilygo T-Display-S3 (ESP32-S3, 16MB Flash, 8MB PSRAM recommended)
- **Computer**: Windows, Linux, or macOS with **Arduino IDE** or **PlatformIO**

## Installation

### Software Prerequisites

- **Arduino IDE** or **PlatformIO**
- **TFT_eSPI** (TFT display library)
- **WebServer** and **DNSServer** (ESP32 libraries)
- (Optional) **SD** for storage

### Steps (Arduino IDE)

1. Install **TFT_eSPI** via the library manager.
2. Add the ESP32 URL in Preferences:  
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. Select **ESP32S3 Dev Module** in Tools > Board.
4. Configure:
   - **Flash Size**: 16MB (128Mb)
   - **Partition Scheme**: 16M Flash (3M APP/9.9MB FATFS)
   - **PSRAM**: OPI PSRAM
   - **Upload Speed**: 921600
   - **Upload Mode**: UART0/Hardware CDC
5. Connect the board via USB and click **Upload**.

## Usage

### Navigation

- **Button A (GPIO0)**: Select/Validate
- **Button B (GPIO14)**: Navigate/Change option

### Menus

- **WiFi Attack**: Deauth or Evil Portal
- **BLE Attack**: (placeholder)
- **Games**: Snake (control with A/B)
- **Info**: System information
- **Settings**: Main color and brightness
- **Battery Info**: Battery voltage and percentage
- **Turn Off**: Screen off and deep sleep

### Settings

- **Main color**: 5 choices
- **Brightness**: 0 to 255 in steps of 20

### Battery management

- Read via **GPIO4**
- Display voltage (mV) and percentage
- "No battery connected!" message if absent

### Sleep & shutdown

- **Auto sleep** after 15s of inactivity (black screen, wake up with button)
- **Full shutdown** via menu (deep sleep, wake up with button)

## Code Structure

```
Titan_Firmware_TS3/
├── Titan_Firmware_TS3.ino      # Main file (setup, loop)
├── config.h / config.cpp       # Global declarations and initializations (pins, colors, objects)
├── display.h / display.cpp     # Display functions (menus, splash, battery)
├── menu.h / menu.cpp           # Main menu and navigation
├── attacks.h / attacks.cpp     # WiFi/BLE attacks and captive portal
├── utils.h / utils.cpp         # Utilities (buttons, battery, helpers)
├── games.h / games.cpp         # Snake game
```

## Troubleshooting

- **Screen does not turn on**: Check GPIO15 (backlight) is set as output and HIGH.
- **Brightness does not change**: Check `ledcWrite()` is used on the correct channel.
- **USB flashing issues**: Hold **BOOT** and press **RST** to enter download mode.

## Security & Legal Notice

> ⚠️ **Warning: WiFi/BLE attacks are for educational use only. Never use these features on networks you do not own or without explicit permission.**

---

**Author:** Titan.cybertech  
**License:** MIT