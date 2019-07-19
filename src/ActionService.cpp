/*
  ActionService.cpp - Class and Methods to trigger and get actions with BoT Service
  Created by Lokesh H K, April 17, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include "ActionService.h"
#define HOST "www.google.com"

//Vector to represent list of offline actions
std::vector <struct OfflineActionMetadata> offlineActionsList;

//Tasks handles
TaskHandle_t offlineActionTask;
TaskHandle_t paymentTask;

//Flags to synchronize between OfflineTask and PaymentTask
bool offlineTaskRunning = false;
bool paymentTaskRunning = false;

//String to hold post operation Response
String* postResponse = NULL;

//Function to check Internet Connectivity Available or Not
bool isInternetConnectivityAvailable(){
  return Ping.ping(HOST);
}

//Function to release memory for offline actions
void clearOfflineActionsList(){
  int pos = 0;
  std::vector<struct OfflineActionMetadata>::iterator i;
  while(!offlineActionsList.empty()){
    i = offlineActionsList.begin();
    delete i->actionID;
    delete i->deviceID;
    delete i->makerID;
    delete i->queueID;
    if(i->alternateID != NULL){
      delete i->alternateID;
    }
    offlineActionsList.erase(i);
    pos++;
    debugI("\nActionService: clearOfflineActionsList: Freed memory and erased offline action at position - %d",pos);
  }
}

//Function to count payments with offline set to 1
int countLeftOverPayments(){
  int leftOverPayments = 0;
  for (std::vector<struct OfflineActionMetadata>::iterator i = offlineActionsList.begin() ; i != offlineActionsList.end(); ++i){
    if(i->offline == 1)
      leftOverPayments++;
  }
  return leftOverPayments;
}

//Task function to trigger offline saved payments
void triggerOfflineActions( void * pvParameters ){
  debugI("\nActionService: offlinePaymentTask: Offline Actions Task running on core: %d",xPortGetCoreID());
  debugI("\nActionService: offlinePaymentTask: Available free heap at the beginning of triggerOfflineActions: %lu",ESP.getFreeHeap());

  offlineTaskRunning = true;

  while(paymentTaskRunning){
    debugI("\nActionService: offlinePaymentTask: Payment task running, going to sleep for some time...");
    delay(1000);
  }

  if(!paymentTaskRunning){
    //Get references to Store and BoT Instances
    KeyStore* store = KeyStore :: getKeyStoreInstance();
    BoTService* bot = BoTService :: getBoTServiceInstance();

    //Logic to read offline actions saved in FS goes here
    offlineActionsList = store->retrieveOfflineActions(true);
    int offlinePaymentsCount = offlineActionsList.size();
    debugI("\nActionService: offlinePaymentTask: Number of pending payments in list: %d",offlinePaymentsCount);

  if(offlinePaymentsCount > 0){
    //For each of offline action in offlineActionsList,
    //check internet connectivity and trigger pending payments
    for (std::vector<struct OfflineActionMetadata>::iterator i = offlineActionsList.begin() ; i != offlineActionsList.end(); ++i){
      if(isInternetConnectivityAvailable()){
        if(i->offline == 1){
          debugI("\nActionService: offlinePaymentTask: Triggerring pending payment with actionID - %s and timestamp - %lu", i->actionID,i->timestamp);
          //Payment triggering logic goes here
          DynamicJsonBuffer jsonBuffer;
          JsonObject& doc = jsonBuffer.createObject();
          JsonObject& botData = doc.createNestedObject("bot");
          botData["deviceID"] = i->deviceID;
          botData["actionID"] = i->actionID;
          botData["queueID"] = i->queueID;

          if (store->isDeviceMultipair()) {
            botData["alternativeID"] = i->alternateID;
          }

          /*if (value != NULL && strlen(value)>0) {
            botData["value"] = value;
          }*/

          char payload[200];
          doc.printTo(payload);
          jsonBuffer.clear();
          debugI("\nActionService : offlinePaymentTask: Minified JSON payload to trigger pending payment: %s", payload);

          //Trigger Action
          String* offlineResponse = bot->post(ACTIONS_END_POINT,payload);

          //Payment successful,
          //Turnoff offline flag for the action
          if(offlineResponse->indexOf("OK") != -1){
            i->offline = 0;
            debugI("\nActionService: offlineActionTask: Pending Payment with actionID: %s for timestamp: %lu trigger successful",i->actionID,i->timestamp);
          }
          else {
            debugE("\nActionService: offlineActionTask: Pending payment - %s failed with message - %s", i->actionID,postResponse->c_str());
          }
        }
      }
      else {
        //No Internet connectivity, stop payments till next chance for the task to run
        debugI("\nActionService: offlineActionTask: No Internet Connectivity available, stopping remaining offline payments");
        break;
      }
      debugI("\nActionService: offlineActionTask: Still to process %d offline actions",countLeftOverPayments());
     }
    //There are chances of losing internet connectivity / failure of triggering any payments
    //Check whether there are any left over pending payments in offlineActionsList
    int leftOverPPs = countLeftOverPayments();
    debugI("\nActionService: offlinePaymentTask: Number of left over pending payments in offlineActionsList: %d", leftOverPPs);
    //Write back such left over pending payments into storage
    if(leftOverPPs>0){
      debugI("\nActionService: offlinePaymentTask: %d pending payments not cleared, hence saving back to file - %s", leftOverPPs,OFFLINE_ACTIONS_FILE);
      if(store->saveOfflineActions(offlineActionsList)){
        debugI("\nActionService: offlinePaymentTask: Left over %d Pending payments successfully saved to %s file",leftOverPPs, OFFLINE_ACTIONS_FILE);
      }
      else {
        debugE("\nActionService: offlinePaymentTask: Saving %d left over pending payments to %s failed...",leftOverPPs,OFFLINE_ACTIONS_FILE);
      }
      //Release memory for left over pending payments
      //clearOfflineActionsList();
    }
  }
  else {
    debugI("\nActionService: offlinePaymentTask: No Offline Tasks to be processed");
  }
 }
 else {
  debugI("\nActionService: paymentTask is active, skipping offlinePaymentsTask!!!");
 }

  debugI("\nActionService: Terminating the triggerOfflineActions task");
  debugI("\nActionService: Available free heap at the end of triggerOfflineActions: %lu",ESP.getFreeHeap());
  //Turnoff offlineActionTask
  offlineTaskRunning = false;
  //Terminate the task
  vTaskDelete(NULL);
}

