/*
  SDKWrapper.cpp - Methods to provide an interface for end user to consume
                 ESP-32 SDK functionality without Webserver
  Created by Lokesh H K, July 03, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include "SDKWrapper.h"

SDKWrapper :: SDKWrapper(){
  store = KeyStore :: getKeyStoreInstance();
  pairService = new PairingService();
  actionService = ActionService :: getActionServiceInstance();
  configService = new ConfigurationService();
}

bool SDKWrapper :: isDevicePaired(){
  //Check pairing status for the device
  String* psResponse = pairService->getPairingStatus();
  debugI("\nSDKWrapper :: isDevicePaired: Pairing Status Response: %s",psResponse->c_str());
  debugI("\nSDKWrapper :: isDevicePaired: Device State -> %s",store->getDeviceStatusMsg());

  if(psResponse->indexOf("true") != -1)
    return true;
  else
    return false;
}

bool SDKWrapper :: pairAndActivateDevice(){
  bool deviceStatus = false;
  //Device is already paired, check for device validity
  if(isDevicePaired()){
    debugI("\nSDKWrapper :: pairAndActivateDevice: Device is already paired, checking device's state is valid or not");
    //Below situation occurs when the same device is switched between Multipair and Singlepair
    //Reset Device State and Initialize
    debugI("\nSDKWrapper :: pairAndActivateDevice: Device State -> %s",store->getDeviceStatusMsg());
    if((!store->isDeviceMultipair() && store->getDeviceState() == DEVICE_MULTIPAIR) ||
       (store->isDeviceMultipair() && store->getDeviceState() != DEVICE_MULTIPAIR))
    {
      debugI("\nSDKWrapper :: pairAndActivateDevice: Invalid device state, initializing as new device");
      store->resetDeviceState();
      store->resetQRCodeStatus();
      configService->initialize();
      configService->configureDevice();
    }
    else
      debugI("\nSDKWrapper :: pairAndActivateDevice: Valid device state, no need to initialize and configure");
  }
  else {
  //Proceed with complete flow to pair device using BLE followed by configure
  debugI("\nSDKWrapper :: pairAndActivateDevice: Device is not paired yet, needs initialization");
  BluetoothService* bleService = new BluetoothService();
  configService->initialize();
  debugD("\nSDKWrapper :: pairAndActivateDevice: Free Heap before BLE Init: %u", ESP.getFreeHeap());
  bleService->initializeBLE();
  bool bleClientConnected = false;
  //Wait till device gets paired from FINN APP through BLE
  do {
    delay(2000);
    bleClientConnected = bleService->isBLEClientConnected();
    if(!bleClientConnected)
      debugI("\nSDKWrapper :: pairAndActivateDevice: Waiting for BLE Client to connect...");
    else
      debugI("\nSDKWrapper :: pairAndActivateDevice: BLE Client connected to BLE Server...");
  }while(!bleClientConnected);

  //Wait till BLE Client disconnects from BLE Server
  while(bleClientConnected){
    debugI("\nSDKWrapper :: pairAndActivateDevice: Waiting for BLE Client to disconnect from BLE Server");
    bleClientConnected = bleService->isBLEClientConnected();
    delay(2000);
  }

  //Release memory used by BLE Service once BLE Client gets disconnected
  if(!bleClientConnected) bleService->deInitializeBLE();
  debugD("\nSDKWrapper :: pairAndActivateDevice: Free Heap after BLE deInit: %u", ESP.getFreeHeap());

  //Deallocate bleService memory
  delete bleService;

  //Proceed with configuring the device
  configService->configureDevice();

 }

 //Wait till device gets paired from FINN Application
 while(!isDevicePaired()){
   debugI("\nSDKWrapper :: pairAndActivateDevice: Waiting for device pairing get completed from FINN Application");
   delay(2000);
 }

 //Call pairing service pairDevice method to activate the device
 pairService->pairDevice();

 //Device State should be active at this Point, if it's single pair
 if(!store->isDeviceMultipair() && (store->getDeviceState() == DEVICE_ACTIVE)){
   deviceStatus = true;
 }
 //Device State should be multipair at this Point, if it's maulti pair
 else if(store->isDeviceMultipair() && (store->getDeviceState() == DEVICE_MULTIPAIR)){
   deviceStatus = true;
 }
 else {
   deviceStatus = false;
 }

 return deviceStatus;
}

String* SDKWrapper :: getActions(){
  return actionService->getActions();
}

bool SDKWrapper :: triggerAction(const char* actionID, const char* value, const char* altID){
  if(store->getDeviceState() < DEVICE_ACTIVE){
    debugW("\nSDKWrapper :: triggerAction : Invalid Device state to trigger action");
    return false;
  }
  else {
    if(actionID == NULL){
      debugW("\nSDKWrapper :: triggerAction : Missing actionID");
      return false;
    }
    else {
      if(store->isDeviceMultipair()){
        altID = (altID == NULL)?store->getAlternateDeviceID():altID;
        if(altID == NULL){
          debugW("\nSDKWrapper :: triggerAction : Missing alternateID");
          return false;
        }
        else {
          debugD("\nSDKWrapper :: triggerAction : deviceAltID : %s", altID);
        }
      }

      bool triggerResult = false;
      String* response = actionService->triggerAction(actionID,value);
      if(response != NULL){
        debugD("\nSDKWrapper :: triggerAction: Response: %s", response->c_str());
        if(response->indexOf("OK") != -1) {
          debugI("\nSDKWrapper :: triggerAction: Action triggered successful");
          triggerResult = true;
        }
        else if(response->indexOf("Action not found") != -1){
          debugW("\nSDKWrapper :: triggerAction: Action not triggered as its not found");
          triggerResult = false;
        }
        else {
          debugE("\nSDKWrapper :: triggerAction: Action triggerring failed, check parameters and try again");
          triggerResult = false;
        }
      }
      else {
        debugW("\nSDKWrapper :: triggerAction: Action not triggered as there is no Internet Available but saved as Offline Action");
        triggerResult = true;
      }

      //Dump actions triggered stats
      int offActionsTriggerCount = actionService->getOfflineActionsTriggerCount();
      int actionsTriggerCount = actionService->getActionsTriggerCount();
      debugI("\nSDKWrapper :: triggerAction: Number of offline actions left over: %d",actionService->getOfflineActionsCount());
      debugI("\nSDKWrapper :: triggerAction: Number of offline actions triggered: %d",offActionsTriggerCount);
      debugI("\nSDKWrapper :: triggerAction: Number of actions triggered: %d",actionsTriggerCount);
      debugI("\nSDKWrapper :: triggerAction: Number of total actions triggered since from board start: %d",actionsTriggerCount+offActionsTriggerCount);

      return triggerResult;
    }
  }
}