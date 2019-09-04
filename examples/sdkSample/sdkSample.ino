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
  4. Sketch has code to trigger actions with various frequencies like minutely, hourly, daily, monthly,half-yearly, yearly and always
  5. Define the actions in maker portal, add service in companion app, update the actionIDs properly before executing the sketch
  6. Remove the comments for required action to be triggered based on the action frequency
  7. Webserver provides the endpoints /actions, /pairing for direct interaction
*/

#include <Storage.h>
#include <Webserver.h>

//Custom WiFi Credentials
#define WIFI_SSID "LJioWiFi"
#define WIFI_PASSWD "adgjmptw"

//Declare service variables
KeyStore *store = NULL;
Webserver *server = NULL;
HTTPClient* httpClient = NULL;

//Action ID with frequency as "always"
String actionIDAlways = String("E6509B49-5048-4151-B965-BB7B2DBC7905");

//Action ID with frequency as "minutely"
String actionIDMinutely = String("A42ABD19-3226-47AB-8045-8129DBDF117E");

//Action ID with frequency as "hourly"
String actionIDHourly = String("749081B8-664D-4A15-908E-1C3F6590930D");

//Action ID with frequency as "daily"
String actionIDDaily = String("81F6011A-9AF0-45AE-91CD-9A0CDA81FA1F");

//Action ID with frequency as "weekly"
String actionIDWeekly = String("0BF5E8D2-9062-467E-BB19-88CB76D06F8E");

//Action ID with frequency as "monthly"
String actionIDMonthly = String("C257DB70-AE57-4409-B94E-678CB1567FA6");

//Action ID with frequency as "half-yearly"
String actionIDHYearly = String("D93F99E1-011B-4609-B04E-AEDBA98A7C5F");

//Action ID with frequency as "yearly"
String actionIDYearly = String("0097430C-FA78-4087-9B78-3AC7FEEF2245");

//Webserver Port
const int port = 3001;

//Variable to hold given deviceID value
const char* deviceID = NULL;

//Variable to keep track of actions submitted
int submitCount = 0;

void setup()
{
  //Get KeyStore Instance
  store = KeyStore::getKeyStoreInstance();

  //Load the given configuration details from the SPIFFS
  store->loadJSONConfiguration();

  //Initialize EEPROM to load previous device state if any
  store->initializeEEPROM();

  //Get the given makerID from the configuration
  const char* makerID = store->getMakerID();

  //Get the given deviceID from the configuration
  deviceID = store->getDeviceID();

  //Continue if makerID is available
  if(makerID != NULL){
    //Variable to flag whether to load WiFi credentials from given configuration or not
    bool loadConfig = true;

    //Override HTTPS
    store->setHTTPS(true);

    //Instantiate Webserver by using WiFi credentials from configuration
    //server = new Webserver(loadConfig);

    //Instantiate Webserver by using the custom WiFi credentials
    loadConfig = false;
    int logLevel = BoT_INFO;
    server = new Webserver(loadConfig,WIFI_SSID, WIFI_PASSWD,logLevel);

    //Enable board to connect to WiFi Network
    server->connectWiFi();
  }
  else {
    LOG("\nsdkSample: MakerID can not be NULL!");
  }
}

