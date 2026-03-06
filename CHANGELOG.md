# Changelog

All notable changes to TitanOs-Firmware-TS3 are documented here.

---

## [2.0.1] — 2026

### Added
- **Beacon Flood attack** — broadcasts hundreds of fake WiFi networks across all 13 channels simultaneously, visible on every nearby device. SSIDs generated from 15 realistic prefixes with random suffixes. MACs fully randomized. Channel hops every 10 frames to cover the full 2.4GHz spectrum.
- **Signal strength bars** in the Deauth network list — 4-bar indicator (replaces raw dBm text), much easier to read at a glance
- **Network counter** displayed below the Deauth network list ("X networks found")
- **Packet progress bar** in Deauth attack screen — visual fill bar alongside the packet counter

### Changed
- WiFi submenu now has **3 options** : Deauth Attack · Beacon Flood · Evil Portal
- SSID truncation in network list reduced to 16 chars (was 18) to make room for signal bars
- `SSID_PREFIXES` stored in `PROGMEM` to save RAM
- `F()` macro used on static strings in Evil Portal handler
- Version bumped to **2.0.1**

### Notes
- Beacon Flood is for **educational use only** — broadcasting fake networks is illegal on networks/spectrum you don't own
- BLE Flood still in development, coming in v2.1

---

## [2.0.0] — 2026

### Added
- Double-buffered rendering via TFT_eSprite — zero flicker
- Persistent settings via ESP32 NVS (theme + brightness survive reboot)
- 5 color themes : Violet, Cyan, Orange, Green, Red
- Topbar / Botbar UI on every screen with contextual button hints
- Battery icon in topbar — color-coded with % inside
- Animated splash screen with loading bar
- System Info — responsive 2×4 card grid layout
- Snake — score, progressive speed, countdown, flashing game-over
- Evil Portal — real-time credential display, capture counter

### Fixed
- Battery screen crash — button state not cleared caused stack overflow on return
- Brightness not changing — 200ms debounce was swallowing button presses
- Backlight pin corrected to GPIO38 (was GPIO15 in v1)

---

## [1.1.6] — 2025

- Last stable v1 release
- Basic menu, WiFi Deauth, Evil Portal, Snake, battery display, settings

## [1.1.5] — 2025

- Improved WiFi scan reliability
- Fixed brightness step overflow bug
- Added deep sleep wake on button B

## [1.1.4] — 2025

- Initial Evil Portal implementation

## [1.1.3] — 2025

- Snake game added

## [1.1.2] — 2025

- WiFi Deauth attack added

## [1.1.1] — 2025

- Battery voltage display

## [1.1.0] — 2025

- First public release
