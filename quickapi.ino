#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ssid";
const char* password = "password";

WebServer server(80);

void connect_wifi(){
    Serial.println("\nConnecting to Wi-Fi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      digitalWrite(LED_BUILTIN, LOW);
      delay(1000);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(1000);
    }
    Serial.println("\nWi-Fi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void say_hi() {
  Serial.println("Endpoint Accessed");
  server.send(200, "application/json", "{}");
}

void light_on(){
  digitalWrite(LED_BUILTIN, HIGH);
  server.send(200, "application/json", "{'status' : 'success','state':'on'}");
}

void light_off(){
  digitalWrite(LED_BUILTIN, LOW);
  server.send(200, "application/json", "{'status' : 'success','state':'off'}");
}

void setup_api() {
  server.on("/hello", say_hi);
  server.on("/on", light_on);
  server.on("/off", light_off);
  //server.on("/setStatus", HTTP_POST, handlePost);
  server.begin();
}


void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
    Serial.print("READY FOR TAKEOFF!");
    connect_wifi();
    setup_api();
}

void loop() {
  server.handleClient();
}
