#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

if ! command -v arduino-cli >/dev/null 2>&1; then
  echo "Error: arduino-cli is not installed or not in PATH."
  echo "Install: https://arduino.github.io/arduino-cli/latest/installation/"
  exit 1
fi

CLI_CONFIG="$SCRIPT_DIR/.arduino-cli.local.yaml"

cat > "$CLI_CONFIG" <<CFG
board_manager:
  additional_urls:
    - file://$SCRIPT_DIR/package_esp8266com_index.json
    - file://$SCRIPT_DIR/package_spacehuhn_index.json
CFG

echo "[1/4] Updating package index..."
arduino-cli core update-index --config-file "$CLI_CONFIG"

echo "[2/4] Ensuring Deauther ESP8266 core is installed..."
if ! arduino-cli core list --config-file "$CLI_CONFIG" | rg -q '^deauther:esp8266\s'; then
  arduino-cli core install deauther:esp8266 --config-file "$CLI_CONFIG"
fi

echo "[3/4] Preparing web assets..."
(
  cd utils/web_converter
  python3 webConverter.py
)

echo "[4/4] Building firmware..."
mkdir -p build
arduino-cli compile \
  --fqbn deauther:esp8266:generic \
  --build-property "build.extra_flags=-DESP8266 -DNODEMCU" \
  --output-dir build \
  --config-file "$CLI_CONFIG" \
  ./esp8266_deauther

echo "Build complete."
ls -lh build/*.bin
