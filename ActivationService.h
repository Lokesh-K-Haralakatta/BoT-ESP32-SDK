/*
  ActivationService.h - Class and Methods to send activation request and
                    poll for activation status with BoT Service
  Created by Lokesh H K, April 17, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#ifndef ActivationService_h
#define ActivationService_h
#include "BoTESP32SDK.h"
#include "Storage.h"
#include "BoTService.h"
#define POLLING_INTERVAL_IN_MILLISECONDS 10000
#define MAXIMUM_TRIES 3
#define ACTIVATION_END_POINT "/status"

class ActivationService {
  public:
    ActivationService();
    void activateDevice();
  private:
    KeyStore *store;
    BoTService *bot;
    String sendActivationRequest();
    bool pollActivationStatus();
};
#endif
