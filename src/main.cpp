#include <Arduino.h>
#include <stdio.h>
#include <SPI.h>
#include <WiFi.h>
#include "wifi_save.h"
#include "DHT.h"
#include "ThingsBoard.h"
#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>
#include <Adafruit_ADS1X15.h>
#include <string.h>
#include <strings.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_log.h"
#include "ADS1X15.h"

#define TOKEN "9cmRR9KzJHszhyzxk7Xa"

//"TKbjHLmi0kHscYbDgb9f"
//"MWJBKhS1AdIklUqUez6B"
//"TKbjHLmi0kHscYbDgb9f" // test _devvvv
//"9cmRR9KzJHszhyzxk7Xa" // nodo_M2M
#define THINGSBOARD_SERVER "thingsboard.cloud"
// ADC MODULES
Adafruit_ADS1115 ads; // ADDR: 0x48(GND) , 0x49(Vin) , 0x4A(SDA) , 0x4B(SCL)
//Adafruit_ADS1115 ads2; // ADDR: 0x48(GND) , 0x49(Vin) , 0x4A(SDA) , 0x4B(SCL)
/*defines PZEM*/
#if !defined(PZEM_SERIAL)
#define PZEM_SERIAL Serial2
#endif
/*Telemetry interval*/
float time_send = 1; // min
/* VOLTAGE SENSORS*/
float Vbat1 = 0.00;
float Vbat2 = 0.00;
float Vp1 = 0.00;
float Vp2 = 0.00;
int avgCountVS = 4;
float VbatDivRatio = 24.5000; // 27.2000 -- 24.5000
float VpDivRatio = 40.2156;   //47.9000 --  40.2156
/* CURRENT SENSORS */
float I1 = 0.00;
float I2 = 0.00;
float I3 = 0.00;
float I4 = 0.00;
float avgCountIs = 4.0000;
float I1mp = 2.5000;
float I2mp = 2.5000;
float I3mp = 2.5000;
float I4mp = 2.5000;
/*AC Sensors*/
const int PZEM_RX_PIN = 16;
const int PZEM_TX_PIN = 17;
float pzemVolt = 0;
float pzemCte = 0;
float pzemPow = 0;
float pzemEne = 0;
float pzemFreq = 0;
float pzemPF = 0;
/* TEMPERATURE HUM */
DHT dht(15, DHT22);
float h = 0;
float t = 0;
/* GPS */
String LATval = "######";
String LNGval = "######";
char inChar;
String gpsData;
String latt;
String la;
String lonn;
String lo;
float lattt;
float lonnn;
int latDeg;
int lonDeg;
float latMin;
float lonMin;
float latttt;
float lonnnn;
String sGPRMC;
//ADS1115 ads1(0x48);
/* WIFI y TB */
WiFiClient espClient;
ThingsBoard tb(espClient);
PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);
/* REMOTE */
bool subscribed = false;
bool stateRelay1 = false;
const int relay1 = 26;
const int relay2 = 25;
bool r1Status = false;
bool userstatus = false;
int memstate = 1;
/* Batery status */
bool lowb1 = false;
bool lowb2 = false;
int state = 0;
int prev_state = 0 ;
/* Functions */
void midPoints();
void SensorDHT();
void readSensorV();
void readSensorI();
void gpsdata();
void connectTB();
void checkCharge();
void readSensorV2();
void AcData();



void setup()
{
    if (!ads.begin(0x48)) 
    {
    Serial.println("Failed to initialize ADS.");
    while (1);
    }
    Serial.begin(9600);
    pinMode(26, OUTPUT);
    pinMode(25, OUTPUT);
    pinMode(32, OUTPUT);
    pinMode(33, OUTPUT);
    dht.begin();
    if (wifi_set_main())
    {
        Serial.println("Connect WIFI SUCCESS");
    }
    else
    {
        Serial.println("Connect WIFI FAULT");
    }
}



void loop()
{  
    connectTB();
    readSensorV();
    SensorDHT();
    AcData();
    //delay(1000);
    vTaskDelay(time_send*60000 / portTICK_PERIOD_MS); //300.000 = 5min

}


