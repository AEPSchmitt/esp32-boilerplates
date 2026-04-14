// Serves a small webpage at http://<device-ip>/
// Includes a /toggle endpoint to flip the onboard LED on/off

#include <WiFi.h>
#include <WebServer.h>

#define WIFI_SSID     "ssid"
#define WIFI_PASSWORD "password"

WebServer server(80);
bool ledOn = false;

void handleRoot() {
    String html =
        "<!DOCTYPE html><html><head>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>ESP32C6</title></head><body>"
        "<h2>ESP32C6 Web Server</h2>"
        "<p>LED is <strong>" + String(ledOn ? "ON" : "OFF") + "</strong></p>"
        "<form action='/toggle' method='POST'>"
        "<button type='submit'>Toggle LED</button>"
        "</form>"
        "</body></html>";
    server.send(200, "text/html", html);
}

void handleToggle() {
    ledOn = !ledOn;
    digitalWrite(LED_BUILTIN, ledOn ? HIGH : LOW);
    server.sendHeader("Location", "/");
    server.send(303);  // redirect back to root
}

void handleNotFound() {
    server.send(404, "text/plain", "Not found");
}

void setup() {
    delay(200);
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected!");
    Serial.print("Open http://");
    Serial.println(WiFi.localIP());

    server.on("/",        HTTP_GET,  handleRoot);
    server.on("/toggle",  HTTP_POST, handleToggle);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP server started.");
}

void loop() {
    server.handleClient();
}
