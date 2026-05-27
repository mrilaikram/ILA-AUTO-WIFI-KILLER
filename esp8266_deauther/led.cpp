/* This software is licensed under the MIT License: https://github.com/spacehuhntech/esp8266_deauther */

#include "led.h"

#include "A_config.h" // Config for LEDs
#include <Arduino.h>  // digitalWrite, analogWrite, pinMode
#include "language.h" // Strings used in printColor and tempDisable
#include "settings.h" // used in update()
#include "Attack.h"   // used in update()
#include "Scan.h"     // used in update()

// Inlcude libraries for Neopixel or LED_MY92xx if used
#if defined(LED_NEOPIXEL)
#include "src/Adafruit_NeoPixel-1.7.0/Adafruit_NeoPixel.h"
#elif defined(LED_MY92)
#include "src/my92xx-3.0.3/my92xx.h"
#elif defined(LED_DOTSTAR)
#include "src/Adafruit_DotStar-1.1.4/Adafruit_DotStar.h"
#endif // if defined(LED_NEOPIXEL)

extern Attack attack;
extern Scan   scan;

namespace led {
    // ===== PRIVATE ===== //
    LED_MODE mode = OFF;
    uint32_t scanBlinkTime = 0;
    bool     scanBlinkOn   = false;

    // One-time attack-start LED indication sequence.
    bool     attackIndicatorActive      = false;
    bool     attackIndicatorPulseOn     = false;
    uint16_t attackIndicatorTargetCount = 0;
    uint16_t attackIndicatorBlinkCount  = 0;
    uint32_t attackIndicatorStartTime   = 0;
    uint32_t attackIndicatorLastBlink   = 0;
    uint32_t attackIndicatorPulseTime   = 0;

    const uint32_t ATTACK_INDICATOR_DELAY_MS    = 3000;
    const uint32_t ATTACK_INDICATOR_INTERVAL_MS = 2000;
    const uint32_t ATTACK_INDICATOR_PULSE_MS    = 180;

#if defined(LED_NEOPIXEL_RGB)
    Adafruit_NeoPixel strip { LED_NUM, LED_NEOPIXEL_PIN, NEO_RGB + NEO_KHZ800 };
#elif defined(LED_NEOPIXEL_GRB)
    Adafruit_NeoPixel strip { LED_NUM, LED_NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800 };
#elif defined(LED_MY92)
    my92xx myled { LED_MY92_MODEL, LED_NUM, LED_MY92_DATA, LED_MY92_CLK, MY92XX_COMMAND_DEFAULT };
#elif defined(LED_DOTSTAR)
    Adafruit_DotStar strip { LED_NUM, LED_DOTSTAR_DATA, LED_DOTSTAR_CLK, DOTSTAR_BGR };
#endif // if defined(LED_NEOPIXEL_RGB)


    void setColor(uint8_t r, uint8_t g, uint8_t b) {
#if defined(LED_DIGITAL)
        if (LED_ANODE) {
            if (LED_PIN_R < 255) digitalWrite(LED_PIN_R, r > 0);
            if (LED_PIN_G < 255) digitalWrite(LED_PIN_G, g > 0);
            if (LED_PIN_B < 255) digitalWrite(LED_PIN_B, b > 0);
        } else {
            if (LED_PIN_R < 255) digitalWrite(LED_PIN_R, r == 0);
            if (LED_PIN_G < 255) digitalWrite(LED_PIN_G, g == 0);
            if (LED_PIN_B < 255) digitalWrite(LED_PIN_B, b == 0);
        }
#elif defined(LED_RGB)
        if (r > 0) r = r * LED_MODE_BRIGHTNESS / 100;
        if (g > 0) g = g * LED_MODE_BRIGHTNESS / 100;
        if (b > 0) b = b * LED_MODE_BRIGHTNESS / 100;

        if (LED_ANODE) {
            r = 255 - r;
            g = 255 - g;
            b = 255 - b;
        }

        analogWrite(LED_PIN_R, r);
        analogWrite(LED_PIN_G, g);
        analogWrite(LED_PIN_B, b);
#elif defined(LED_NEOPIXEL) || defined(LED_DOTSTAR)

        for (size_t i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, r, g, b);
        }

