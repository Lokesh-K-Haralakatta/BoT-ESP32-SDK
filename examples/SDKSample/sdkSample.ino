/*
  sdkSample.ino - Sample sketch to show case the ESP-32 SDK usage.
  Created by Lokesh H K, April 26, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

/*
  Sample assumes that We have valid private-public key pair for the device, api public key.
  Also we have valid makerID, deviceID, actionID defined.
  All these information is provided through files present in data directory of the sample.
  Update the required configuration, key-pair for the device in the data directory
  Make sure to flash all the files present in data directory onto ESP32 board using Arduino IDE.
*/

/*
  To use the ESP-32 SDK, include BoT-ESP32-SDK.zip through Arduino IDE
*/

/* Change Partition Scheme to No OTA (Large APP) in Arduino IDE -> Tools after connecting ESP-32 board
   to avoid the error message saying "Sketch is too big" before compiling and uploading Sketch to ESP-32 */

/*
  Here is the sketch flow:

  1. Gets makerID from the provided configuration
  2. Initializes the configuration, internally it waits for pairing and device gets activated
  3. Starts the Async Webserver on port 3001 on ESP32 board
  4. Triggers the action for every 1 minute if device state is active and connected to WiFi
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

//Variable to hold given deviceID value
const char* deviceID = NULL;

//Variable to keep track of action triggered
int triggerCount = 0;

void setup()
{
  //Get KeyStore Instance
  store = KeyStore::getKeyStoreInstance();

  //Load the given configuration details from the SPIFFS
  store->loadJSONConfiguration();

  //Get the given makerID from the configuration
  const char* makerID = store->getMakerID();

  //Get the given deviceID from the configuration
  deviceID = store->getDeviceID();

  //Continue if makerID is available
  if(makerID != NULL){
    //Variable to flag whether to load WiFi credentials from given configuration or not
    bool loadConfig = true;

    //Instantiate Webserver by using WiFi credentials from configuration
    server = new Webserver(loadConfig);

    //Instantiate Webserver by using the custom WiFi credentials
    //loadConfig = false;
    //server = new Webserver(loadConfig,WIFI_SSID, WIFI_PASSWD);

    //Enable board to connect to WiFi Network
    server->connectWiFi();

    //Proceed further if board connects to WiFi Network
    if(server->isWiFiConnected()){

      //Start the Async Webserver on ESP32 board to serve external requests
      server->startServer();

    }
    else {
    LOG("\nsdkSample: ESP-32 board not connected to WiFi Network");
    LOG("\nsdkSample: Make sure given WiFi SSID is up and reachable to the board");
    LOG("\nsdkSample: Restart the ESP-32 board again to start from beginning");
    }

  }
  else {
    LOG("\nsdkSample: MakerID can not be NULL!");
  }
}

void loop()
{
  //Check for Webserver availability to trigger the action
  if(server->isServerAvailable()){
    //Check for the device state, should be active
    if(store->getDeviceState() == DEVICE_ACTIVE){
      LOG("\nsdkSample: Device State is ACTIVE and triggering the action - %s", actionID.c_str());

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

      //Deallocate memory allocated for httpClient
      delete httpClient;

      //Check for successful triggerring of given action
      if(httpCode == 200){
        triggerCount++;
        LOG("\nsdkSample: Action triggered, actionTriggerCount = %d", triggerCount);
      }
      else {
        LOG("\nsdkSample: Action triggerring failed with httpCode - %d and message: %s", httpCode, payload.c_str());
      }
    }
    else {
      LOG("\nsdkSample: Device State is not active to trigger the action, Try pairing the device again:");
      //Instantiate HTTP Client to send HTTP Request to pair the device
      httpClient = new HTTPClient();
      httpClient->begin((server->getBoardIP()).toString(),port,"/pairing");

      //Call GET on httpClient to pair the device
      int httpCode = httpClient->GET();

      //Get response body contents
      String payload = httpClient->getString();

      //End http
      httpClient->end();

      //Deallocate memory allocated for httpClient
      delete httpClient;

      //Check for successful triggerring of pairing the device
      if(httpCode == 200){
        LOG("\nsdkSample: Device paired successfully, we can trigger the action now");
      }
      else {
        LOG("\nsdkSample: Device pairing failed with httpCode - %d and message: %s", httpCode, payload.c_str());
      }
    }

    //Introduce delay of 1 minute
    delay(1*60*1000);

  }
  else {
    LOG("\nsdkSample: Webserver not available, restarting the server");
    if(server->isWiFiConnected()){
      //Start the Async Webserver on ESP32 board to serve external requests
      server->startServer();
    }
    else {
      //Enable board to connect to WiFi Network
      LOG("\nsdkSample: Board not connected to WiFi, reconnecting board to WiFi network");
      server->connectWiFi();
    }
    ////Introduce delay of 2 secs
    delay(2000);
  }
}
