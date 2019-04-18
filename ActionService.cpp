/*
  ActionService.cpp - Class and Methods to trigger and get actions with BoT Service
  Created by Lokesh H K, April 17, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include "ActionService.h"

ActionService :: ActionService(){
  bot = new BoTService();
  store = KeyStore :: getKeyStoreInstance();
  timeClient = new NTPClient(ntpUDP);
  timeClient->begin();
}

String ActionService :: triggerAction(const char* actionID, const char* value, const char* altID){
  String response = "";
  if(isValidAction(actionID)){
    store->initializeEEPROM();
    store->loadJSONConfiguration();

    const char* deviceID = store->getDeviceID();
    const char* queueID = store->getQueueID();

    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& doc = jsonBuffer.createObject();
    JsonObject& botData = doc.createNestedObject("bot");
    botData["deviceID"] = deviceID;
    botData["actionID"] = actionID;
    botData["queueID"] = queueID;

    if (store->getDeviceState() == DEVICE_MULTIPAIR) {
      botData["alternativeID"] = altID;
    }

    if (value != NULL && strlen(value)>0) {
      botData["value"] = value;
    }

    char payload[200];
    doc.printTo(payload);
    LOG("\nActionService :: triggerAction: Minified JSON payload to triggerAction: %s", payload);
    response = bot->post(ACTIONS_END_POINT,payload);
    LOG("\nActionService :: triggerAction: %s is valid actionID", actionID);

    //TODO: save complete actions details into SPIFFS for future reference
  }
  else {
    LOG("\nActionService :: triggerAction: %s is invalid actionID", actionID);
    response = "{\"code\": \"404\", \"message\": \"Action not found\"}";
  }
  return response;
}

String ActionService :: getActions(){
  String actions = bot->get(ACTIONS_END_POINT);
  if(actions.indexOf("[") != -1 && actions.indexOf("]") != -1){
    DynamicJsonBuffer jsonBuffer;
    JsonArray& actionsArray = jsonBuffer.parseArray(actions);
    if(actionsArray.success()){
        int actionsCount = actionsArray.size();
        LOG("\nActionService :: getActions: JSON Actions array parsed successfully");
        LOG("\nActionService :: getActions: Number of actions returned: %d", actionsCount);

        if(!actionsList.empty()){
          actionsList.clear();
          LOG("\nActionService :: getActions: cleared contents of previous actions");
        }

        for(byte i=0 ; i < actionsCount; i++){
           const char* actionID = actionsArray[i]["actionID"];
           const char* frequency = actionsArray[i]["frequency"];
           LOG("\nID: %s  Frequency: %s", actionID, frequency);
           struct Action actionItem;
           actionItem.actionID = new char[strlen(actionID)+1];
           actionItem.actionFrequency = new char[strlen(frequency)+1];
           actionItem.triggeredTime = -1;
           strcpy(actionItem.actionID,actionID);
           strcpy(actionItem.actionFrequency,frequency);
           actionsList.push_back(actionItem);
        }
        LOG("\nActionService :: getActions: saved %d fresh actions returned from server", actionsList.size());
        return actions;
    }
    else {
      LOG("\nActionService :: getActions: JSON Actions array parsed failed!");
      LOG("\nActionService :: getActions: use locally stored actions, if available");
      //To Do: Read actions from file and load into actionsList if present
      return "";
    }
  }
  else {
    LOG("\nActionService :: getActions: Could not retrieve actions from server");
    LOG("\nActionService :: getActions: use locally stored actions, if available");
    //To Do: Read actions from file and load into actionsList if present
    return "";
  }
}

bool ActionService :: isValidAction(const char* actionID){
  //Get fresh list of actions from server
  getActions();

  //Update lastTriggeredTime for actions from saved details
  //TODO: updateActionsLastTriggeredTime();

  //Check existence of given action in the actions list
  bool actionIDExists = false;
  struct Action actionItem;
  for (auto i = actionsList.begin(); i != actionsList.end(); ++i)
    if(strcmp((*i).actionID,actionID) == 0){
      actionIDExists = true;
      actionItem = (*i);
      break;
    }
  return (actionIDExists && isValidActionFrequency(&actionItem));
}

bool ActionService :: isValidActionFrequency(const struct Action* pAction){
  //Get last triggered time for the given action
  unsigned long lastTriggeredAt = pAction->triggeredTime;
  if (lastTriggeredAt == -1) {
      return true;
  }

  while(!timeClient->update()) {
    timeClient->forceUpdate();
  }

  LOG("\nActionService :: isValidActionFrequency: lastTriggeredTime: %l", lastTriggeredAt);
  unsigned long presentTime = timeClient->getEpochTime();
  LOG("\nActionService :: isValidActionFrequency: presentTime: %l", presentTime);
  unsigned int secondsSinceLastTriggered = presentTime - lastTriggeredAt;
  LOG("\nActionService :: isValidActionFrequency: secondsSinceLastTriggered: %d", secondsSinceLastTriggered);

  if(strcmp(pAction->actionFrequency,"minuetly") == 0){
    return secondsSinceLastTriggered > MINUTE_IN_SECONDS;
  }
  else if(strcmp(pAction->actionFrequency,"hourly") == 0){
    return secondsSinceLastTriggered > HOUR_IN_SECONDS;
  }
  else if(strcmp(pAction->actionFrequency,"daily") == 0){
    return secondsSinceLastTriggered > DAY_IN_SECONDS;
  }
  else if(strcmp(pAction->actionFrequency,"weekly") == 0){
    return secondsSinceLastTriggered > WEEK_IN_SECONDS;
  }
  else if(strcmp(pAction->actionFrequency,"monthly") == 0){
    return secondsSinceLastTriggered > MONTH_IN_SECONDS;
  }
  else if(strcmp(pAction->actionFrequency,"half-yearly") == 0){
    return secondsSinceLastTriggered > HALF_YEAR_IN_SECONDS;
  }
  else if(strcmp(pAction->actionFrequency,"half-yearly") == 0){
    return secondsSinceLastTriggered > YEAR_IN_SECONDS;
  }
  else {
    return true;
  }
}