        strip.show();
#elif defined(LED_MY9291)
        myled.setChannel(LED_MY92_CH_R, r);
        myled.setChannel(LED_MY92_CH_G, g);
        myled.setChannel(LED_MY92_CH_B, b);
        myled.setChannel(LED_MY92_CH_BRIGHTNESS, LED_MODE_BRIGHTNESS);
        myled.setState(true);
        myled.update();
#endif // if defined(LED_DIGITAL)
    }

    // ===== PUBLIC ===== //
    void setup() {
        analogWriteRange(0xff);

#if defined(LED_DIGITAL) || defined(LED_RGB)
        if (LED_PIN_R < 255) pinMode(LED_PIN_R, OUTPUT);
        if (LED_PIN_G < 255) pinMode(LED_PIN_G, OUTPUT);
        if (LED_PIN_B < 255) pinMode(LED_PIN_B, OUTPUT);
#elif defined(LED_NEOPIXEL) || defined(LED_DOTSTAR)
        strip.begin();
        strip.setBrightness(LED_MODE_BRIGHTNESS);
        strip.show();
#elif defined(LED_MY9291)
        myled.setChannel(LED_MY92_CH_R, 0);
        myled.setChannel(LED_MY92_CH_G, 0);
        myled.setChannel(LED_MY92_CH_B, 0);
        myled.setChannel(LED_MY92_CH_BRIGHTNESS, LED_MODE_BRIGHTNESS);
        myled.setState(true);
        myled.update();
#endif // if defined(LED_DIGITAL) || defined(LED_RGB)
    }

    void update() {
        if (!settings::getLEDSettings().enabled) {
            scanBlinkOn = false;
            attackIndicatorActive = false;
            attackIndicatorPulseOn = false;
            setMode(OFF);
        } else if (attack.isRunning()) {
            scanBlinkOn = false;
            setMode(ATTACK);

            if (attackIndicatorActive) {
                uint32_t now = millis();

                // Wait before starting the count indication.
                if (now - attackIndicatorStartTime >= ATTACK_INDICATOR_DELAY_MS) {
                    if (attackIndicatorPulseOn) {
                        // End a short blink pulse and return to OFF.
                        if (now - attackIndicatorPulseTime >= ATTACK_INDICATOR_PULSE_MS) {
                            attackIndicatorPulseOn = false;
                            setColor(LED_MODE_OFF);
                        }
                    } else if (attackIndicatorBlinkCount < attackIndicatorTargetCount) {
                        // Trigger next pulse every 2 seconds.
                        if ((attackIndicatorLastBlink == 0) ||
                            (now - attackIndicatorLastBlink >= ATTACK_INDICATOR_INTERVAL_MS)) {
                            attackIndicatorLastBlink = now;
                            attackIndicatorPulseTime = now;
                            attackIndicatorPulseOn   = true;
                            attackIndicatorBlinkCount++;
                            setColor(LED_MODE_IDLE);
                        }
                    } else {
                        // Sequence complete for this attack cycle.
                        attackIndicatorActive = false;
                        setColor(LED_MODE_OFF);
                    }
                }
            }
        } else if (scan.isScanning() && (scan.deauths < settings::getSnifferSettings().min_deauth_frames)) {
            attackIndicatorActive = false;
            attackIndicatorPulseOn = false;
            setMode(SCAN);

            // Blink while scanning: alternate SCAN color and OFF every 300ms.
            uint32_t now = millis();
            if (now - scanBlinkTime >= 300) {
                scanBlinkTime = now;
                scanBlinkOn   = !scanBlinkOn;

                if (scanBlinkOn) setColor(LED_MODE_SCAN);
                else setColor(LED_MODE_OFF);
            }
        } else {
            scanBlinkOn = false;
            attackIndicatorActive = false;
            attackIndicatorPulseOn = false;
            setMode(IDLE);
        }
    }

    void setMode(LED_MODE new_mode, bool force) {
        if ((new_mode != mode) || force) {
            mode = new_mode;

            switch (mode) {
                case OFF:
                    setColor(LED_MODE_OFF);
                    break;
                case SCAN:
                    setColor(LED_MODE_SCAN);
                    break;
                case ATTACK:
                    // Keep LED off during active attack.
                    setColor(LED_MODE_OFF);
                    break;
                case IDLE:
                    setColor(LED_MODE_IDLE);
                    break;
            }
        }
    }

    void startAttackIndicator(uint16_t targetCount) {
        attackIndicatorTargetCount = targetCount;
        attackIndicatorBlinkCount  = 0;
        attackIndicatorStartTime   = millis();
        attackIndicatorLastBlink   = 0;
        attackIndicatorPulseTime   = 0;
        attackIndicatorPulseOn     = false;
        attackIndicatorActive      = targetCount > 0;
    }
}
