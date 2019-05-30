/*
  BoTESP32SDK.h - Common header file for BoT ESP-32 SDK Library.
                  Includes all required header files from Arduino and other libraries
  Created by Lokesh H K, April 9, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#ifndef BoTESP32SDK_h
#define BoTESP32SDK_h

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <EEPROM.h>
#include <HTTPClient.h>
#include <mbedtls/pk.h>
#include <mbedtls/error.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#define EEPROM_SIZE 1
#define DEVICE_NEW  0
#define DEVICE_PAIRED 1
#define DEVICE_ACTIVE 2
#define DEVICE_MULTIPAIR 3
#define LOG Serial.printf

//RemoteDebug Speicifics go here

//#define DEBUG_DISABLED true
#include <RemoteDebug.h>

//Debug levels for RemoteDebug feature
//Effective only if RemoteDebug is enabled
#define BoT_DEBUG 2
#define BoT_INFO 3
#define BoT_WARNING 4
#define BoT_ERROR 5
#ifndef DEBUG_DISABLED
  extern RemoteDebug Debug;
#endif

struct Action{
  char* actionID;
  char* actionFrequency;
  unsigned long triggeredTime;
};

#endif
