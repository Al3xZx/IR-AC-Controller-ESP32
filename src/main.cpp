/*
 * Example
 *
 * If you encounter any issues:
 * - check the readme.md at https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md
 * - ensure all dependent libraries are installed
 * - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#arduinoide
 * - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#dependencies
 * - open serial monitor and check whats happening
 * - check full user documentation at https://sinricpro.github.io/esp8266-esp32-sdk
 * - visit https://github.com/sinricpro/esp8266-esp32-sdk/issues and check for existing issues or open a new one
 */

 // Custom devices requires SinricPro ESP8266/ESP32 SDK 2.9.6 or later

// Uncomment the following line to enable serial debug output
//#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
  #define DEBUG_ESP_PORT Serial
  #define NODEBUG_WEBSOCKETS
  #define NDEBUG
#endif

#include <Arduino.h>
#ifdef ESP8266
  #include <ESP8266WiFi.h>
#endif
#ifdef ESP32
  #include <WiFi.h>
#endif

#include <SinricPro.h>
#include "CondizionatoreIRController.h"

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Coolix.h>

// #define APP_KEY    getenv("APP_KEY") 
// #define APP_SECRET getenv("APP_SECRET") 
// #define DEVICE_ID  getenv("DEVICE_ID") 

// #define SSID       getenv("SSID")
// #define PASS       getenv("PASS")

#define BAUD_RATE  115200

#define GPIO_IR_PINOUT 16
IRCoolixAC coolix(GPIO_IR_PINOUT);

CondizionatoreIRController &condizionatoreIRController = SinricPro[DEVICE_ID];

/*************
 * Variables *
 ***********************************************
 * Global variables to store the device states *
 ***********************************************/

// PowerStateController
bool globalPowerState;

uint32_t acInternalState = 0;


// RangeController
std::map<String, int> globalRangeValues;

// ModeController
std::map<String, String> globalModes;



/*************
 * Callbacks *
 *************/

// PowerStateController
bool onPowerState(const String &deviceId, bool &state) {
  Serial.printf("[Device: %s]: Powerstate changed to %s\r\n", deviceId.c_str(), state ? "on" : "off");
  globalPowerState = state;
  if(state){
    //coolix.on();
    if(acInternalState == 0){
      Serial.println("Stato non trovato uso quello di default");
      //Mesg Desc.: Power: On, Mode: 2 (Auto), Fan: 0 (Auto0), Temp: 25C, Zone Follow: Off, Sensor Temp: Off
      coolix.on();
    } else {
      coolix.setRaw(acInternalState);
    }
  } else {
    //save current ac state used in on case
    acInternalState = coolix.getRaw();
    coolix.off();
  }
  coolix.send();
  return true; // request handled properly
}

// RangeController
bool onRangeValue(const String &deviceId, const String& instance, int &rangeValue) {
  Serial.printf("[Device: %s]: Value for \"%s\" changed to %d\r\n", deviceId.c_str(), instance.c_str(), rangeValue);
  globalRangeValues[instance] = rangeValue;
  return true;
}

bool onAdjustRangeValue(const String &deviceId, const String& instance, int &valueDelta) {
  globalRangeValues[instance] += valueDelta;
  Serial.printf("[Device: %s]: Value for \"%s\" changed about %d to %d\r\n", deviceId.c_str(), instance.c_str(), valueDelta, globalRangeValues[instance]);
  globalRangeValues[instance] = valueDelta;
  return true;
}

// ModeController
bool onSetMode(const String& deviceId, const String& instance, String &mode) {
  Serial.printf("[Device: %s]: Modesetting for \"%s\" set to mode %s\r\n", deviceId.c_str(), instance.c_str(), mode.c_str());
  globalModes[instance] = mode;
  return true;
}


/**********
 * Events *
 *************************************************
 * Examples how to update the server status when *
 * you physically interact with your device or a *
 * sensor reading changes.                       *
 *************************************************/

// PowerStateController
void updatePowerState(bool state) {
  condizionatoreIRController.sendPowerStateEvent(state);
}

// RangeController
void updateRangeValue(String instance, int value) {
  condizionatoreIRController.sendRangeValueEvent(instance, value);
}

// ModeController
void updateMode(String instance, String mode) {
  condizionatoreIRController.sendModeEvent(instance, mode, "PHYSICAL_INTERACTION");
}

/********* 
 * Setup *
 *********/

void setupSinricPro() {

  // PowerStateController
  condizionatoreIRController.onPowerState(onPowerState);

  // RangeController
  condizionatoreIRController.onRangeValue("rangeInstance1", onRangeValue);
  condizionatoreIRController.onAdjustRangeValue("rangeInstance1", onAdjustRangeValue);


  // ModeController
  condizionatoreIRController.onSetMode("modeInstance1", onSetMode);

  SinricPro.onConnected([]{ Serial.printf("[SinricPro]: Connected\r\n"); });
  SinricPro.onDisconnected([]{ Serial.printf("[SinricPro]: Disconnected\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
};

void setupWiFi() {
  #if defined(ESP8266)
    WiFi.setSleepMode(WIFI_NONE_SLEEP); 
    WiFi.setAutoReconnect(true);
  #elif defined(ESP32)
    WiFi.setSleep(false); 
    WiFi.setAutoReconnect(true);
  #endif

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.printf("[WiFi]: Connecting to %s", WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  Serial.printf("connected\r\n");
}

void setup() {
  Serial.begin(BAUD_RATE);
  setupWiFi();
  setupSinricPro();
  coolix.begin();
}

/********
 * Loop *
 ********/

void loop() {
  SinricPro.handle();
}