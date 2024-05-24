/*
Mis à jour par Charles-Étienne Côté (cotc1105) et Thomas Charbonneau (chat1002)

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
#include <Wire.h>

static bool uart = false;


BLEServer* pServer = NULL;
BLECharacteristic* pTemperature = NULL;
int pasttemp = NULL;
BLECharacteristic* pHumidite = NULL;
float pastHumidity = NULL;
BLECharacteristic* pPression = NULL;
int pastpression = NULL;
BLECharacteristic* pEnsoleillement = NULL;
int pasEnsoleillement = NULL;
BLECharacteristic* pPluie = NULL;
bool pastpluie = NULL;
BLECharacteristic* pDirectionVent = NULL;
char pastDirectionVent = NULL;
BLECharacteristic* pVitesseVent = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

int AN_directionVENT = 35;
int AN_lumiere_in = 34;


int scl = 22;
int sda = 21;
int adress = 0x77; 
int reg = 0x0D;
uint32_t scalingFactor = 253952;
float waterlvl = 0;


struct uint_coefficients {
  uint32_t c0;
  uint32_t c1;
  uint32_t c00;
  uint32_t c01;
  uint32_t c10;
  uint32_t c20;
  uint32_t c11;
  uint32_t c21;
  uint32_t c30;
};

struct coefficients {
  int32_t c0;
  int32_t c1;
  int32_t c00;
  int32_t c01;
  int32_t c10;
  int32_t c20;
  int32_t c11;
  int32_t c21;
  int32_t c30;
};

coefficients data_coeff;

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
  Wire.begin();
  pinMode(AN_lumiere_in, INPUT);
  pinMode(AN_directionVENT, INPUT);
  pinMode(32, INPUT);

  uint_coefficients uint_data;
  uint_data = getCoefficient();
  data_coeff.c0 = utoi(uint_data.c0, 12);
  data_coeff.c1 = utoi(uint_data.c1, 12);
  data_coeff.c00 = utoi(uint_data.c00, 20);
  data_coeff.c10 = utoi(uint_data.c10, 20);
  data_coeff.c20 = utoi(uint_data.c20, 16);
  data_coeff.c30 = utoi(uint_data.c30, 16);
  data_coeff.c01 = utoi(uint_data.c01, 16);
  data_coeff.c11 = utoi(uint_data.c11, 16);
  data_coeff.c21 = utoi(uint_data.c21, 16);

  configPressureTemp();

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID, (uint32_t)50, 0);

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
      char temp_dir = direction(); 
      
      int temp_temp = read_Temperature();
      char temp_str[2];
      itoa(temp_temp,temp_str,10);
      float temp_pression = read_Pressure();

      // Serial.println(temp_pression);
      char pression_str[10];
      sprintf(pression_str, "%-9g",temp_pression);
      // Serial.println(pression_str);

      // itoa(temp_pression,pression_str,10);
      uint8_t temp_lux = lumiere();
      char ensoleillement_str[3];
      itoa(temp_lux,ensoleillement_str,10);

      float temp_humidity = humidity_temp();
      char humidity_str[3];
      sprintf(humidity_str, "%g",temp_humidity);

      bool temp_pluie = pluie();
      char pluie_str[8];
      if (temp_pluie) {
        strcpy(pluie_str, "rain");
      } 
      else {
        strcpy(pluie_str, "no rain");
      }

      pTemperature->setValue((uint8_t*)&temp_str, 2);
      pHumidite->setValue((uint8_t*)&humidity_str, 3);
      pPression->setValue((uint8_t*)&pression_str, 10);
      pEnsoleillement->setValue((uint8_t*)&ensoleillement_str, 3);
      pPluie->setValue((uint8_t*)&pluie_str, 8);
      pDirectionVent->setValue((uint8_t*)&temp_dir, 1);
      pVitesseVent->setValue((uint8_t*)&value, 4);
        
      // pTemperature->notify();
      // pHumidite->notify();
      // pPression->notify();
      // pEnsoleillement->notify();
      // pPluie->notify();
      // pDirectionVent->notify();
      // pVitesseVent->notify();
      // pEnsoleillement->notify();
      if(pasttemp != temp_temp){
        pTemperature->notify();
      }
      if(pastpression != temp_pression){
        pPression->notify();
      }
      // if (pastDirectionVent != temp_dir){
      //   Serial.println("in if direction");
      //   pDirectionVent->notify();
      // }
      // pDirectionVent->notify();
      if (pasEnsoleillement != temp_lux){
        pEnsoleillement->notify();
      }
      if(pastHumidity != temp_humidity){
        pHumidite->notify();
      }
      
      // value++;
      pastDirectionVent = temp_dir;
      pasEnsoleillement = temp_lux;
      pasttemp = temp_temp;
      pastpression = temp_pression;
      pastHumidity = temp_humidity;
      pastpluie = temp_pluie;

      if(uart){
          char dataBuffer[100];
          sprintf(dataBuffer, "Temperature: %s, Humidity: %s, Pressure: %s, Light: %s, Rain: %s, Wind Direction: %c\n",temp_str, humidity_str, pression_str, ensoleillement_str, pluie_str, temp_dir);
           // Send the data over UART
          Serial.print(dataBuffer);
      }


      delay(1000); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
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


char direction(){
  int dirvalue = analogRead(AN_directionVENT);
  int value[] = {226,1005,3925,3050};
  char directiontable[] = {'N','E','S','W'};
  int smallestdelta = 1000;
  int dirint;
  for(int i = 0; i < 4; i++){
    int delta = abs(dirvalue -value[i]);
    if (delta < smallestdelta){
      smallestdelta = delta;
      dirint = i;
    }
  }
  // Serial.println(directiontable[dirint]);
  return directiontable[dirint];
}

int lumiere(){
  int lumval = analogRead(AN_lumiere_in);
  int lux = 0.0756f*lumval - 35;
  // Serial.println(lumval);
  // Serial.println(lux);
  return lux;
}

int read_reg(int regadress){
  Wire.beginTransmission(adress);
  Wire.write(regadress);
  bool reset = false;
  Wire.endTransmission(reset);
  Wire.requestFrom(adress, 1);
  char data = Wire.read();
  int data2 = data;
  return data2;
  // Serial.println(data2);
  // delay(1000);
}

void read_reg_incremental(int regadress, int length,char* dataArray){
  Wire.beginTransmission(adress);
  Wire.write(regadress);
  bool reset = false;
  Wire.endTransmission(reset);
  Wire.requestFrom(adress, length);
  for (int i = 0; i < length; i++){
    dataArray[i] = Wire.read();
  }
}

uint_coefficients getCoefficient(){
  int length = 15;
  char data[length];
  read_reg_incremental(0x13,length, data);
  uint_coefficients separated_data;
  // Serial.println("c0 :");
  // Serial.println(data[0], BIN);
  // Serial.println(data[1], BIN);
  // Serial.println(data[2], BIN);

  separated_data.c00 = ((uint32_t)data[0])<<12;
  separated_data.c00 += ((uint32_t)data[1])<<4;
  separated_data.c00 += ((uint8_t)data[2])>>4;
  // Serial.println("separeted data:");
  // Serial.println(separated_data.c00);

  separated_data.c10 = (uint32_t)(((uint8_t)data[2])&0x0F)<<16;
  separated_data.c10 += ((uint32_t)data[3])<<8;
  separated_data.c10 += ((uint32_t)data[4]);
  separated_data.c01 = ((uint32_t)data[5])<<8;
  separated_data.c01 += (uint32_t)data[6];
  separated_data.c11 = ((uint32_t)data[7])<<8;
  separated_data.c11 += (uint32_t)data[8];
  separated_data.c20 = ((uint32_t)data[9])<<8;
  separated_data.c20 += (uint32_t)data[10];
  separated_data.c21 = ((uint32_t)data[11])<<8;
  separated_data.c21 += (uint32_t)data[12];
  separated_data.c30 = ((uint32_t)data[13])<<8;
  separated_data.c30 += (uint32_t)data[14];

  char data2[3];
  read_reg_incremental(0x10,3, data2);

  separated_data.c0 = (data2[0])<<4;
  separated_data.c0 += ((data2[1] >> 4) & 0x0F);

  separated_data.c1 = (data2[1] & 0x0F) << 8;
  separated_data.c1 += (data2[2]);


  return separated_data;
}

int read_Pressure(){
  char pressuretable[3];
  uint32_t pressure;
  read_reg_incremental(0x00,3,pressuretable);
  pressure = ((uint32_t)pressuretable[0])<<16;
  pressure += ((uint32_t)pressuretable[1])<<8;
  pressure += ((uint32_t)pressuretable[2]);
  pressure = utoi(pressure,24);
  return ajustPressure(pressure);
}

int ajustPressure(int raw_pressure){
  float pressure_raw_sc = raw_pressure/ 524288.f;
  float temp_raw_sc = temperature()/524288.f;
  int pressure_comp = (float)data_coeff.c00 + pressure_raw_sc * ((float)data_coeff.c10 + pressure_raw_sc * ((float)data_coeff.c20 + pressure_raw_sc * (float)data_coeff.c30)) + temp_raw_sc * (float)data_coeff.c01 + temp_raw_sc * pressure_raw_sc * ((float)data_coeff.c1 + pressure_raw_sc * (float)data_coeff.c21);
  // Serial.println(pressure_comp);
  return pressure_comp;
}

float temperature(){
  char temperaturetable[3];
  uint32_t temperature;
  read_reg_incremental(0x03,3,temperaturetable);
  temperature = ((uint32_t)(temperaturetable[0]) << 16) + ((uint32_t)(temperaturetable[1]) << 8) + temperaturetable[2];
  temperature = utoi(temperature,24);
  return temperature;
}

float read_Temperature(){
  return ajustTemp(temperature());
}

float ajustTemp(int raw_temp){
  float temp_raw_sc = raw_temp/ 524288.f;
  float temp_comp = (float)data_coeff.c0*0.5f + (float)data_coeff.c1*temp_raw_sc;
  return temp_comp;
}

static inline int32_t utoi(int32_t input, int8_t MSBposition)
{
  if (input > pow(2, MSBposition - 1) - 1)
  {
    return input - pow(2, MSBposition);
  }
  return input;
}

void configPressureTemp(){

  Wire.beginTransmission(0x77);
  Wire.write(byte(0x06));
  Wire.write(byte(0b00100000));
  Wire.endTransmission();

  Wire.beginTransmission(0x77);
  Wire.write(byte(0x07));
  Wire.write(byte(0b10100000));
  Wire.endTransmission();

  Wire.beginTransmission(0x77);
  Wire.write(byte(0x08));
  Wire.write(byte(0b00000111));
  Wire.endTransmission();
}

float humidity_temp(){
  int i, j;
  int duree[42];
  unsigned long pulse;
  byte data[5];
  float humidite;
  float temperature;
  int broche = 16;

  pinMode(broche, OUTPUT_OPEN_DRAIN);
  digitalWrite(broche, HIGH);
  delay(250);
  digitalWrite(broche, LOW);
  delay(20);
  digitalWrite(broche, HIGH);
  delayMicroseconds(40);
  pinMode(broche, INPUT_PULLUP);
  
  while (digitalRead(broche) == HIGH);
  i = 0;

  do {
        pulse = pulseIn(broche, HIGH);
        duree[i] = pulse;
        i++;
  } while (pulse != 0);
 
  if (i != 42) 
    Serial.printf(" Erreur timing \n"); 

  for (i=0; i<5; i++) {
    data[i] = 0;
    for (j = ((8*i)+1); j < ((8*i)+9); j++) {
      data[i] = data[i] * 2;
      if (duree[j] > 50) {
        data[i] = data[i] + 1;
      }
    }
  }

  if ( (data[0] + data[1] + data[2] + data[3]) != data[4] ) 
    Serial.println(" Erreur checksum");

  humidite = data[0] + (data[1] / 256.0);
  temperature = data [2] + (data[3] / 256.0);
  return humidite;
  // Serial.printf(" Humidite = %4.0f \%%  Temperature = %4.2f degreC \n", humidite, temperature);
}

bool pluie(){
  if(digitalRead(32)){

    return true;
  }
  else{
    return false;
  }
  Serial.println(digitalRead(32));

}