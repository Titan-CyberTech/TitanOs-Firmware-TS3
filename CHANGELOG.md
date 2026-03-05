# Changelog

All notable changes to TitanOs-Firmware-TS3 are documented here.

---

## [2.0.0] — 2026

### Added
- **Double-buffered rendering** via `TFT_eSprite` — zero flicker on all screens
- **Persistent settings** via ESP32 NVS (`Preferences`) — theme and brightness survive reboot
- **5 color themes** : Violet, Cyan, Orange, Green, Red
- **Topbar / Botbar** UI chrome on every screen with contextual button hints
- **Battery icon** in topbar — color-coded (green / yellow / red) with % inside the icon
- **Animated splash screen** with loading bar
- **System Info screen** — responsive 2×4 card grid layout
- **Battery screen** — large gauge + voltage in mV
- **Deauth attack** — paginated network list with RSSI & channel, live packet counter, pulsing animation
- **Evil Portal** — real-time credential display, blink indicator, capture counter
- **Snake** — visible grid, in-topbar score, progressive speed, 3-2-1 countdown, flashing game-over
- **Auto screen-off** after 20 seconds of inactivity (wake on any button)
- **Fade-out animation** before deep sleep (Power Off)
- `CHANGELOG.md` and `.gitignore` added to repository

### Changed
- Backlight pin corrected to **GPIO38** (was GPIO15 in v1)
- Battery ADC uses `esp_adc_cal` with 8-sample averaging for accuracy
- Settings screen: short A = switch section, long A = save & quit, B = change value
- `utils_waitBtn()` now fully purges button states on exit — fixes re-trigger crash
- Code split into focused modules: `ui`, `utils`, `screens`, `menu`, `attacks`, `games`
- All header guards replaced with `#pragma once`

### Fixed
- **Battery screen crash** — button state not cleared after `waitBtn()` caused immediate re-entry → stack overflow
- **Brightness not changing** — debounce delay too long (200ms) was swallowing button presses; reduced to 150ms with direct GPIO read

### Removed
- Monolithic single-file structure from v1
- `display.cpp/h` merged into `ui.cpp/h` and `screens.cpp/h`

---

## [1.1.6] — 2025

- Initial public release
- Basic menu, WiFi Deauth, Evil Portal, Snake, battery display, settings
