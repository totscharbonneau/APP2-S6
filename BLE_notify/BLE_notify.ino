/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updated by chegewara

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
   And has a characteristic of: beb5483e-36e1-4688-b7f5-ea07361b26a8

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   A connect hander associated with the server starts a background task that performs notification
   every couple of seconds.
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pTemperature = NULL;
BLECharacteristic* pHumidite = NULL;
BLECharacteristic* pPression = NULL;
BLECharacteristic* pEnsoleillement = NULL;
BLECharacteristic* pPluie = NULL;
BLECharacteristic* pDirectionVent = NULL;
BLECharacteristic* pVitesseVent = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

static BLEUUID   SERVICE_UUID("4cf21db2-ec77-43ca-a480-435e1740f6a6");
static BLEUUID   TEMPERATURE_UUID("c1fa9d3b-3141-421a-8762-840838983d6c");
static BLEUUID   HUMIDITE_UUID("fde5570a-729d-4d6a-bd40-3e796395f773");
static BLEUUID   PRESSION_UUID("ab1704fd-6de2-41b5-a12b-b5ed9690093b");
static BLEUUID   ENSOLEILLEMENT_UUID("5268707b-6a5a-4021-b066-9150177eda25");
static BLEUUID   PLUIE_UUID("552e2c36-b34a-4f7e-96a2-3da2fe8871e2");
static BLEUUID   DIRECTION_VENT_UUID("21f12340-4f1b-4542-8544-df09263feb24");
static BLEUUID   VITESSE_VENT_UUID("4dc6dec7-f957-4be9-9935-9923481d40c2");


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};



void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID, 21);

  // Create a BLE Characteristic
  pTemperature = pService->createCharacteristic(
                      TEMPERATURE_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  pHumidite = pService->createCharacteristic(
                      HUMIDITE_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  pPression = pService->createCharacteristic(
                    PRESSION_UUID,
                    BLECharacteristic::PROPERTY_READ   |
                    BLECharacteristic::PROPERTY_WRITE  |
                    BLECharacteristic::PROPERTY_NOTIFY |
                    BLECharacteristic::PROPERTY_INDICATE
                  );
  pEnsoleillement = pService->createCharacteristic(
                    ENSOLEILLEMENT_UUID,
                    BLECharacteristic::PROPERTY_READ   |
                    BLECharacteristic::PROPERTY_WRITE  |
                    BLECharacteristic::PROPERTY_NOTIFY |
                    BLECharacteristic::PROPERTY_INDICATE
                  );
  pPluie = pService->createCharacteristic(
                      PLUIE_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  pDirectionVent = pService->createCharacteristic(
                      DIRECTION_VENT_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  pVitesseVent = pService->createCharacteristic(
                      VITESSE_VENT_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );                                        

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pTemperature->addDescriptor(new BLE2902());
  pHumidite->addDescriptor(new BLE2902());
  pPression->addDescriptor(new BLE2902());
  pEnsoleillement->addDescriptor(new BLE2902());
  pPluie->addDescriptor(new BLE2902());
  pDirectionVent->addDescriptor(new BLE2902());
  pVitesseVent->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
    // notify changed value
    if (deviceConnected) {
        pTemperature->setValue((uint8_t*)&value, 4);
        pHumidite->setValue((uint8_t*)&value, 4);
        pPression->setValue((uint8_t*)&value, 4);
        pEnsoleillement->setValue((uint8_t*)&value, 4);
        pPluie->setValue((uint8_t*)&value, 4);
        pDirectionVent->setValue((uint8_t*)&value, 4);
        pVitesseVent->setValue((uint8_t*)&value, 4);
        
        pTemperature->notify();
        pHumidite->notify();
        pPression->notify();
        pEnsoleillement->notify();
        pPluie->notify();
        pDirectionVent->notify();
        pVitesseVent->notify();
        value++;
        delay(3); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}