void gpsdata(){
    Serial.println("GPS DATA");
    while (Serial.available()) {
        inChar = Serial.read();
        gpsData += inChar;
        if(inChar == '$'){
        gpsData = Serial.readStringUntil('\n');
        break;
        }
    }
    sGPRMC = gpsData.substring(0, 5);
    if (sGPRMC == "GPRMC") {
    Serial.flush();
    latt = gpsData.substring(19, 28);
    la = gpsData.substring(29, 30);
    lonn = gpsData.substring(31, 41);
    lo = gpsData.substring(42, 43);
    lattt = latt.toFloat();
    lonnn = lonn.toFloat();

   }

    if (la == "N" and lo == "E") {

      latDeg = float(int(lattt / 100));
      latMin = float(lattt - (latDeg * 100));
      latMin = latMin / 60;
        
      lonDeg = float(int(lonnn / 100));
      lonMin = float(lonnn - (lonDeg * 100));
      lonMin = lonMin / 60;
          
      latttt = latDeg + latMin;
      lonnnn = lonDeg + lonMin;
      tb.sendTelemetryFloat("latitude", latttt);
      tb.sendTelemetryFloat("longitude", lonnnn);
      LATval = String(latttt);
      LNGval = String(lonnnn);

    }
    if (la == "S" and lo == "W") {

      latDeg = float(int(lattt / 100));
      latMin = float(lattt - (latDeg * 100));
      latMin = latMin / 60;   
      lonDeg = float(int(lonnn / 100));
      lonMin = float(lonnn - (lonDeg * 100));
      lonMin = lonMin / 60;    
      latttt = latDeg + latMin;
      lonnnn = lonDeg + lonMin;
      tb.sendTelemetryFloat("latitude", latttt*-1);
      tb.sendTelemetryFloat("longitude", lonnnn*-1);
      LATval = String(latttt);
      LNGval = String(lonnnn);
    }  
}

void connectTB(){
  if (!tb.connected()) {
    subscribed = false;
    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect, retrying ...");
      return;
    }
  }
}

void SensorDHT()
{
    h = dht.readHumidity();
    t = dht.readTemperature();
    if (isnan(h) || isnan(t))
    {
        Serial.println("Error de sensor DHT11");
        return;
    }
    tb.sendTelemetryFloat("Temperature",t);
    tb.sendTelemetryFloat("Humidity",h);
}
void readSensorV2(){
  int16_t adc0, adc1, adc2, adc3;
  float volts0, volts1, volts2, volts3;

  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  adc2 = ads.readADC_SingleEnded(2);
  adc3 = ads.readADC_SingleEnded(3);

  volts0 = ads.computeVolts(adc0);
  volts1 = ads.computeVolts(adc1);
  volts2 = ads.computeVolts(adc2);
  volts3 = ads.computeVolts(adc3);

  Serial.println("-----------------------------------------------------------");
  Serial.print("AIN0: "); Serial.print(adc0); Serial.print("  "); Serial.print(volts0); Serial.println("V");
  Serial.print("AIN1: "); Serial.print(adc1); Serial.print("  "); Serial.print(volts1); Serial.println("V");
  Serial.print("AIN2: "); Serial.print(adc2); Serial.print("  "); Serial.print(volts2); Serial.println("V");
  Serial.print("AIN3: "); Serial.print(adc3); Serial.print("  "); Serial.print(volts3); Serial.println("V");

  delay(1000);
}



void readSensorV(){
  /////////// VOLTAGE & CURRENT SENSORS /////////////
  Vbat1 = 0.0000; 
  Vbat2 = 0.0000;     //Clear Previous Input Voltage
  Vp1 = 0.0000; 
  Vp2 = 0.0000;     //Clear Previous Output Voltage

  //VOLTAGE SENSOR - Instantenous Averaging
  for(int i = 0; i<avgCountVS; i++){
    Vbat1 = Vbat1 + ads.computeVolts(ads.readADC_SingleEnded(0));
    //Vbat2 = Vbat2 + ads.computeVolts(ads.readADC_SingleEnded(1));
    Vp1 = Vp1 + ads.computeVolts(ads.readADC_SingleEnded(2));
    //Vp2 = Vp2 + ads.computeVolts(ads.readADC_SingleEnded(3));
  }
  float Vbat_1  = (Vbat1/avgCountVS);
  //float Vbat_2  = (Vbat2/avgCountVS);
  float Vp_1 = (Vp1/avgCountVS);
  //float Vp_2 = (Vp2/avgCountVS);

  float Vbat_1r  = (Vbat_1*VbatDivRatio);
  //float Vbat_2r  = (Vbat_2*VbatDivRatio);
  float Vp_1r = (Vp_1*VpDivRatio);
  //float Vp_2r = (Vp_2*VpDivRatio);

  // Serial.print("Vbat1: " ); Serial.println(Vbat_1r);
  // Serial.print("Vbat2: " ); Serial.println(Vbat_2r);
  // Serial.print("Vp1: " ); Serial.println(Vp_1r);
  // Serial.print("Vp2: " ); Serial.println(Vp_2r);
  // Serial.println("------------------------------");
  // Serial.print("Vbat_1: " ); Serial.println(Vbat_1);
  // Serial.print("Vbat_2: " ); Serial.println(Vbat_2);
  // Serial.print("Vp_1: " ); Serial.println(Vp_1);
  // Serial.print("Vp_2: " ); Serial.println(Vp_2);
  // Serial.println("**********************************");
  //delay(1000);

    tb.sendTelemetryFloat("VBat1",Vbat_1r);
    //tb.sendTelemetryFloat("VBat2",Vbat_2r);
    tb.sendTelemetryFloat("VPanel1",Vp_1r);
    //tb.sendTelemetryFloat("VPanel2",Vp_2r);
}
  

