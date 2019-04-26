/*
  sdkSample.ino - Sample sketch to show case the ESP-32 SDK usage.
  Created by Lokesh H K, April 26, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

/*
  Sample assumes that the device is already paired using Node-SDK through BLE.
  We have valid private-public key pair for the device, api public key.
  Also we have valid makerID, deviceID, actionID defined.
  All these information is provided through files present in data directory of the sample.
  Update the required configuration, key-pair for the device in the data directory
  Make sure to flash all the files present in data directory onto ESP32 board using Arduino IDE.
*/

/*
  To use the ESP-32 SDK, include BoT-ESP32-SDK.zip through Arduino IDE
*/

/*
  Here is the sketch flow:

  1. Gets makerID from the provided configuration
  2. Initializes the configuration, internally it waits for pairing and device gets activated
  3. Starts the Async Webserver on port 3001 on ESP32 board
  4. Triggers the action for every 5 mins
  5. Webserver provides the endpoints /actions, /pairing for direct interaction
*/

#include<Storage.h>
#include <Webserver.h>

//Custom WiFi Credentials
#define WIFI_SSID "LJioWiFi"
#define WIFI_PASSWD "adgjmptw"

//Declare service variables
KeyStore *store = NULL;
Webserver *server = NULL;
HTTPClient* httpClient = NULL;

//Define valid actionID to be triggered
String actionID = String("E6509B49-5048-4151-B965-BB7B2DBC7905");

//Webserver Port
const int port = 3001;

void setup()
{
  //Get KeyStore Instance
  store = KeyStore::getKeyStoreInstance();

  //Load the given configuration details from the SPIFFS
  store->loadJSONConfiguration();

  //Get the given makerID from the configuration
  const char* makerID = store->getMakerID();

  //Continue if makerID is available
  if(makerID != NULL){
    //Instantiate Webserver by using WiFi credentials from configuration
    server = new Webserver(true);

    //Instantiate Webserver by using the custom WiFi credentials
    //server = new Webserver(false,WIFI_SSID, WIFI_PASSWD);

    //Enable board to connect to WiFi Network
    server->connectWiFi();

    //Proceed further if board connects to WiFi Network
    if(server->isWiFiConnected()){

      //Start the Async Webserver on ESP32 board to serve external requests
      server->startServer();

    }
    else {
    LOG("\nESP-32 board not connected to WiFi Network");
    LOG("\nMake sure given WiFi SSID is up and reachable to the board");
    LOG("\nRestart the ESP-32 board again to start from beginning");
    }

  }
  else {
    LOG("\nMakerID can not be NULL!");
  }
}

void loop()
{
  //Check for Webserver availability to trigger the action
  if(server->isServerAvailable()){
    //Check for the device state, should be active
    if(store->getDeviceState() == DEVICE_ACTIVE){
      LOG("\nDevice State is ACTIVE and triggering the action - %s", actionID.c_str());

      //Instantiate HTTP Client to send HTTP Request to trigger the action
      httpClient = new HTTPClient();
      httpClient->begin((server->getBoardIP()).toString(),port,"/actions");

      //Prepare body with actionID
      String body = (String)"{\"actionID\": \"" + actionID +   "\"}";

      //Set required headers for HTTP Call
      httpClient->addHeader("Content-Type", "application/json");
      httpClient->addHeader("Content-Length",String(body.length()));

      //Trigger action
      int httpCode = httpClient->POST(body);

      //Get response body contents
      String payload = httpClient->getString();

      //End http
      httpClient->end();

      //Check for successful triggerring of given action
      if(httpCode == 200){
        LOG("\nAction triggered successfully...");
      }
      else {
        LOG("\nAction triggerring failed with httpCode - %d and message: %s", httpCode, payload.c_str());
      }

      //Introduce delay of 5 mins before next action trigger
      delay(5*60*1000);
    }
    else {
      LOG("\nDevice State is not active to trigger the action...");
    }
  }
  else {
    if(server->isWiFiConnected()){
      //Start the Async Webserver on ESP32 board to serve external requests
      server->startServer();
    }
    else {
      //Enable board to connect to WiFi Network
      server->connectWiFi();
    }
  }
}
