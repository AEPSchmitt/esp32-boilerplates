// Minimal CO2 sensor reading example

HardwareSerial co2Serial(1);

#define RX_PIN 17
#define TX_PIN 16

void setup() {
    delay(200);  
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
    co2Serial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
    Serial.println("READY FOR TAKEOFF!");
}

void loop() {
    int co2ppm = readCO2();
    Serial.println(co2ppm);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(3000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(3000);
}

int readCO2() {
  byte cmd[9] = {0xFF, 0x01, 0x86, 0, 0, 0, 0, 0, 0x79};
  byte response[9];

  // Clear buffer
  while (co2Serial.available()) co2Serial.read();

  // Send command
  co2Serial.write(cmd, 9);

  delay(100);

  if (co2Serial.available() >= 9) {
    co2Serial.readBytes(response, 9);

    // Validate response
    if (response[0] == 0xFF && response[1] == 0x86) {
      int ppm = (response[2] << 8) | response[3];
      return ppm;
    }
  }

  return -1; // error
}