void readSensorI(){
    // I1 = 0.0000;
    // I2 = 0.0000;
    // I3 = 0.0000;
    // I4 = 0.0000;
    // for (int i = 0; i < avgCountIs; i++){
    //     I1 = I1 + float(ads2.toVoltage(ads2.readADC(0)));
    //     I2 = I2 + float(ads2.toVoltage(ads2.readADC(1)));
    //     I3 = I3 + float(ads2.toVoltage(ads2.readADC(2)));
    //     I4 = I4 + float(ads2.toVoltage(ads2.readADC(3)));
    // }
    // float VI1 = ((I1 / avgCountIs));
    // I1 = (((I1 / avgCountIs))-I1mp)/0.0631;
    // I2 = (((I2 / avgCountIs))-I2mp)/0.0631;
    // I3 = (((I3 / avgCountIs))-I3mp)/0.0631;
    // I4 = (((I4 / avgCountIs))-I4mp)/0.0631;

    // Serial.print("I1V: "); Serial.println(VI1);
    // Serial.print("I1mp: "); Serial.println(I1mp);
    // Serial.print("I1: "); Serial.println(I1);
    // Serial.print("I2: "); Serial.println(I2);
    // Serial.print("I3: "); Serial.println(I3);
    // Serial.print("I4: "); Serial.println(I4);

    // tb.sendTelemetryFloat("I1",I1);
    // tb.sendTelemetryFloat("I2",I2);
    // tb.sendTelemetryFloat("I3",I3);
    // tb.sendTelemetryFloat("I4",I4);
}

void midPoints(){
    // I1 = 0.0000;
    // I2 = 0.0000;
    // I3 = 0.0000;
    // I4 = 0.0000;
    // I1 = float(ads2.toVoltage(ads2.readADC(0)));
    // I2 = float(ads2.toVoltage(ads2.readADC(1)));
    // I3 = float(ads2.toVoltage(ads2.readADC(2)));
    // I4 = float(ads2.toVoltage(ads2.readADC(3)));
    // I1mp = (I1);
    // I2mp = (I2);
    // I3mp = (I3);
    // I4mp = (I4);
    
    // Serial.print("I1mp: ");
    // Serial.println(I1mp);
    // Serial.print("I2mp: ");
    // Serial.println(I2mp);
    // Serial.print("I3mp: ");
    // Serial.println(I3mp);
    // Serial.print("I4mp: ");
    // Serial.println(I4mp);
}


void AcData(){
    pzemVolt = pzem.voltage();
    pzemCte = pzem.current();
    pzemPow = pzem.power();
    pzemEne = pzem.energy();
    pzemFreq = pzem.frequency();
    pzemPF = pzem.pf();
    if (isnan(pzemVolt))
    {
        pzemVolt = 0;
    }
    if (isnan(pzemCte))
    {
        pzemCte = 0;
    }
    if (isnan(pzemPow))
    {
        pzemPow = 0;
    }
    if (isnan(pzemEne))
    {
        pzemEne = 0;
    }
    if (isnan(pzemFreq))
    {
        pzemFreq = 0;
    }
    if (isnan(pzemPF))
    {
        pzemPF = 0;
    }
    tb.sendTelemetryFloat("VAC", pzemVolt);
    tb.sendTelemetryFloat("IAC", pzemCte);
    float powerout = (pzemVolt * pzemCte);
    tb.sendTelemetryFloat("POWER", powerout);
    tb.sendTelemetryFloat("Frec", pzemFreq);
    tb.sendTelemetryFloat("Energy", pzemEne);
    tb.sendTelemetryFloat("Fp", pzemPF);
}