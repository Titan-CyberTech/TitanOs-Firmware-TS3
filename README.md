<div align="center">

# TitanOs-Firmware-TS3

**A modern open-source firmware for the Lilygo T-Display-S3**

![Version](https://img.shields.io/badge/version-2.0.1-blueviolet?style=flat-square)
![Board](https://img.shields.io/badge/board-T--Display--S3-blue?style=flat-square)
![License](https://img.shields.io/badge/license-MIT-green?style=flat-square)
![Platform](https://img.shields.io/badge/platform-ESP32--S3-orange?style=flat-square)

</div>

> ⚠️ **Legal Notice** — WiFi attack features are strictly for **educational purposes only**.
> Never use them on networks you do not own or without explicit written permission.

---

## Overview

**TitanOs** is a complete firmware for the **Lilygo T-Display-S3** (ESP32-S3).
It features a modern double-buffered UI, WiFi attack tools, a Snake game, persistent settings, and full battery management — all navigated with just 2 buttons.

---

## Features

| Category | Details |
|----------|---------|
| 🎨 **UI** | Double-buffered sprite (no flicker), topbar/botbar, scrollable menu, 5 color themes |
| 📶 **WiFi Attacks** | Deauth (broadcast, live packet counter) · Evil Portal (fake Google login, real-time credential capture) |
| 🦷 **BLE** | Placeholder — coming in v2.1 |
| 🐍 **Snake** | Grid display, score, progressive speed, countdown, flashy game over |
| ⚙️ **Settings** | Theme & brightness, persisted to NVS (survive reboot) |
| 🔋 **Battery** | Topbar icon (color-coded), dedicated screen with large gauge + voltage |
| 💤 **Sleep** | Auto screen-off after 20s · Full deep sleep via menu (wake on button A) |

---

## Hardware

| Component | Detail |
|-----------|--------|
| Board | Lilygo T-Display-S3 (ESP32-S3) |
| Display | ST7789V 1.9″ — 320×170 px, 8-bit parallel |
| Flash | 16 MB |
| PSRAM | 8 MB OPI |
| Button A | GPIO0 (Boot button) |
| Button B | GPIO14 (Side button) |
| Backlight | GPIO38 (PWM via LEDC) |
| Battery ADC | GPIO4 (÷2 voltage divider) |

---

## Installation

### 1 — Arduino IDE setup

Add the ESP32 board URL in **File → Preferences**:
```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

Install **esp32 by Espressif** (v2.0.14+ recommended) via **Boards Manager**.

Install **TFT_eSPI** via **Library Manager**.

### 2 — Configure TFT_eSPI

In `TFT_eSPI/User_Setup_Select.h`, uncomment:
```cpp
#include <User_Setups/Setup206_LilyGo_T_Display_S3.h>
```

### 3 — Board settings (Tools menu)

| Setting | Value |
|---------|-------|
| Board | LilyGo T-Display-S3 (or ESP32S3 Dev Module) |
| Flash Size | 16MB (128Mb) |
| Partition Scheme | 16M Flash (3M APP/9.9MB FATFS) |
| PSRAM | OPI PSRAM |
| Upload Speed | 921600 |
| USB CDC On Boot | Disabled |

### 4 — Flash

Connect via USB-C and click **Upload**.

> If the port is not detected: hold **BOOT** then press **RST** to enter download mode.

---

## Controls

| Button | Short press | Long press (> 800ms) |
|--------|-------------|----------------------|
| **A (GPIO0)** | Select / Confirm / Back | Save & quit (Settings) |
| **B (GPIO14)** | Navigate / Change option | — |

---

## File Structure

```
TitanOs-Firmware-TS3/
├── Titan_Firmware_TS3.ino   # Entry point — setup(), loop(), auto-sleep
├── config.h / config.cpp    # Pins, constants, color themes, globals
├── utils.h / utils.cpp      # Buttons, ADC, backlight, NVS, deep sleep
├── ui.h / ui.cpp            # All graphical components (sprite double-buffer)
├── menu.h / menu.cpp        # Main menu navigation
├── screens.h / screens.cpp  # Info, Settings, Battery, Power Off screens
├── attacks.h / attacks.cpp  # WiFi Deauth, Evil Portal, BLE stub
└── games.h / games.cpp      # Snake game
```

---

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| Black screen | Check `Setup206_LilyGo_T_Display_S3.h` is active in TFT_eSPI |
| USB port not found | Hold BOOT + press RST to enter download mode |
| Wrong battery reading | Check the ÷2 voltage divider on GPIO4 |
| Settings not saved | Use long press on A inside Settings |

---

## Changelog

See [CHANGELOG.md](CHANGELOG.md)

---

## License

MIT — © 2026 [Titan-CyberTech](https://github.com/Titan-CyberTech)
