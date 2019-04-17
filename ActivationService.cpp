/*
  ActivationService.cpp - Class and Methods to send activation request and
                    poll for activation status with BoT Service
  Created by Lokesh H K, April 17, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include "ActivationService.h"

ActivationService :: ActivationService(){
  store = KeyStore :: getKeyStoreInstance();
  bot = new BoTService();
}

bool ActivationService :: pollActivationStatus(){
  LOG("\nActivationService :: pollActivationStatus: Started polling BoT for activation status for the device...");
  int counter = 1;
  String response;
  do {
    LOG("\nActivationService :: pollActivationStatus: Checking activation status, attempt %d of %d", counter,MAXIMUM_TRIES);
    response = sendActivationRequest();
    if(response.equals("")){
      return true;
    }
    ++counter;
    delay(POLLING_INTERVAL_IN_MILLISECONDS);
  }while(counter <= MAXIMUM_TRIES);

   return false;
}

String ActivationService :: sendActivationRequest(){
  const char* deviceID = store->getDeviceID();

  StaticJsonBuffer<100> jsonBuffer;
  JsonObject& doc = jsonBuffer.createObject();
  JsonObject& botData = doc.createNestedObject("bot");
  botData["deviceID"] = deviceID;

  char payload[100];
  doc.printTo(payload);
  LOG("\nActivationService :: sendActivationRequest: Minified JSON payload to send: %s", payload);

  return bot->post(ACTIVATION_END_POINT,payload);
}

void ActivationService :: activateDevice(){
  store->initializeEEPROM();
  if(pollActivationStatus() == true){
    store->setDeviceState(DEVICE_ACTIVE);
    LOG("\nActivationService :: activateDevice: Activation successful. Triggering actions enabled");
  }
  else {
    LOG("\nActivationService :: activateDevice: Unable to activate device, try activating again");
  }
}
