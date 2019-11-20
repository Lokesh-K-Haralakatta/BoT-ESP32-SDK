/*
  ActionService.cpp - Class and Methods to trigger and get actions with BoT Service
  Created by Lokesh H K, April 17, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include "ActionService.h"
#define HOST "www.google.com"

ActionService :: ActionService(){
  store = KeyStore :: getKeyStoreInstance();
  bot = BoTService :: getBoTServiceInstance();
  timeClient = new NTPClient(ntpUDP);
  presentActionTriggerTimeInSeconds = 0l;
  previousActionTriggerTimeInSeconds = 0l;
  totalActionsTrigger = 0;
  totalOfflineActionsTrigger = 0;
}

ActionService :: ~ActionService(){
  delete timeClient;
}

bool ActionService :: isInternetConnectivityAvailable(){
  return Ping.ping(HOST);
}

int ActionService :: countLeftOverOfflineActions(){
  int leftOverActions = 0;
  for (std::vector<struct OfflineActionMetadata>::iterator i = offlineActionsList.begin() ; i != offlineActionsList.end(); ++i){
    if(i->offline == 1)
      leftOverActions++;
  }
  return leftOverActions;
}

String* ActionService :: postAction(const char* actionID, const char* qID, const double value){
  //Action triggering logic goes here
  DynamicJsonBuffer jsonBuffer;
  JsonObject& doc = jsonBuffer.createObject();
  JsonObject& botData = doc.createNestedObject("bot");
  botData["deviceID"] = store->getDeviceID();
  botData["actionID"] = actionID;
  botData["queueID"] = qID;

  if (store->isDeviceMultipair()) {
    botData["alternativeID"] = store->getAlternateDeviceID();
  }

  if (value > 0.0) {
    botData["value"] = value;
  }

  char payload[200];
  doc.printTo(payload);
  jsonBuffer.clear();
  debugI("\nActionService : postAction: Minified JSON payload to trigger action: %s", payload);

  return(bot->post(ACTIONS_END_POINT,payload));
}

int ActionService :: getOfflineActionsCount(){
  offlineActionsList = store->retrieveOfflineActions();
  return offlineActionsList.size();
}

int ActionService :: getOfflineActionsTriggerCount(){
  return totalOfflineActionsTrigger;
}

int ActionService :: getActionsTriggerCount(){
  return totalActionsTrigger;
}

void ActionService :: triggerOfflineActions(){
  debugI("\nActionService: triggerOfflineActions: Processing pending offline actions");

  int offlineActionsCount = getOfflineActionsCount();

  if(offlineActionsCount > 0){
    debugI("\nActionService: triggerOfflineActions: Number of offline actions in list: %d",offlineActionsCount);
    //For each of offline action in offlineActionsList,
    //check internet connectivity and trigger pending actions
    for (std::vector<struct OfflineActionMetadata>::iterator i = offlineActionsList.begin() ; i != offlineActionsList.end(); ++i){
      if(isInternetConnectivityAvailable()){
        if(i->offline == 1){
          debugD("\nActionService: triggerOfflineActions: Triggerring pending action with actionID - %s and timestamp - %lu", i->actionID,i->timestamp);

          //Trigger Offline Action
          String* offlineResponse = postAction(i->actionID,i->queueID,i->value);

          //Post successful,
          //Turnoff offline flag for the action
          if(offlineResponse != NULL && offlineResponse->indexOf("OK") != -1){
            i->offline = 0;
            totalOfflineActionsTrigger++;
            debugI("\nActionService: triggerOfflineActions: Offline Action with actionID: %s for timestamp: %lu trigger successful",i->actionID,i->timestamp);
          }
          else {
            debugE("\nActionService: triggerOfflineActions: Offline Action - %s failed with message - %s", i->actionID,offlineResponse->c_str());
          }
        }
      }
      else {
        //No Internet connectivity, stop processing offline actions
        debugW("\nActionService: triggerOfflineActions: No Internet Connectivity available, stopping remaining offline actions to be triggered");
        break;
      }
      debugI("\nActionService: triggerOfflineActions: Still to process %d offline actions",countLeftOverOfflineActions());
     }

    //There are chances of losing internet connectivity / failure of triggering any action
    //Check whether there are any left over pending offline actions in offlineActionsList
    int leftOverOfflineActions = countLeftOverOfflineActions();
    debugI("\nActionService: triggerOfflineActions: Number of left over offline actions in offlineActionsList: %d", leftOverOfflineActions);
    //Write back such left over offline actions into storage
    if(leftOverOfflineActions > 0){
      debugI("\nActionService: triggerOfflineActions: %d offline actions not cleared, hence saving back to file - %s", leftOverOfflineActions,OFFLINE_ACTIONS_FILE);
      if(store->saveOfflineActions(offlineActionsList)){
        debugI("\nActionService: triggerOfflineActions: Left over %d Offline Actions successfully saved to %s file",leftOverOfflineActions, OFFLINE_ACTIONS_FILE);
      }
      else {
        debugE("\nActionService: triggerOfflineActions: Saving %d left over offline actions to %s failed...",leftOverOfflineActions,OFFLINE_ACTIONS_FILE);
      }
    }
    else {
      debugI("\nActionService: triggerOfflineActions: No left over Offline Actions, clearing off saved offline actions from SPIFFS");
      if(store->clearOfflineActions()){
        debugI("\nActionService: triggerOfflineActions: Cleared Offline Actions");
      }
      else {
        debugE("\nActionService: triggerOfflineActions: Failed to clear Offline Actions");
      }
    }
  }
  else {
    debugD("\nActionService: triggerOfflineActions: No Offline Actions to be processed");
  }
}

String* ActionService :: triggerOnlineAction(const char* actionID,const char* value){
  String* postResponse = NULL;
  debugI("\nActionService: triggerOnlineAction: Preparing to trigger action with actionID: %s",actionID);

  //Check for availability of internet connectivity
  //if(isInternetConnectivityAvailable()){
    //Update the latest time from network
    while(!timeClient->update()) {
      timeClient->forceUpdate();
    }

    previousActionTriggerTimeInSeconds = timeClient->getEpochTime();

    debugD("\nActionService: triggerOnlineAction: Internet connectivity available, triggering the action with actionID: %s",actionID);

    //Trigger Action
    postResponse = postAction(actionID,store->generateUuid4(),String(value).toDouble());

    //Check trigger action result
    if(postResponse->indexOf("OK") != -1){
      totalActionsTrigger++;
      debugI("\nActionService: triggerOnlineAction: Action with actionID: %s for timestamp: %lu trigger successful",actionID,previousActionTriggerTimeInSeconds);
      return postResponse;
    }
    else {
      debugE("\nActionService: triggerOnlineAction: Action with actionID: %s failed with response: %s",actionID,postResponse->c_str());
      //Trigger action failed, add as an offline action if there is no internet
      /*if(!isInternetConnectivityAvailable()) {
        debugW("\nActionService: triggerOnlineAction: adding failed action: %s to offline actions since there is no internet available",actionID);
        if(store->saveOfflineAction(actionID,value,previousActionTriggerTimeInSeconds)){
          debugI("\nActionService: triggerOnlineAction: Action - %s associated with timestamp - %lu saved as Offline Action",actionID,previousActionTriggerTimeInSeconds);
          return NULL;
        }
        else {
          debugE("\nActionService: triggerOnlineAction: Action - %s associated with timestamp - %lu failed to be saved as Offline Action",actionID,previousActionTriggerTimeInSeconds);
          return postResponse;
        }
      }*/
    }
  /*}
  else {
    debugI("\nActionService: triggerOnlineAction: Internet connectivity not available, saving the action onto storage");
    if(store->saveOfflineAction(actionID,value,previousActionTriggerTimeInSeconds)){
      debugI("\nActionService: triggerOnlineAction: Action - %s associated with timestamp - %lu saved as Offline Action",actionID,previousActionTriggerTimeInSeconds);
    }
    else {
      debugE("\nActionService: triggerOnlineAction: Action - %s failed to be saved as Offline Action",actionID);
    }
    return NULL;
  }*/
  return postResponse;
}

