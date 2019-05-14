/*
  PairingService.cpp - Class and Methods to poll for pairing status with BoT Service
  Created by Lokesh H K, April 16, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include "PairingService.h"

PairingService :: PairingService(){
  store = KeyStore :: getKeyStoreInstance();
}

bool PairingService :: isMultipair(){
  return (store->getDeviceState() == DEVICE_MULTIPAIR?true:false);
}

bool PairingService :: isPairable(){
  if(isMultipair())
    return true;

  return (store->getDeviceState() == DEVICE_NEW?true:false);
}

String PairingService :: getPairingStatus(){
  BoTService* bots = new BoTService();
  String response = bots->get(PAIRING_END_POINT);
  delete bots;
  LOG("\nPairingService :: getPairingStatus : %s", response.c_str());
  return response;
}

bool PairingService :: pollPairingStatus(){
  if (isPairable() == false) {
      return false;
  }

  LOG("\nPairingService :: pollPairingStatus: Started polling BoT for pairing status for the device...");
  int counter = 1;
  String response;
  do {
    LOG("\nPairingService :: pollPairingStatus: Checking pairing status, attempt %d of %d", counter,MAXIMUM_TRIES);
    response = getPairingStatus();
    if(response.indexOf("true") != -1){
      return true;
    }
    ++counter;
    delay(POLLING_INTERVAL_IN_MILLISECONDS);
  }while(counter <= MAXIMUM_TRIES);

   return false;
}

void PairingService :: pairDevice(){
  store->initializeEEPROM();
  if (isPairable() == false) {
      return;
  }

  if (isMultipair()) {
      return;
  }

  if(pollPairingStatus() == true){
    if (isPairable() == false) {
        return;
    }
    store->setDeviceState(DEVICE_PAIRED);
    LOG("\nPairingService :: pairDevice: Device successfully paired. Ready to activate.");
    //Remove Device publickey from SPIFFS
    ActivationService *actService = new ActivationService();
    actService->activateDevice();
    LOG("\nPairingService :: pairDevice: Returned from actService->activateDevice()");
    delete actService;
  }
  else {
    LOG("\nPairingService :: pairDevice: Device pairing not yet completed.Try again...");
  }
}
