# ILA AUTO WIFI KILLER

> Autonomous ESP8266 Wi-Fi scanning and test-automation firmware for authorized wireless security labs.

![Version](https://img.shields.io/badge/version-v1.0-blue)
![Platform](https://img.shields.io/badge/platform-ESP8266-1f6feb)
![License](https://img.shields.io/badge/license-MIT-green)

## 🚀 Why This Project
`ILA AUTO WIFI KILLER` is a focused embedded security engineering project that automates a full RF cycle on ESP8266:

1. Scan nearby Wi-Fi environments
2. Select discovered targets automatically
3. Execute timed attack cycle
4. Repeat continuously

This repository is designed to be:
- practical for firmware researchers
- readable for collaborators
- strong enough for portfolio/resume presentation

## ⚠️ Legal & Ethical Use
This project must be used only in **authorized** environments:
- your own lab hardware
- written-permission security tests
- isolated education/research setups

Do **not** use on networks or devices without explicit authorization.

## 🧠 Core Behavior (v1.0)
- Scan phase: **15 seconds**
- Attack phase: **30 minutes**
- Loop: `scan -> attack -> scan`

LED behavior:
- Scanning: blinking
- Pre-attack: solid ON (brief)
- Attack: OFF
- Attack-start indicator: one-time count pattern (blink count = number of selected targets)

## 🏗️ Repository Layout
- `esp8266_deauther/` -> firmware source
- `build_auto.sh` -> reproducible local build script
- `docs/` -> architecture, deep guide, release docs, resume value
- `utils/` -> conversion and helper tooling
- `web_interface/` -> web UI assets from base project

## 🔧 Build
Requirements:
- `arduino-cli`
- `python3`
- `rg` (ripgrep)

Build command:
```bash
./build_auto.sh
```

Output:
- `build/esp8266_deauther.ino.bin`

## 📦 Release Policy
- Source code is versioned in git.
- Compiled firmware `.bin` is provided as a **GitHub Release asset** per tag.
- `v1.0` release includes one-click downloadable binary.

## 🙌 Inspiration & Credits
Attack engine and upstream ecosystem inspiration from SpacehuhnTech:
- [https://github.com/SpacehuhnTech](https://github.com/SpacehuhnTech)

## 📚 Documentation
- [Complete Technical Guide](docs/COMPLETE_TECHNICAL_GUIDE.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Release v1.0 Notes](docs/RELEASE_v1.0.md)
- [Resume / Portfolio Value](docs/RESUME_VALUE.md)
- [Contributing](CONTRIBUTING.md)
- [Security Policy](SECURITY.md)