void triggerPayment( void * pvParameters ){
  debugI("\nActionService: Payment Task running on core: %d",xPortGetCoreID());
  debugI("\nActionService: Available free heap at the beginning of triggerPaymentTask: %lu",ESP.getFreeHeap());

  paymentTaskRunning = true;

  while(offlineTaskRunning){
    debugI("\nActionService: PaymentTask: Offline Payment task running, going to sleep for some time...");
    delay(1000);
  }

  if(!offlineTaskRunning){
    //Get references to Store and BoT Instances
    KeyStore* store = KeyStore :: getKeyStoreInstance();
    BoTService* bot = BoTService :: getBoTServiceInstance();

    //Assume this is the time to trigger an payment
    unsigned long paymentTime = millis();

    //Get details for the payment
    const char* actionID = (char*)pvParameters;
    const char* deviceID = store->getDeviceID();
    const char* makerID = store->getMakerID();
    const char* queueID = store->generateUuid4();
    const char* altID = store->getAlternateDeviceID();

    //Check for availability of internet connectivity
    if(isInternetConnectivityAvailable()){
      debugI("\nActionService: PaymentTask: Connected to Internet, triggering the payment");
      //Payment triggering logic goes here
      DynamicJsonBuffer jsonBuffer;
      JsonObject& doc = jsonBuffer.createObject();
      JsonObject& botData = doc.createNestedObject("bot");
      botData["deviceID"] = deviceID;
      botData["actionID"] = actionID;
      botData["queueID"] = queueID;

      if (store->isDeviceMultipair()) {
        botData["alternativeID"] = altID;
      }

      /*if (value != NULL && strlen(value)>0) {
        botData["value"] = value;
      }*/

      char payload[200];
      doc.printTo(payload);
      jsonBuffer.clear();
      debugI("\nActionService : paymentTask: Minified JSON payload to trigger payment: %s", payload);

      //Trigger Action
      postResponse = bot->post(ACTIONS_END_POINT,payload);

    }
    else {
      debugI("\nActionService: PaymentTask: Not Connected to Internet, saving the action onto storage");
      //Fill in action metadata for payment
      struct OfflineActionMetadata pendingPayment;
      pendingPayment.offline = 1;
      pendingPayment.deviceID = new char[strlen(deviceID)+1];
      strcpy(pendingPayment.deviceID,deviceID);
      pendingPayment.makerID = new char[strlen(makerID)+1];
      strcpy(pendingPayment.makerID,makerID);
      pendingPayment.actionID = new char[strlen(actionID)+1];
      strcpy(pendingPayment.actionID,actionID);
      pendingPayment.queueID = new char[strlen(queueID)+1];
      strcpy(pendingPayment.queueID,queueID);
      if(store->isDeviceMultipair()){
        pendingPayment.multipair = 1;
        pendingPayment.alternateID = new char[strlen(altID)+1];
        strcpy(pendingPayment.alternateID,altID);
      }
      else {
        pendingPayment.multipair = 0;
        pendingPayment.alternateID = NULL;
      }
      pendingPayment.value = 0.0;
      pendingPayment.timestamp = paymentTime;
      debugI("\nActionService: PaymentTask: Payment details added to pendingPayment variable for paymentTime: %lu",pendingPayment.timestamp);
      //Get saved offline actions from storage
      offlineActionsList = store->retrieveOfflineActions();
      debugI("\nActionService: PaymentTask: Number of offlineActions retrieved from storage: %d", offlineActionsList.size());
      //Add pendignPayment item to offlineActionsList
      offlineActionsList.push_back(pendingPayment);
      debugI("\nActionService: PaymentTask: Pending payment added to offlineActionsList, PP Count: %d",offlineActionsList.size());
      if(store->saveOfflineActions(offlineActionsList)){
        debugI("\nActionService: PaymentTask: Pending payments successfully saved to %s file",OFFLINE_ACTIONS_FILE);
      }
      else {
        debugE("\nActionService: PaymentTask: Saving pending payments to file - %s failed...",OFFLINE_ACTIONS_FILE);
      }
      //Release memory for offline payments present in offlineActionsList
      //clearOfflineActionsList();
    }
  }
  else {
    debugI("\nActionService: PaymentTask: offlinePaymentTask is active, skipping paymentsTask!!!");
  }

  debugI("\nActionService: PaymentTask: Terminating the triggerPayment task");
  debugI("\nActionService: PaymentTask: Available free heap at the end of triggerPaymemntTask: %lu",ESP.getFreeHeap());
  //Turn of paymentTask flag
  paymentTaskRunning = false;
  //Terminate the task
  vTaskDelete(NULL);
}

