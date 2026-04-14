#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0" // change this
#define CHARACTERISTIC_UUID "abcdef01-1234-5678-1234-56789abcdef0" // change this

BLECharacteristic *pCharacteristic;

class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = std::string(pCharacteristic->getValue().c_str());
        if (value.length() > 0) {
            Serial.print("Received: ");
            Serial.println(value.c_str());
        }
    }
};

void setup() {
    Serial.begin(115200);
    BLEDevice::deinit(true);
    BLEDevice::init("ESP-nis");

    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);

    pCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID,
                        BLECharacteristic::PROPERTY_READ |
                        BLECharacteristic::PROPERTY_WRITE
                     );
    pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->setValue("Hello from ESP32-C6");
    
    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();
    Serial.println("BLE Server is running...");
}

void loop() {
    delay(1000);
}
