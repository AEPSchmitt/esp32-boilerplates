void setup() {
    delay(200);
    Serial.begin(115200);
    Serial.println("Serial echo ready. Type something!");
}

void loop() {
    if (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        line.trim();
        if (line.length() > 0) {
            Serial.print("You said: ");
            Serial.println(line);
        }
    }
}
