/*
  ControllerService.cpp - Class and Methods to interface between Webserver End Points
                          and the backend service components
  Created by Lokesh H K, April 24, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include "ControllerService.h"
ActionService* ControllerService :: actionService;

ControllerService :: ControllerService(){
  store = KeyStore :: getKeyStoreInstance();
}

ActionService* ControllerService :: getActionServiceObject(){
  if(actionService == NULL){
      actionService = new ActionService();
      debugI("\nControllerService :: getActionServiceObject: Instantiated ActionService Instance");
  }

  return actionService;
}

void ControllerService :: getActions(AsyncWebServerRequest *request){
  ActionService* actionService = ControllerService :: getActionServiceObject();
  String* response = actionService->getActions();
  //delete actionService;

  if(response == NULL){
    DynamicJsonBuffer jsonBuffer;
    JsonObject& doc = jsonBuffer.createObject();
    doc["message"] = "Unable to retrieve actions";
    char body[100];
    doc.printTo(body);
    jsonBuffer.clear();
    debugE("\nControllerService :: getActions: %s", body);
    request->send(503, "application/json", body);
  }
  else {
    const char* responseString = response->c_str();
    debugD("\nControllerService :: getActions: %s", responseString);
    request->send(200, "application/json", responseString);
  }
}

void ControllerService :: postAction(AsyncWebServerRequest *request){
    debugI("\nControllerService :: postAction: Request received to trigger an action");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& doc = jsonBuffer.createObject();
    char body[100];

    int paramsCount = request->params();
    if(paramsCount == 0){
      debugE("\nControllerService :: postAction: Need actionID as query parameter to trigger an action");
      doc["message"] = "Need actionID as query parameter to trigger an action";
      doc.printTo(body);
      jsonBuffer.clear();
      request->send(400,"application/json", body);
    }
    else if(store->getDeviceState() < DEVICE_ACTIVE){
      doc["message"] = "Device not activated";
      doc.printTo(body);
      jsonBuffer.clear();
      debugE("\nControllerService :: postAction: %s", body);
      request->send(400, "application/json", body);
    }
    else {
      AsyncWebParameter* p = request->getParam(0);
      if(strcmp(p->name().c_str(), "actionID") != 0){
        debugE("\nControllerService :: postAction: Query Parameter should be `actionID`");
        doc["message"] = "Query Parameter should be `actionID`";
        doc.printTo(body);
        jsonBuffer.clear();
        request->send(400,"application/json", body);
      }
      else if(store->isDeviceMultipair() &&
        store->getAlternateDeviceID() == NULL){
        doc["message"] = "Missing parameter `AlternativeID`";
        doc.printTo(body);
        jsonBuffer.clear();
        debugE("\nControllerService :: postAction: %s", body);
        request->send(400, "application/json", body);
      }
      else {
        const char* actionID = p->value().c_str();
        if(actionID != NULL && strlen(actionID) > 0){
          debugI("\nControllerService :: postAction: Given actionID: %s",actionID);
          const int responseCode = triggerAction(actionID);
          switch(responseCode) {
            case 200 : debugI("\nControllerService :: postAction: Action triggered successful");
                       doc["message"] = "Action triggered successful";
                       break;
            case 201 : debugI("\nControllerService :: postAction: Action saved as Offline Action");
                       doc["message"] = "Action saved as Offline Action";
                       break;
            default  : debugE("\nControllerService :: postAction: Check parameters and try again");
                       doc["message"] = "Check parameters and try again";
                       break;
          }
          doc.printTo(body);
          jsonBuffer.clear();
          request->send(responseCode, "application/json", body);
        }
        else {
          debugE("\nControllerService :: postAction: actionID cannot be NULL");
          doc["message"] = "actionID cannot be NULL";
          doc.printTo(body);
          jsonBuffer.clear();
          request->send(400,"application/json", body);
        }
      }
    }
}

int ControllerService :: triggerAction(const char* actionID){
  ActionService* actionService = ControllerService :: getActionServiceObject();
  String* response = actionService->triggerAction(actionID);
  int responseCode = 400;
  if(response == NULL){
    debugI("\nControllerService :: triggerAction: Action saved as offline action");
    responseCode = 201;
  }
  else if(response->indexOf("OK") != -1){
    debugI("\nControllerService :: triggerAction: triggerAction Response: %s", response->c_str());
    responseCode = 200;
  }
  else {
    debugI("\nControllerService :: triggerAction: triggerAction Response: %s", response->c_str());
    responseCode = 404;
  }

  //Dump actions triggered stats
  int offActionsTriggerCount = actionService->getOfflineActionsTriggerCount();
  int actionsTriggerCount = actionService->getActionsTriggerCount();
  debugI("\nControllerService :: postAction: Number of offline actions left over: %d",actionService->getOfflineActionsCount());
  debugI("\nControllerService :: postAction: Number of offline actions triggered: %d",offActionsTriggerCount);
  debugI("\nControllerService :: postAction: Number of actions triggered: %d",actionsTriggerCount);
  debugI("\nControllerService :: postAction: Number of total actions triggered since from board start: %d",actionsTriggerCount+offActionsTriggerCount);

  return responseCode;
}

void ControllerService :: getQRCode(AsyncWebServerRequest *request){
  //Check QR Code generation qrCodeStatus
  bool qrCodeStatus = store->isQRCodeGeneratedandSaved();
  if(!qrCodeStatus){
    debugD("\nControllerService :: getQRCode: Generating QR Code and saving to SPIFFS");
    qrCodeStatus = store->generateAndSaveQRCode();
  }
  if(qrCodeStatus){
    debugI("\nControllerService :: getQRCode: QR Code exists on SPIFFS, serving through webresponse");
    request->send(SPIFFS,QRCODE_FILE,"image/svg+xml");
  }
  else{
    debugE("\nControllerService :: getQRCode: QR Code not available on SPIFFS, returning 404 as web response");
    request->send(404,"text/plain","QR Code not available on SPIFFS");
  }
}

void ControllerService :: pairDevice(AsyncWebServerRequest *request){
  store->initializeEEPROM();
  DynamicJsonBuffer jsonBuffer;
  JsonObject& doc = jsonBuffer.createObject();
  char body[100];

  if(store->getDeviceState() > DEVICE_NEW){
    doc["message"] = "Device is already paired";
    doc.printTo(body);
    jsonBuffer.clear();
    debugW("\nControllerService :: pairDevice: %s", body);
    request->send(403, "application/json", body);
  }
  else {
    PairingService* pairService = new PairingService();
    pairService->pairDevice();
    delete pairService;

    int deviceState = store->getDeviceState();
    debugD("\nControllerService :: pairDevice: Device state after return from pairService->pairDevice() : %d",deviceState);
    if( deviceState != DEVICE_NEW){
      doc["message"] = "Device pairing successful";
      doc.printTo(body);
      jsonBuffer.clear();
      debugD("\nControllerService :: pairDevice: %s", body);
      request->send(200, "application/json", body);
    }
    else {
      doc["message"] = "Unable to pair device";
      doc.printTo(body);
      jsonBuffer.clear();
      debugE("\nControllerService :: pairDevice: %s", body);
      request->send(503, "application/json", body);
    }
  }
}
