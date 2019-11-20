/*
  ControllerService.h - Class and Methods to interface between Webserver End Points
                        and the backend service components
  Created by Lokesh H K, April 24, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#ifndef ControllerService_h
#define ControllerService_h
#include "BoTESP32SDK.h"
#include "Storage.h"
#include "PairingService.h"
#include "ActionService.h"
#include "ConfigurationService.h"
class ControllerService {
  public:
          ControllerService();
          static ActionService* getActionServiceObject();
          void getActions(AsyncWebServerRequest *request);
          void pairDevice(AsyncWebServerRequest *request);
          void getQRCode(AsyncWebServerRequest *request);
          void postAction(AsyncWebServerRequest *request);
  private:
    KeyStore* store;
    static ActionService* actionService;
    int triggerAction(const char* actionID);
};
#endif