String* ActionService :: triggerAction(const char* aID, const char* aVal){
  char* actionID = new char[strlen(aID)+1];
  strcpy(actionID,aID);
  char* value = NULL;
  if(aVal != NULL) {
    value = new char[strlen(aVal)+1];
    strcpy(value,aVal);
  }

  debugD("\nActionService :: triggerAction: Initializing NTPClient to capture action trigger time");
  timeClient->begin();
  debugD("\nActionService :: triggerAction: Checking actionID - %s valid or not", actionID);
  presentActionTriggerTimeInSeconds = 0;
  //if(isValidAction(actionID)){
    debugD("\nActionService :: triggerAction: %s is valid actionID, trying to trigger now", actionID);
    store->initializeEEPROM();
    store->loadJSONConfiguration();

    //Process offline actions if any
    /*if(isInternetConnectivityAvailable() && store->offlineActionsExist()){
      debugI("\nActionService :: triggerAction: Internet Connectivity Available and There are some offline actions need to be processed");
      triggerOfflineActions();
    }
    else {
      debugW("\nActionService :: triggerAction: No Internet Connectivity or There are no offline actions to handle");
    }*/

    //Trigger the provided action
    String* postResponse = triggerOnlineAction(actionID,value);

   /*
    const char* deviceID = store->getDeviceID();
    debugD("\nActionService :: triggerAction: Provided deviceID : %s", deviceID);
    const char* queueID = store->generateUuid4();
    debugD("\nActionService :: triggerAction: Generated queueID : %s", queueID);

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
    debugD("\nActionService :: triggerAction: Minified JSON payload to triggerAction: %s", payload);
    jsonBuffer.clear();

    //Trigger Action
    postResponse = bot->post(ACTIONS_END_POINT,payload);
  */
    //Update the trigger time for the actionID if its success
    if((postResponse != NULL) && (postResponse->indexOf("OK") != -1)){
      debugI("\nActionService :: triggerAction: Action %s successful ",actionID);
      /* if(updateTriggeredTimeForAction(actionID)){
        debugD("\nActionService :: triggerAction: Action trigger time - %lu updated to %s",presentActionTriggerTimeInSeconds,actionID);
      }
      else {
        debugW("\nActionService :: triggerAction: Action trigger time - %lu failed to update to %s",presentActionTriggerTimeInSeconds,actionID);
      } */
    }
    else if(postResponse == NULL){
      debugW("\nActionService :: triggerAction: No Internet Connectivity, payment saved as offline action to storage");
    }
    else {
      debugE("\nActionService :: triggerAction: Failed with response - %s",postResponse->c_str());
    }

    //Save the actions present in actionsList to ACTIONS_FILE for reference
    /*if(store->saveActions(actionsList)){
      debugD("\nActionService :: triggerAction: %d actions successfully saved to file - %s",actionsList.size(),ACTIONS_FILE);
    }
    else {
      debugE("\nActionService :: triggerAction: %d actions failed to save to file - %s",actionsList.size(),ACTIONS_FILE);
    }*/

  /* }
  else {
    debugE("\nActionService :: triggerAction: %s is invalid actionID", actionID);
    response = "{\"code\": \"404\", \"message\": \"Invalid Action\"}";
  } */

  //Delete memory for action Data
  delete actionID;
  if(value != NULL) delete value;

  return postResponse;
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
    debugD("\nActionService :: updateTriggeredTimeForAction: Updating TriggeredTime for action - %s", i->actionID);
    i->triggeredTime = presentActionTriggerTimeInSeconds;
    debugD("\nActionService :: updateTriggeredTimeForAction: %s : %s : %lu", i->actionID, i->actionFrequency, i->triggeredTime);
    return true;
  }
  else {
    debugW("\nActionService :: updateTriggeredTimeForAction: Action - %s not present in actionsList", actionID);
    return false;
  }
}

