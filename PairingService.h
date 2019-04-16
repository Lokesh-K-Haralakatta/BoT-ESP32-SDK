/*
  PairingService.h - Class and Methods to poll for pairing status with BoT Service
  Created by Lokesh H K, April 16, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#ifndef PairingService_h
#define PairingService_h
#include "BoTESP32SDK.h"
#include "Storage.h"
#include "BoTService.h"
#define POLLING_INTERVAL_IN_MILLISECONDS 10000
#define MAXIMUM_TRIES 10
#define END_POINT "/pair"

class PairingService {
  public:
    PairingService();
    void pairDevice();
  private:
    KeyStore *store;
    BoTService *bot;
    bool isPairable();
    bool isMultipair();
    String getPairingStatus();
    bool pollPairingStatus();
};
#endif
