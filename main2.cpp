// This sketch demonstrates connecting and sending telemetry
// using ThingsBoard SDK and GSM modem, such as SIM900
//
// Hardware:
//  - Arduino Uno
//  - SIM900 Arduino shield connected to Arduino Uno

// Select your modem:
#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_SIM900
// #define TINY_GSM_MODEM_UBLOX
// #define TINY_GSM_MODEM_BG96
// #define TINY_GSM_MODEM_A6
// #define TINY_GSM_MODEM_A7
// #define TINY_GSM_MODEM_M590
// #define TINY_GSM_MODEM_ESP8266

#include <TinyGsmClient.h>
//#include <SoftwareSerial.h>
#include "ThingsBoard.h"

// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "internet";
const char user[] = "";
const char pass[] = "";

// See https://thingsboard.io/docs/getting-started-guides/helloworld/
// to understand how to obtain an access token
#define TOKEN               "nRzuQ3Qa6Dj1jBssuYwe"
#define THINGSBOARD_SERVER  "demo.thingsboard.io"

// Baud rate for debug serial
#define SERIAL_DEBUG_BAUD   115200

// Serial port for GSM shield
//SoftwareSerial Serial2(7, 8); // RX, TX pins for communicating with modem

// Initialize GSM modem
TinyGsm modem(Serial2);

// Initialize GSM client
TinyGsmClient client(modem);

// Initialize ThingsBoard instance
ThingsBoard tb(client);

// Set to true, if modem is connected
bool modemConnected = false;

int pump = 9;
bool pumpStatus = false;
bool previousState = false;

void setup() {
  // Set console baud rate
  Serial.begin(SERIAL_DEBUG_BAUD);
  pinMode(pump, OUTPUT);
  digitalWrite(pump, LOW);

  // Set GSM module baud rate
  Serial2.begin(115200);
  delay(3000);

  // Lower baud rate of the modem.
  // This is highly practical for Uno board, since SoftwareSerial there
  // works too slow to receive a modem data.
  Serial2.write("AT+IPR=9600\r\n");
  Serial2.end();
  Serial2.begin(9600);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println(F("Initializing modem..."));
  modem.restart();

  String modemInfo = modem.getModemInfo();
  Serial.print(F("Modem: "));
  Serial.println(modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");
}

// Processes function for RPC call "example_set_temperature"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response processTemperatureChange(const RPC_Data &data)
{
  Serial.println("Received the set temperature RPC method");

  // Process data

  float example_temperature = data["temp"];

  Serial.print("Example temperature: ");
  Serial.println(example_temperature);

  // Just an response example
  return RPC_Response("example_response", 42);
}

// Processes function for RPC call "example_set_switch"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response processSwitchChange(const RPC_Data &data)
{
  Serial.println("Received the set switch method");
  char params[10];
  serializeJson(data, params);
  Serial.println(params);
  String _params = params;
  if (_params == "true") {
    Serial.println("True");
    pumpStatus = true;
    //digitalWrite(pump, HIGH);
    return RPC_Response("example_set_switch", "true");
  }
  else  if (_params == "false")  {
    Serial.println("False");
    pumpStatus = false;
    // digitalWrite(pump, LOW);
    return RPC_Response("example_set_switch", "false");
  }

  //  // Process data
  //
  //  bool switch_state = data["switch"];
  //
  //  Serial.print("Example switch state: ");
  //  Serial.println(switch_state);
  //  digitalWrite(pump, switch_state);
  //
  //
  //  // Just an response example
  //  return RPC_Response("example_set_switch", switch_state);
}

const size_t callbacks_size = 2;
RPC_Callback callbacks[callbacks_size] = {
  { "example_set_temperature",    processTemperatureChange },
  { "example_set_switch",         processSwitchChange }
};

bool subscribed = false;

void loop() {


  delay(100);

  if (!modemConnected) {
    pumpStatus = false;
    Serial.print(F("Waiting for network..."));
    if (!modem.waitForNetwork()) {
      Serial.println(" fail");
      delay(10000);
      return;
    }
    Serial.println(" OK");

    Serial.print(F("Connecting to "));
    Serial.print(apn);
    if (!modem.gprsConnect(apn, user, pass)) {
      pumpStatus = false;
      Serial.println(" fail");
      delay(10000);
      return;
    }

    modemConnected = true;
    Serial.println(" OK");
  }

  if (!tb.connected()) {
    subscribed = false;
    pumpStatus = false;
    // modemConnected = false;
    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      return;
    }
  }

  if (!subscribed) {
    Serial.println("Subscribing for RPC...");

    // Perform a subscription. All consequent data processing will happen in
    // processTemperatureChange() and processSwitchChange() functions,
    // as denoted by callbacks[] array.
    if (!tb.RPC_Subscribe(callbacks, callbacks_size)) {
      Serial.println("Failed to subscribe for RPC");
      return;
    }

    Serial.println("Subscribe done");
    subscribed = true;
  }



  //Serial.println("Sending data...");

  // Uploads new telemetry to ThingsBoard using MQTT.
  // See https://thingsboard.io/docs/reference/mqtt-api/#telemetry-upload-api
  // for more details

  tb.sendTelemetryInt("temperature", 30);
  tb.sendTelemetryFloat("humidity", 62.5);
  if (previousState != pumpStatus) {
    digitalWrite(pump, pumpStatus);
    previousState = pumpStatus;
    tb.sendAttributeBool("pumpStatus", pumpStatus);
  }

  tb.loop();
}
