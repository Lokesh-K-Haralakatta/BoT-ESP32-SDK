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
  debugD("\nPairingService :: getPairingStatus : %s", response.c_str());
  return response;
}

bool PairingService :: pollPairingStatus(){
  if (isPairable() == false) {
      return false;
  }

  debugD("\nPairingService :: pollPairingStatus: Started polling BoT for pairing status for the device...");
  int counter = 1;
  String response;
  do {
    debugD("\nPairingService :: pollPairingStatus: Checking pairing status, attempt %d of %d", counter,MAXIMUM_TRIES);
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
  if(!isPairable() || isMultipair())
    return;

  if(pollPairingStatus() == true){
    store->setDeviceState(DEVICE_PAIRED);
    debugI("\nPairingService :: pairDevice: Device successfully paired. Ready to activate.");
    //Remove Device publickey from SPIFFS
    ActivationService *actionService = new ActivationService();
    actionService->activateDevice();
    debugD("\nPairingService :: pairDevice: Returned from actService->activateDevice()");
    delete actionService;
  }
  else {
    debugW("\nPairingService :: pairDevice: Device pairing not yet completed.Try again...");
  }
}
