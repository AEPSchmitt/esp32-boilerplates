// How to persist values across reboots using NVS
// Preferences wraps the ESP-IDF NVS (Non-Volatile Storage) flash partition

#include <Preferences.h>

Preferences prefs;

void setup() {
    delay(200);
    Serial.begin(115200);

    // Open namespace "app" in read-write mode
    prefs.begin("app", false);

    int bootCount = prefs.getInt("boots", 0);
    bootCount++;
    prefs.putInt("boots", bootCount);

    Serial.printf("Boot number: %d\n", bootCount);

    // Store and read a string
    if (!prefs.isKey("name")) {
        prefs.putString("name", "ESP32C6");
    }
    Serial.printf("Device name: %s\n", prefs.getString("name").c_str());

    prefs.end();
}

void loop() {
    // Nothing to do after setup
    delay(10000); // except chill loop
}
