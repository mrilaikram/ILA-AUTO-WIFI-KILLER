# Architecture

## High-Level Design
`ILA AUTO WIFI KILLER` is a state-machine-driven ESP8266 firmware with a minimal runtime loop:

- `SCAN` state
- `ATTACK` state
- transition timers and target-selection bridge

## Orchestrator
Main loop in `esp8266_deauther/esp8266_deauther.ino` coordinates:
- `led::update()`
- `wifi::update()`
- `attack.update()`
- `scan.update()`

## Data Model
Global managers:
- `Accesspoints`
- `Stations`
- `SSIDs`
- `Names`

These feed both scan and attack logic.

## Timing and Determinism
The design uses cooperative scheduling (`millis()` checks) instead of preemptive tasks.
This keeps behavior deterministic enough for embedded test cycles.

## Extension Points
- dynamic target filtering
- adaptive scan intervals
- watchdog and telemetry export
- safer attack gating rules for controlled labs

