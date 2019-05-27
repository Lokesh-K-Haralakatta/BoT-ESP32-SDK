/*
  ActionService.cpp - Class and Methods to trigger and get actions with BoT Service
  Created by Lokesh H K, April 17, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include "ActionService.h"

ActionService :: ActionService(){
  store = KeyStore :: getKeyStoreInstance();
  timeClient = new NTPClient(ntpUDP);
}

const char* ActionService ::generateUuid4() {
  uint8_t uuid[16];
  String* uuidStr = new String();

  // Generate a Version 4 UUID according to RFC4122
  for (int i=0;i<16;i++) uuid[i] = esp_random();

  // Although the UUID contains 128 bits, only 122 of those are random.
  // The other 6 bits are fixed, to indicate a version number.
  uuid[6] = 0x40 | (0x0F & uuid[6]);
  uuid[8] = 0x80 | (0x3F & uuid[8]);

  //Convert generated uuid to string format
  for (int i=0; i<16; i++) {
    if (i==4) *uuidStr += "-";
    if (i==6) *uuidStr += "-";
    if (i==8) *uuidStr += "-";
    if (i==10) *uuidStr += "-";
    int topDigit = uuid[i] >> 4;
    int bottomDigit = uuid[i] & 0x0f;
    // High hex digit
    *uuidStr += "0123456789abcdef"[topDigit];
    // Low hex digit
    *uuidStr += "0123456789abcdef"[bottomDigit];
  }

  LOG("\nActionService :: generateuuid4 : %s", uuidStr->c_str());

  return uuidStr->c_str();
}

String ActionService :: triggerAction(const char* actionID, const char* value, const char* altID){
  String response = "";
  LOG("\nActionService :: triggerAction: Initializing NTPClient to capture action trigger time");
  timeClient->begin();
  LOG("\nActionService :: triggerAction: Checking actionID - %s valid or not", actionID);
  presentActionTriggerTimeInSeconds = 0;
  if(isValidAction(actionID)){
    LOG("\nActionService :: triggerAction: %s is valid actionID, trying to trigger now", actionID);
    store->initializeEEPROM();
    store->loadJSONConfiguration();

    const char* deviceID = store->getDeviceID();
    LOG("\nActionService :: triggerAction: Provided deviceID : %s", deviceID);
    const char* queueID = generateUuid4();
    LOG("\nActionService :: triggerAction: Generated queueID : %s", queueID);

    DynamicJsonBuffer jsonBuffer;
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

    BoTService *bot = new BoTService();
    response = bot->post(ACTIONS_END_POINT,payload);
    delete bot;

    //Update the trigger time for the actionID
    if(updateTriggeredTimeForAction(actionID)){
      LOG("\nActionService :: triggerAction: Action trigger time - %lu updated to %s",presentActionTriggerTimeInSeconds,actionID);
    }
    else {
      LOG("\nActionService :: triggerAction: Action trigger time - %lu failed to update to %s",presentActionTriggerTimeInSeconds,actionID);
    }
    //Save the actions present in actionsList to ACTIONS_FILE for reference
    if(store->saveActions(actionsList)){
      LOG("\nActionService :: triggerAction: %d actions successfully saved to file - %s",actionsList.size(),ACTIONS_FILE);
    }
    else {
      LOG("\nActionService :: triggerAction: %d actions failed to save to file - %s",actionsList.size(),ACTIONS_FILE);
    }
  }
  else {
    LOG("\nActionService :: triggerAction: %s is invalid actionID", actionID);
    response = "{\"code\": \"404\", \"message\": \"Invalid Action\"}";
  }
  return response;
}

bool ActionService :: updateTriggeredTimeForAction(const char* actionID){
  //Find iterator to action item with the given actionID to be updated
  struct Action x;
  x.actionID = new char[strlen(actionID)+1];
  strcpy(x.actionID,actionID);
  std::vector<struct Action>::iterator i = find_if(actionsList.begin(), actionsList.end(),
                                                    [x](const struct Action& y) {
                                                      return (strcmp(x.actionID, y.actionID) == 0); });
  delete x.actionID;

  if(i != actionsList.end()){
    LOG("\nActionService :: updateTriggeredTimeForAction: Updating TriggeredTime for action - %s", i->actionID);
    i->triggeredTime = presentActionTriggerTimeInSeconds;
    LOG("\nActionService :: updateTriggeredTimeForAction: %s : %s : %lu", i->actionID, i->actionFrequency, i->triggeredTime);
    return true;
  }
  else {
    LOG("\nActionService :: updateTriggeredTimeForAction: Action - %s not present in actionsList", actionID);
    return false;
  }
}

String ActionService :: getActions(){
  BoTService *bot = new BoTService();
  String actions = bot->get(ACTIONS_END_POINT);
  delete bot;

  LOG("\nActionService :: getActions: %s", actions.c_str());

  if(!actionsList.empty()){
    actionsList.clear();
    LOG("\nActionService :: getActions: cleared contents of previous actions");
  }

  if(actions.indexOf("[") != -1 && actions.indexOf("]") != -1){
    DynamicJsonBuffer jsonBuffer;
    JsonArray& actionsArray = jsonBuffer.parseArray(actions);
    if(actionsArray.success()){
        int actionsCount = actionsArray.size();
        LOG("\nActionService :: getActions: JSON Actions array parsed successfully");
        LOG("\nActionService :: getActions: Number of actions returned: %d", actionsCount);

        for(byte i=0 ; i < actionsCount; i++){
           const char* actionID = actionsArray[i]["actionID"];
           const char* frequency = actionsArray[i]["frequency"];
           LOG("\nID: %s  Frequency: %s", actionID, frequency);
           struct Action actionItem;
           actionItem.actionID = new char[strlen(actionID)+1];
           actionItem.actionFrequency = new char[strlen(frequency)+1];
           actionItem.triggeredTime = 0;
           strcpy(actionItem.actionID,actionID);
           strcpy(actionItem.actionFrequency,frequency);
           actionsList.push_back(actionItem);
        }
        LOG("\nActionService :: getActions: Added %d actions returned from server into actionsList", actionsList.size());
        return actions;
    }
    else {
      LOG("\nActionService :: getActions: JSON Actions array parsed failed!");
      LOG("\nActionService :: getActions: use locally stored actions, if available");
      actionsList = store->retrieveActions();
      return "";
    }
  }
  else {
    LOG("\nActionService :: getActions: Could not retrieve actions from server");
    LOG("\nActionService :: getActions: use locally stored actions, if available");
    actionsList = store->retrieveActions();
    return "";
  }
}

void ActionService :: updateActionsLastTriggeredTime(){
  //Get Saved Actions from KeyStore
  std::vector <struct Action> savedActionsList = store->retrieveActions();
  if(savedActionsList.empty()){
    LOG("\nActionService :: updateActionsLastTriggeredTime: Zero saved actions, no need to update any triggered time");
    return;
  }
  else {
    LOG("\nActionService :: updateActionsLastTriggeredTime: There are %d saved actions retrieved from file - %s",savedActionsList.size(),ACTIONS_FILE);
    for (std::vector<struct Action>::iterator i = actionsList.begin() ; i != actionsList.end(); ++i){
      LOG("\nActionService :: updateActionsLastTriggeredTime: %s : %s : %lu", i->actionID, i->actionFrequency, i->triggeredTime);
      std::vector<struct Action>::iterator j = find_if(savedActionsList.begin(), savedActionsList.end(),
                                                        [i](const struct Action& k) {
                                                            return (strcmp(i->actionID, k.actionID) == 0); });

      if(j != savedActionsList.end()){
        LOG("\nActionService :: updateActionsLastTriggeredTime: Updating lastTriggeredTime for action - %s", j->actionID);
        i->triggeredTime = j->triggeredTime;
        LOG("\nActionService :: updateActionsLastTriggeredTime: %s : %s : %lu", i->actionID, i->actionFrequency, i->triggeredTime);
      }
      else {
        LOG("\nActionService :: updateActionsLastTriggeredTime: Action - %s not present in savedActionsList", i->actionID);
      }
    }
  }

}

bool ActionService :: isValidAction(const char* actionID){
  //Get fresh list of actions from server
  String actions = getActions();

  //Update lastTriggeredTime for actions from saved details if actions successfully retrieved from BoT Server
  if(!actions.equals("")){
    LOG("\nActionService :: isValidAction: Actions retrieved from BoT Server, calling updateActionsLastTriggeredTime");
    updateActionsLastTriggeredTime();
  }
  else {
    LOG("\nActionService :: isValidAction: Actions not retrieved from BoT Server, no need to update actions last triggered time");
  }

  //Check existence of given action in the actions list
  bool actionIDExists = false;
  struct Action actionItem;
  for (auto i = actionsList.begin(); i != actionsList.end(); ++i)
    if(strcmp((*i).actionID,actionID) == 0){
      actionIDExists = true;
      actionItem = (*i);
      LOG("\nActionService :: isValidAction: Action - %s present in retrieved actions from server, lastTriggeredTime: %lu",
                    actionItem.actionID, actionItem.triggeredTime);
      break;
    }
  return (actionIDExists && isValidActionFrequency(&actionItem));
}

bool ActionService :: isValidActionFrequency(const struct Action* pAction){
  //Get last triggered time for the given action
  unsigned long lastTriggeredAt = pAction->triggeredTime;
  LOG("\nActionService :: isValidActionFrequency: Action - %s last triggered time in seconds - %lu",pAction->actionID,lastTriggeredAt);
  if (lastTriggeredAt == -1) {
      return true;
  }

  while(!timeClient->update()) {
    timeClient->forceUpdate();
  }

  LOG("\nActionService :: isValidActionFrequency: Action - %s frequency is %s",pAction->actionID,pAction->actionFrequency);
  LOG("\nActionService :: isValidActionFrequency: lastTriggeredTime: %lu", lastTriggeredAt);
  presentActionTriggerTimeInSeconds = timeClient->getEpochTime();
  LOG("\nActionService :: isValidActionFrequency: presentTime: %lu", presentActionTriggerTimeInSeconds);
  unsigned int secondsSinceLastTriggered = presentActionTriggerTimeInSeconds - lastTriggeredAt;
  LOG("\nActionService :: isValidActionFrequency: secondsSinceLastTriggered: %d", secondsSinceLastTriggered);

  if(strcmp(pAction->actionFrequency,"minutely") == 0){
    LOG("\nActionService :: isValidActionFrequency: %s frequency matched for Minutely",pAction->actionID);
    return secondsSinceLastTriggered > MINUTE_IN_SECONDS;
  }
  else if(strcmp(pAction->actionFrequency,"hourly") == 0){
    LOG("\nActionService :: isValidActionFrequency: %s frequency matched for Hourly",pAction->actionID);
    return secondsSinceLastTriggered > HOUR_IN_SECONDS;
  }
  else if(strcmp(pAction->actionFrequency,"daily") == 0){
    LOG("\nActionService :: isValidActionFrequency: %s frequency matched for Daily",pAction->actionID);
    return secondsSinceLastTriggered > DAY_IN_SECONDS;
  }
  else if(strcmp(pAction->actionFrequency,"weekly") == 0){
    LOG("\nActionService :: isValidActionFrequency: %s frequency matched for Weekly",pAction->actionID);
    return secondsSinceLastTriggered > WEEK_IN_SECONDS;
  }
  else if(strcmp(pAction->actionFrequency,"monthly") == 0){
    LOG("\nActionService :: isValidActionFrequency: %s frequency matched for Monthly",pAction->actionID);
    return secondsSinceLastTriggered > MONTH_IN_SECONDS;
  }
  else if(strcmp(pAction->actionFrequency,"half_yearly") == 0){
    LOG("\nActionService :: isValidActionFrequency: %s frequency matched for Half-Yearly",pAction->actionID);
    return secondsSinceLastTriggered > HALF_YEAR_IN_SECONDS;
  }
  else if(strcmp(pAction->actionFrequency,"yearly") == 0){
    LOG("\nActionService :: isValidActionFrequency: %s frequency matched for Yearly",pAction->actionID);
    return secondsSinceLastTriggered > YEAR_IN_SECONDS;
  }
  else if(strcmp(pAction->actionFrequency,"always") == 0){
    LOG("\nActionService :: isValidActionFrequency: %s frequency matched for Always",pAction->actionID);
    return true;
  }
  else {
    LOG("\nActionService :: isValidActionFrequency: %s frequency matched for None",pAction->actionID);
    return false;
  }
}
