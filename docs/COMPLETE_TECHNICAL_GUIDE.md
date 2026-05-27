# Complete Technical Guide - ILA AUTO WIFI KILLER

## 1) Runtime Flow (Step by Step)

On power-up, firmware does:
1. Start serial at `115200`
2. Mount LittleFS
3. Load settings from EEPROM + JSON mirror
4. Initialize Wi-Fi subsystem
5. Attach promiscuous packet callback
6. Initialize LED subsystem
7. Start scan cycle (15s)

Then it loops:
1. Scan AP/station landscape
2. Select all discovered targets
3. Enter pre-attack LED stage (solid ON)
4. Start attack for 30 minutes
5. Return to scan when timeout is reached

## 2) Timing Constants
Defined in `esp8266_deauther/esp8266_deauther.ino`:
- `AUTO_SCAN_TIME = 15000 ms`
- `AUTO_ATTACK_TIME = 1800000 ms`
- `PRE_ATTACK_GLOW_TIME = 1500 ms`

## 3) LED State Logic

During scan:
- LED blinks continuously (software timed)

Before attack:
- LED forced solid ON for brief visual transition

During attack:
- LED remains OFF

Attack-start count indicator:
- Wait 3 seconds after attack begins
- Blink once per selected target (`AP + Station`)
- 2 second gap between blinks
- Runs once per 30-minute attack cycle

## 4) Main Modules

### `Attack`
- Packet scheduling
- Attack mode state
- Counters/rates and timeout

### `Scan`
- AP discovery
- Sniffer processing
- Station relationship detection

### `wifi`
- Mode and channel handling
- AP/web server features from base code (disabled in this config)

### `settings`
- Persistent config load/save
- Defaults and validation

## 5) Data and Storage
- `/settings.json` mirror for settings visibility
- `/scan.json` for scan output snapshots

## 6) Build System
Use:
```bash
./build_auto.sh
```

Build script behavior:
1. Generates local `arduino-cli` config
2. Updates indexes
3. Ensures `deauther:esp8266` core
4. Regenerates web assets
5. Builds firmware binary

## 7) Troubleshooting

### No target activity
- Check antenna and RF environment
- Check that scan returns AP/station records
- Verify serial logs and timing transitions

### LED appears inactive
- Verify board LED pin compatibility
- Verify settings LED flag is enabled
- Verify current mode (attack intentionally keeps LED off)

### Build fails
- Confirm `arduino-cli`, `python3`, `rg` installed
- Re-run build script from repository root