void ActionService :: clearActionsList(){
  int pos = 0;
  std::vector<struct Action>::iterator i;
  while(!actionsList.empty()){
    i = actionsList.begin();
    if(i->actionID != NULL){
      delete i->actionID;
      i->actionID = NULL;
    }
    if(i->actionFrequency != NULL){
      delete i->actionFrequency;
      i->actionFrequency = NULL;
    }
    actionsList.erase(i);
    pos++;
    debugD("\nActionService :: clearActionsList : Freed memory and erased action at position - %d",pos);
  }
}

String* ActionService :: getActions(){
  String* actions = bot->get(ACTIONS_END_POINT);

  debugD("\nActionService :: getActions: %s", actions->c_str());

  if(actions->indexOf("[") != -1 && actions->indexOf("]") != -1){
    DynamicJsonBuffer jsonBuffer;
    JsonArray& actionsArray = jsonBuffer.parseArray(*actions);
    if(actionsArray.success()){
        int actionsCount = actionsArray.size();
        debugD("\nActionService :: getActions: JSON Actions array parsed successfully");
        debugD("\nActionService :: getActions: Number of actions returned: %d", actionsCount);

        //Clear off previous stored actions before processing new set
        if(!actionsList.empty()){
          clearActionsList();
          if(actionsList.empty()){
            debugD("\nActionService :: getActions: Cleared contents of previous actions present in ActionsList");
          }
          else {
            debugE("\nActionService :: getActions: Not cleared contents of previous actions present, retaining the same back");
            jsonBuffer.clear();
            return actions;
          }
        }

        //Process the new set of actions and add to actionsList
        for(byte i=0 ; i < actionsCount; i++){
           const char* actionID = actionsArray[i]["actionID"];
           const char* frequency = actionsArray[i]["frequency"];
           debugD("\nID: %s  Frequency: %s", actionID, frequency);
           struct Action actionItem;
           actionItem.actionID = new char[strlen(actionID)+1];
           actionItem.actionFrequency = new char[strlen(frequency)+1];
           actionItem.triggeredTime = 0;
           strcpy(actionItem.actionID,actionID);
           strcpy(actionItem.actionFrequency,frequency);
           actionsList.push_back(actionItem);
        }

        debugI("\nActionService :: getActions: Added %d actions returned from server into actionsList", actionsList.size());
        jsonBuffer.clear();
        return actions;
    }
    else {
      debugE("\nActionService :: getActions: JSON Actions array parsed failed!");
      debugW("\nActionService :: getActions: use locally stored actions, if available");
      jsonBuffer.clear();
      localActionsList = store->retrieveActions();
      debugW("\nActionService :: getActions: Local actions count: %d", localActionsList.size());
      return NULL;
    }
  }
  else {
    debugE("\nActionService :: getActions: Could not retrieve actions from server");
    debugW("\nActionService :: getActions: use locally stored actions, if available");
    localActionsList = store->retrieveActions();
    debugW("\nActionService :: getActions: Local actions count: %d", localActionsList.size());
    return NULL;
  }
}

