//  Requires: Adafruit NeoPixel library
//   Arduino IDE: Sketch > Include Library > Manage Libraries > "Adafruit NeoPixel"
#include <Adafruit_NeoPixel.h>

#define LED_PIN    8    // GPIO8 = onboard led on ESP32C6 DevKit
#define NUM_LEDS   1
#define BRIGHTNESS 180  // 0–255
#define STEP_DELAY 50    // ms per hue step // lower = faster cycle

Adafruit_NeoPixel led(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

uint16_t hue = 0;  // 0–65535

void setup() {
    delay(200);
    led.begin();
    led.setBrightness(BRIGHTNESS);
    led.show();
    Serial.begin(115200);
    Serial.println("Rainbow cycle starting.");
}

void loop() {
    led.setPixelColor(0, led.ColorHSV(hue, 255, 255));
    led.show();

    hue += 256;
    delay(STEP_DELAY);
}
