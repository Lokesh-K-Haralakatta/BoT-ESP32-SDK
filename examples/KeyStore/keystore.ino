/*
  keyStore.ino - Example sketch program to show the usage for configuration retrieval and store for BoT Service.
  Created by Lokesh H K, April 9, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include <Storage.h>

KeyStore* store;

void setup(){
  //Serial.begin(115200);
  store = KeyStore :: getKeyStoreInstance();

  store->loadJSONConfiguration();
  if(store->isJSONConfigLoaded()){
    LOG("\n WiFi SSID: %s", store->getWiFiSSID());
    LOG("\n WiFi Passwd: %s", store->getWiFiPasswd());
    LOG("\n Maker ID: %s", store->getMakerID());
    LOG("\n Device ID: %s", store->getDeviceID());
    LOG("\n Altternate Device ID: %s", store->getAlternateDeviceID());
  }
  store->retrieveAllKeys();
  if(store->isPrivateKeyLoaded()){
    LOG("\n Private Key Contents: \n%s\n", store->getDevicePrivateKey());
  }
  if(store->isPublicKeyLoaded()){
    LOG("\n Public Key Contents: \n%s\n", store->getDevicePublicKey());
  }
  if(store->isAPIKeyLoaded()){
    LOG("\n API Key Contents: \n%s\n", store->getAPIPublicKey());
  }

  //Initialize actions to save to file
  const char* id1 = "E6509B49-5048-4151-B965-BB7B2DBC7905";
  const char* freq1 = "minuetly";
  const unsigned long ltt1 = 1557225825;

  const char* id2 = "E6509B49-5048-4151-B965-BB7B2DBC7906";
  const char* freq2 = "hourly";
  const unsigned long ltt2 = 1557225886;

  const char* id3 = "E6509B49-5048-4151-B965-BB7B2DBC7907";
  const char* freq3 = "daily";
  const unsigned long ltt3 = 1557225936;

  const char* id4 = "E6509B49-5048-4151-B965-BB7B2DBC7908";
  const char* freq4 = "weekly";
  const unsigned long ltt4 = 1557225987;

  std::vector <struct Action> actionsList;

  struct Action item1;
  item1.actionID = new char[strlen(id1)+1];
  item1.actionFrequency = new char[strlen(freq1)+1];

  strcpy(item1.actionID,id1);
  strcpy(item1.actionFrequency,freq1);
  item1.triggeredTime = ltt1;
  actionsList.push_back(item1);

  struct Action item2;
  item2.actionID = new char[strlen(id2)+1];
  item2.actionFrequency = new char[strlen(freq2)+1];

  strcpy(item2.actionID,id2);
  strcpy(item2.actionFrequency,freq2);
  item2.triggeredTime = ltt2;
  actionsList.push_back(item2);

  struct Action item3;
  item3.actionID = new char[strlen(id3)+1];
  item3.actionFrequency = new char[strlen(freq3)+1];

  strcpy(item3.actionID,id3);
  strcpy(item3.actionFrequency,freq3);
  item3.triggeredTime = ltt3;
  actionsList.push_back(item3);

  struct Action item4;
  item4.actionID = new char[strlen(id4)+1];
  item4.actionFrequency = new char[strlen(freq4)+1];
  strcpy(item4.actionID,id4);
  strcpy(item4.actionFrequency,freq4);
  item4.triggeredTime = ltt4;
  actionsList.push_back(item4);

  if(store->saveActions(actionsList)){
    LOG("\n %d actions saved to file - %s", actionsList.size(),ACTIONS_FILE);
  }
  else {
    LOG("\n Failed in saving actions to file - %s", ACTIONS_FILE);
  }

  actionsList = store->retrieveActions();
  if(actionsList.empty()){
    LOG("\n No Actions retrieved from the file - %s", ACTIONS_FILE);
  }
  else {
    LOG("\n Actions retrieved from the file - %s : %d", ACTIONS_FILE,actionsList.size());
    for (std::vector<struct Action>::iterator i = actionsList.begin() ; i != actionsList.end(); ++i){
      LOG("\n %s : %s : %lu", i->actionID, i->actionFrequency, i->triggeredTime);
    }
  }
}

void loop(){

}
