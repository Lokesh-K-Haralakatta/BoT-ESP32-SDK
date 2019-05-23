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
          void getActions(AsyncWebServerRequest *request);
          void pairDevice(AsyncWebServerRequest *request);
          void triggerAction(AsyncWebServerRequest *request, JsonVariant &json);
  private:
    KeyStore* store;
    PairingService* pairService;
    ActivationService* activateService;
    ActionService* actionService;
    ConfigurationService* configService;
};
#endif
