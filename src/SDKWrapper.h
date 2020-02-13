/*
  SDKWrapper.h - Class and Methods to provide an interface for end user to consume
                 ESP-32 SDK functionality without Webserver
  Created by Lokesh H K, July 03, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#ifndef SDKWrapper_h
#define SDKWrapper_h
#include "BoTESP32SDK.h"
#include "Storage.h"
#include "PairingService.h"
#include "ActionService.h"
#include "ConfigurationService.h"
#include "BluetoothService.h"
class SDKWrapper {
  public:
          SDKWrapper();
          String* getActions();
          bool pairAndActivateDevice();
          bool triggerAction(const char* actionID, const char* value = NULL, const char* altID = NULL);
          void waitForSeconds(const int seconds);
  private:
    KeyStore* store;
    PairingService* pairService;
    ActionService* actionService;
    ConfigurationService* configService;
    bool isDevicePaired();
};
#endif