void loop()
{
  debugI("\nAvalable free heap at the beginning of loop: %lu",ESP.getFreeHeap());
  //Check for Webserver availability to trigger the action
  if(server->isServerAvailable()){
    int dState = store->getDeviceState();
    debugI("\nsdkSample :: Device State -> %s",store->getDeviceStatusMsg());
    //Check for the device state, should be active to trigger the action
    if(dState >= DEVICE_ACTIVE){
      //Trying to trigger an action with frequency as "minutely"
      debugI("\nsdkSample: Device State is ACTIVE and triggering the minutely action - %s", actionIDMinutely.c_str());
      submitAnAction(actionIDMinutely.c_str());

      /*
      //Trying to trigger an action with frequency as "hourly"
      debugI("\nsdkSample: Device State is ACTIVE and triggering the hourly action - %s", actionIDHourly.c_str());
      triggerAnAction(actionIDHourly.c_str());
      */

      /*
      //Trying to trigger an action with frequency as "daily"

      debugI("\nsdkSample: Device State is ACTIVE and triggering the daily action - %s", actionIDDaily.c_str());
      triggerAnAction(actionIDDaily.c_str());
      */

      /*
      //Trying to trigger an action with frequency as "weekly"

      debugI("\nsdkSample: Device State is ACTIVE and triggering the weekly action - %s", actionIDWeekly.c_str());
      triggerAnAction(actionIDWeekly.c_str());
      */

      /*
      //Trying to trigger an action with frequency as "monthly"

      debugI("\nsdkSample: Device State is ACTIVE and triggering the monthly action - %s", actionIDMonthly.c_str());
      triggerAnAction(actionIDMonthly.c_str());
      */

      /*
      //Trying to trigger an action with frequency as "half-yearly"

      debugI("\nsdkSample: Device State is ACTIVE and triggering the half-yearly action - %s", actionIDHYearly.c_str());
      triggerAnAction(actionIDHYearly.c_str());
      */

      /*
      //Trying to trigger an action with frequency as "yearly"

      debugI("\nsdkSample: Device State is ACTIVE and triggering the yearly action - %s", actionIDYearly.c_str());
      triggerAnAction(actionIDYearly.c_str());
      */

      /*
      //Trying to trigger an action with frequency as "always"
      debugI("\nsdkSample: Device State is ACTIVE and triggering the action with always frequency - %s", actionIDAlways.c_str());
      triggerAnAction(actionIDAlways.c_str());
      */
    }
    else {
      debugI("\nsdkSample: Device State is not active to trigger the action, Try pairing the device again:");
      //Instantiate HTTP Client to send HTTP Request to pair the device
      httpClient = new HTTPClient();
      httpClient->begin((server->getBoardIP()).toString(),port,"/pairing");

      //Set HTTP Call timeout as 1 min
      httpClient->setTimeout(1*60*1000);

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
        debugI("\nsdkSample: Device paired successfully, we can trigger the action now");
      }
      else {
        debugE("\nsdkSample: Device pairing failed with httpCode - %d and message: %s", httpCode, payload.c_str());
      }
    }

    //Put delay to reclaim the released memory
    delay(1000);
    debugI("\nAvalable free heap at the end of loop: %lu",ESP.getFreeHeap());

    #ifndef DEBUG_DISABLED
      Debug.handle();
    #endif

    //Introduce delay of 2 mins
    delay(2*60*1000);

  }
  else {
    if(server->isWiFiConnected()){
      debugI("\nsdkSample: Webserver not available, Starting the server");
      //Start the Async Webserver on ESP32 board to serve external requests
      server->startServer();

      #ifndef DEBUG_DISABLED
        Debug.handle();
      #endif

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

void submitAnAction(const char* actionID){
  //Instantiate HTTP Client to send HTTP Request to submit the action
  httpClient = new HTTPClient();
  httpClient->begin((server->getBoardIP()).toString(),port,"/actions");

  //Prepare body with actionID
  String body = (String)"{\"actionID\": \"" + actionID +   "\"";

  //Add alternativeID if device is DEVICE_MULTIPAIR
  if(store->isDeviceMultipair())
    body.concat(",\"alternativeID\": \"" + String(store->getAlternateDeviceID()) +"\"");
  body.concat(" } ");
  debugI("\nsdkSample: triggerAction Body contents: %s",body.c_str());

  //Set required headers for HTTP Call
  httpClient->addHeader("Content-Type", "application/json");
  httpClient->addHeader("Content-Length",String(body.length()));

  //Set HTTP Call timeout as 1 min
  httpClient->setTimeout(1*60*1000);

  //Call HTTP Post to submit action
  int httpCode = httpClient->POST(body);

  //Get response body contents
  String payload = httpClient->getString();

  //Check for successful submitting of given action to be triggered
  if(httpCode == 200){
    submitCount++;
    debugI("\nsdkSample: Action Submitted, actionSubmitCount = %d", submitCount);
  }
  else {
    debugE("\nsdkSample: Action submission failed with httpCode - %d and message: %s", httpCode, payload.c_str());
  }

  //End http
  httpClient->end();

  //Deallocate memory allocated for httpClient
  delete httpClient;

}
