/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 */

#include "BLEDevice.h"
//#include "BLEScan.h"

// The remote service we wish to connect to.
static BLEUUID serviceUUID("4cf21db2-ec77-43ca-a480-435e1740f6a6");
// The characteristic of the remote service we are interested in.
static BLEUUID    temperatureUUID("c1fa9d3b-3141-421a-8762-840838983d6c");
static BLEUUID    humiditeUUID("fde5570a-729d-4d6a-bd40-3e796395f773");
static BLEUUID    pressionUUID("ab1704fd-6de2-41b5-a12b-b5ed9690093b");
static BLEUUID    ensoleillementUUID("5268707b-6a5a-4021-b066-9150177eda25");
static BLEUUID    pluieUUID("552e2c36-b34a-4f7e-96a2-3da2fe8871e2");
static BLEUUID    directionVentUUID("21f12340-4f1b-4542-8544-df09263feb24");
static BLEUUID    vitesseVentUUID("4dc6dec7-f957-4be9-9935-9923481d40c2");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteTemperature;
static BLERemoteCharacteristic* pRemoteHumidite;
static BLERemoteCharacteristic* pRemotePression;
static BLERemoteCharacteristic* pRemoteEnsoleillement;
static BLERemoteCharacteristic* pRemotePluie;
static BLERemoteCharacteristic* pRemoteDirectionVent;
static BLERemoteCharacteristic* pRemoteVitesseVent;
static BLEAdvertisedDevice* myDevice;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.write(pData, length);
    Serial.println();
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");
    pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)
  
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteTemperature = pRemoteService->getCharacteristic(temperatureUUID);
    if (pRemoteTemperature == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(temperatureUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    pRemoteHumidite = pRemoteService->getCharacteristic(humiditeUUID);
    if (pRemoteHumidite == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(humiditeUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    pRemotePression = pRemoteService->getCharacteristic(pressionUUID);
    if (pRemotePression == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(pressionUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    pRemoteEnsoleillement = pRemoteService->getCharacteristic(ensoleillementUUID);
    if (pRemoteEnsoleillement == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(ensoleillementUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    pRemotePluie = pRemoteService->getCharacteristic(pluieUUID);
    if (pRemotePluie == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(pluieUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    pRemoteDirectionVent = pRemoteService->getCharacteristic(directionVentUUID);
    if (pRemoteDirectionVent == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(directionVentUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    pRemoteVitesseVent = pRemoteService->getCharacteristic(vitesseVentUUID);
    if (pRemoteVitesseVent == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(vitesseVentUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteTemperature->canRead()) {
      std::string value = pRemoteTemperature->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }
    if(pRemoteHumidite->canRead()) {
      std::string value = pRemoteHumidite->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }
    if(pRemotePression->canRead()) {
      std::string value = pRemotePression->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }
    if(pRemoteEnsoleillement->canRead()) {
      std::string value = pRemoteEnsoleillement->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }
    if(pRemotePluie->canRead()) {
      std::string value = pRemotePluie->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }
    if(pRemoteDirectionVent->canRead()) {
      std::string value = pRemoteDirectionVent->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }
    if(pRemoteVitesseVent->canRead()) {
      std::string value = pRemoteVitesseVent->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteTemperature->canNotify())
      pRemoteTemperature->registerForNotify(notifyCallback);
    if(pRemoteHumidite->canNotify())
      pRemoteHumidite->registerForNotify(notifyCallback);
    if(pRemotePression->canNotify())
      pRemotePression->registerForNotify(notifyCallback);
    if(pRemoteEnsoleillement->canNotify())
      pRemoteEnsoleillement->registerForNotify(notifyCallback);
    if(pRemotePluie->canNotify())
      pRemotePluie->registerForNotify(notifyCallback);
    if(pRemoteDirectionVent->canNotify())
      pRemoteDirectionVent->registerForNotify(notifyCallback);
    if(pRemoteVitesseVent->canNotify())
      pRemoteVitesseVent->registerForNotify(notifyCallback);
    

    connected = true;
    return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
} // End of setup.


// This is the Arduino main loop function.
void loop() {

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    String newValue = "Time since boot: " + String(millis()/1000);
    Serial.println("Setting new characteristic value to \"" + newValue + "\"");
    
    // Set the characteristic's value to be the array of bytes that is actually a string.
    pRemoteTemperature->writeValue(newValue.c_str(), newValue.length());
    pRemoteHumidite->writeValue(newValue.c_str(), newValue.length());
    pRemotePression->writeValue(newValue.c_str(), newValue.length());
    pRemoteEnsoleillement->writeValue(newValue.c_str(), newValue.length());
    pRemotePluie->writeValue(newValue.c_str(), newValue.length());
    pRemoteDirectionVent->writeValue(newValue.c_str(), newValue.length());
    pRemoteVitesseVent->writeValue(newValue.c_str(), newValue.length());
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
  }
  
  delay(1000); // Delay a second between loops.
} // End of loop
