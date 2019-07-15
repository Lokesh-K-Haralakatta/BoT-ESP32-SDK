/*
  ConfigurationService.h - Class and Methods to configure the device specifics
                          and prepare environment to perform required operation
  Created by Lokesh H K, April 22, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#ifndef ConfigurationService_h
#define ConfigurationService_h
#include "BoTESP32SDK.h"
#include "Storage.h"
#include "ActivationService.h"
#include "PairingService.h"
#include "ActionService.h"

class ConfigurationService {
  public:
          ConfigurationService();
          void initialize();
          void configureDevice();
  private:
          KeyStore* store;
          PairingService* pairService;
          ActivationService* activateService;
};

#endif