ActionService :: ActionService(){
  store = KeyStore :: getKeyStoreInstance();
  bot = BoTService :: getBoTServiceInstance();
  timeClient = new NTPClient(ntpUDP);
}

ActionService :: ~ActionService(){
  delete timeClient;
}

String* ActionService :: triggerAction(const char* actionID, const char* value, const char* altID){
  debugD("\nActionService :: triggerAction: Initializing NTPClient to capture action trigger time");
  timeClient->begin();
  debugD("\nActionService :: triggerAction: Checking actionID - %s valid or not", actionID);
  presentActionTriggerTimeInSeconds = 0;
  //if(isValidAction(actionID)){
    debugD("\nActionService :: triggerAction: %s is valid actionID, trying to trigger now", actionID);
    store->initializeEEPROM();
    store->loadJSONConfiguration();

    // Main task runs on Core-1 and keeps track of doing it's work
    debugI("\nActionService :: triggerAction: Main task running on core: %d",xPortGetCoreID());
    debugI("\nActionService :: triggerAction: Available free heap at the beginning of triggerAction: %lu",ESP.getFreeHeap());

    //Wait till previous task instances get removed
    while(offlineTaskRunning || paymentTaskRunning){
      debugI("\nActionService :: triggerAction: Waiting for previous tasks to be removed...");
      delay(1000);
    }
    //create a task to get offline actions from file if any, with priority 1
    //and executed on core 0 to trigger the payments
    if(isInternetConnectivityAvailable() && store->offlineActionsExist()){
      debugI("\nActionService :: triggerAction: Internet Connectivity Available and There are some offline actions to handle");
      if(!offlineTaskRunning){
        xTaskCreatePinnedToCore(
          triggerOfflineActions,      /* Task function. */
          "Offline Actions Task",     /* name of task. */
          10000,                      /* Stack size of task */
          NULL,                       /* parameter of the task */
          1,                          /* priority of the task */
          &offlineActionTask,         /* Task handle to keep track of created task */
          0);                         /* pin task to core 1 */
      }
      else {
        debugI("\nActionService :: triggerAction: offlinePaymentTask already exists.. not creating any new offlinePaymentTask");
      }
    }
    else {
      debugI("\nActionService :: triggerAction: No Internet Connectivity or There are no offline actions to handle");
    }
    //Give some time for offline payment task
    delay(1000);

    //Reinitialize postResponse
    postResponse = NULL;

    //create a task to trigger payment, with priority 1 and executed on core 0
    if(!paymentTaskRunning){
      xTaskCreatePinnedToCore(
        triggerPayment,  /* Task function. */
        "Payment Task",  /* name of task. */
        10000,           /* Stack size of task */
        (void*)actionID,        /* parameter of the task */
        1,               /* priority of the task */
        &paymentTask,    /* Task handle to keep track of created task */
        0);             /* pin task to core 0 */
     }
    else {
      debugI("\nActionService :: triggerAction: paymentTask already exists.. not creating any new paymentTask");
    }
    //Give some time for payment task
    delay(1000);

    debugI("\nActionService :: triggerAction: Available free heap at the end of triggerAction: %lu",ESP.getFreeHeap());

    //Wait till the task instances get removed
    while(offlineTaskRunning || paymentTaskRunning){
      debugI("\nActionService :: triggerAction: Waiting for payment tasks to be completed...");
      delay(1000);
    }

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