void ActionService :: updateActionsLastTriggeredTime(){
  //Get Saved Actions from KeyStore
  std::vector <struct Action> savedActionsList = store->retrieveActions();
  if(savedActionsList.empty()){
    debugD("\nActionService :: updateActionsLastTriggeredTime: Zero saved actions, no need to update any triggered time");
    return;
  }
  else {
    debugD("\nActionService :: updateActionsLastTriggeredTime: There are %d saved actions retrieved from file - %s",savedActionsList.size(),ACTIONS_FILE);
    for (std::vector<struct Action>::iterator i = actionsList.begin() ; i != actionsList.end(); ++i){
      debugD("\nActionService :: updateActionsLastTriggeredTime: %s : %s : %lu", i->actionID, i->actionFrequency, i->triggeredTime);
      std::vector<struct Action>::iterator j = find_if(savedActionsList.begin(), savedActionsList.end(),
                                                        [i](const struct Action& k) {
                                                            return (strcmp(i->actionID, k.actionID) == 0); });

      if(j != savedActionsList.end()){
        debugD("\nActionService :: updateActionsLastTriggeredTime: Updating lastTriggeredTime for action - %s", j->actionID);
        i->triggeredTime = j->triggeredTime;
        debugD("\nActionService :: updateActionsLastTriggeredTime: %s : %s : %lu", i->actionID, i->actionFrequency, i->triggeredTime);
      }
      else {
        debugD("\nActionService :: updateActionsLastTriggeredTime: Action - %s not present in savedActionsList", i->actionID);
      }
    }
  }

}

