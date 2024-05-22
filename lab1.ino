#include <Wire.h>
int AN_lumiere_in = 34;
int AN_directionVENT = 35;
int scl = 22;
int sda = 21;
int adress = 0x77; 
int reg = 0x0D;
uint32_t scalingFactor = 253952;
float waterlvl = 0;

int16_t c0;
int16_t c1;
int32_t c00;
int32_t c10;
int16_t c01;
int16_t c11;
int16_t c20;
int16_t c21;
int16_t c30;


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

void setup() {
 // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();
  pinMode(AN_lumiere_in, INPUT);
  pinMode(AN_directionVENT, INPUT);

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
}

void loop() {
  // pressure();
  // Serial.println(data_coeff.c0);
  // configPressureTemp();
  // Serial.println(read_Pressure());
  // humidity_temp();
  //lumiere();
  // pluie();
  // direction();
  wind();
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


void lumiere(){
  int lumval = analogRead(AN_lumiere_in);
  float lux = 0.0756f*lumval - 35;
  Serial.println(lux);
  delay(100);
}

void direction(){
  int dirvalue = analogRead(AN_directionVENT);
  int value[] = {226,1005,3925,3050};
  char* directiontable[] = {"north","east","south","west"};
  int smallestdelta = 1000;
  int dirint;
  for(int i = 0; i < 4; i++){
    int delta = abs(dirvalue -value[i]);
    if (delta < smallestdelta){
      smallestdelta = delta;
      dirint = i;
    }
  }
  Serial.println(directiontable[dirint]);
}

void wind(){


  {
  if (millis() >= lastResetTime + 1000)
  {
    windSpeed = windSpeedClick * 2.4;
    windSpeedClick = 0;
    lastResetTime = millis();
  }
  else
    windSpeedClick += 1;
}
  static bool oldClick = false;
  static long start = millis();
  static int clicks = 0;
  bool val = digitalRead(27);
  

  if (oldClick)
  {
    if (val == 0)
    {
      clicks++;
      oldClick = false;
    }
  }
  else
  {
    if (val > 0)
    {
      oldClick = true;
    }
  }
  float windSpeed = clicks * 1000.0 / (millis() - start);

  if (millis() - start > 1000)
  {
    start = millis();
    clicks = 0;
  }

  Serial.println(windSpeed);
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

float ajustPressure(int raw_pressure){
  float pressure_raw_sc = raw_pressure/ 524288.f;
  float temp_raw_sc = temperature()/524288.f;
  float pressure_comp = (float)data_coeff.c00 + pressure_raw_sc * ((float)data_coeff.c10 + pressure_raw_sc * ((float)data_coeff.c20 + pressure_raw_sc * (float)data_coeff.c30)) + temp_raw_sc * (float)data_coeff.c01 + temp_raw_sc * pressure_raw_sc * ((float)data_coeff.c1 + pressure_raw_sc * (float)data_coeff.c21);
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

void humidity_temp(){
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
  Serial.printf(" Humidite = %4.0f \%%  Temperature = %4.2f degreC \n", humidite, temperature);
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