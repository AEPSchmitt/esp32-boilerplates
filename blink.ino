// Blink onboard LED

void setup() {
    delay(200);  
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
    Serial.println("READY FOR TAKEOFF!");
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(2000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(2000);
}