bool ActionService :: isValidAction(const char* actionID){
  //Get fresh list of actions from server
  String* actions = getActions();

  //Update lastTriggeredTime for actions from saved details if actions successfully retrieved from BoT Server
  if(actions != NULL){
    debugD("\nActionService :: isValidAction: Actions retrieved from BoT Server, calling updateActionsLastTriggeredTime");
    updateActionsLastTriggeredTime();
  }
  else {
    debugW("\nActionService :: isValidAction: Actions not retrieved from BoT Server, no need to update actions last triggered time");
  }

  //Check existence of given action in the actions list
  bool actionIDExists = false;
  struct Action actionItem;
  for (auto i = actionsList.begin(); i != actionsList.end(); ++i)
    if(strcmp((*i).actionID,actionID) == 0){
      actionIDExists = true;
      actionItem = (*i);
      debugD("\nActionService :: isValidAction: Action - %s present in retrieved actions from server, lastTriggeredTime: %lu",
                    actionItem.actionID, actionItem.triggeredTime);
      break;
    }
  return (actionIDExists && isValidActionFrequency(&actionItem));
}

bool ActionService :: isValidActionFrequency(const struct Action* pAction){
  //Get last triggered time for the given action
  unsigned long lastTriggeredAt = pAction->triggeredTime;
  debugD("\nActionService :: isValidActionFrequency: Action - %s last triggered time in seconds - %lu",pAction->actionID,lastTriggeredAt);
  if (lastTriggeredAt == -1) {
      return true;
  }

  while(!timeClient->update()) {
    timeClient->forceUpdate();
  }

  debugD("\nActionService :: isValidActionFrequency: Action - %s frequency is %s",pAction->actionID,pAction->actionFrequency);
  debugD("\nActionService :: isValidActionFrequency: lastTriggeredTime: %lu", lastTriggeredAt);
  presentActionTriggerTimeInSeconds = timeClient->getEpochTime();
  debugD("\nActionService :: isValidActionFrequency: presentTime: %lu", presentActionTriggerTimeInSeconds);
  unsigned int secondsSinceLastTriggered = presentActionTriggerTimeInSeconds - lastTriggeredAt;
  debugD("\nActionService :: isValidActionFrequency: secondsSinceLastTriggered: %d", secondsSinceLastTriggered);

  if(strcmp(pAction->actionFrequency,"minutely") == 0){
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
  else if(strcmp(pAction->actionFrequency,"half_yearly") == 0){
    return secondsSinceLastTriggered > HALF_YEAR_IN_SECONDS;
  }
  else if(strcmp(pAction->actionFrequency,"yearly") == 0){
    return secondsSinceLastTriggered > YEAR_IN_SECONDS;
  }
  else if(strcmp(pAction->actionFrequency,"always") == 0){
    return true;
  }
  else {
    return false;
  }
}
