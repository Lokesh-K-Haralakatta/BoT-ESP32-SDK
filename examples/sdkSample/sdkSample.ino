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
  6. Update required action to be triggered based on the action frequency in the URI formation statement
  7. Webserver provides the endpoints /qrcode, /actions, /pairing and /action/actionID for direct interaction
*/

#include <Storage.h>
#include <Webserver.h>
#include <AsyncTCP.h>

//Custom WiFi Credentials
#define WIFI_SSID "FINN"
#define WIFI_PASSWD "Id4S7719G99XG1R"

//Declare service variables
KeyStore *store = KeyStore::getKeyStoreInstance();;
Webserver *server = NULL;
AsyncClient *client_tcp = new AsyncClient;;
TaskHandle_t tTask;
TaskHandle_t pTask;

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

//Function to request for actions from the Server using the end point /actions
void getActionsFromServer(void *arg)
{
  AsyncClient *client = reinterpret_cast<AsyncClient *>(arg);
  // We now create a URI for the request
  String requestString = String("GET /actions HTTP/1.1\r\n http://") + client->remoteIP().toString() + ":3001/ \r\n Connection: reuse\r\n\r\n";
  const char* serverReqStr = requestString.c_str();
  debugD("\nsdkSample: getActionsFromServer: Server Request: %s",serverReqStr);
  // Send request to get actions from server
  size_t nBytes = client->write(serverReqStr);
  debugD("\nsdkSample: getActionsFromServer: Amount of bytes written to Server: %d",nBytes);
}

//Function to request to trigger action to the Server using the end point /action
void triggerAction(void *arg1){
  AsyncClient *client = reinterpret_cast<AsyncClient *>(arg1);
  // We now create a URI for the request
  // Update the required actionId in the URI formation below
  String requestString = String("GET /action?actionID=") + actionIDMinutely.c_str() + " HTTP/1.1\r\n http://" + client->remoteIP().toString() + ":3001/ \r\n Connection: reuse\r\n\r\n";
  const char* serverReqStr = requestString.c_str();
  debugD("\nsdkSample: triggerAction: Server Request: %s",serverReqStr);

  //Send action trigger request to server
  //size_t nBytes = client->write("GET /action?actionID=E6509B49-5048-4151-B965-BB7B2DBC7905 HTTP/1.1\r\n http://10.26.16.126:3001/ \r\n Connection: reuse\r\n\r\n");
  size_t nBytes = client->write(serverReqStr);
  debugD("\nsdkSample: triggerAction: Amount of bytes written to Server: %d",nBytes);

}

//Function to request for pairing from the Server using the end point /pairing
void devicePair(void *arg){
  AsyncClient *client = reinterpret_cast<AsyncClient *>(arg);
  // We now create a URI for the request
  String requestString = String("GET /pairing HTTP/1.1\r\n http://") + client->remoteIP().toString() + ":3001/ \r\n Connection: reuse\r\n\r\n";
  const char* serverReqStr = requestString.c_str();
  debugD("\nsdkSample: devicePair: Server Request: %s",serverReqStr);
  // Send pairing request to server
  size_t nBytes = client->write(serverReqStr);
  debugD("\nsdkSample: devicePair: Amount of bytes written to Server: %d",nBytes);
}

//Async function to handle data received from server
void handleData(void *arg, AsyncClient *client, void *data, size_t len)
{
  debugD("\nsdkSample: handleData: Data received from %s \n", client->remoteIP().toString().c_str());
  Serial.write((uint8_t *)data, len);
}

//Async function to notify on client connected to server
void onConnect(void *arg, AsyncClient *client)
{
  debugI("\nsdkSample: onConnect: Async TCP Client Connected to Webserver at port: %d", port);
  int dState = store->getDeviceState();
  debugI("\nsdkSample :: Device State -> %s",store->getDeviceStatusMsg());
  //Check for the device state, should be active to trigger the action
  if(dState >= DEVICE_ACTIVE){
    triggerAction(client);
  }
  else {
    devicePair(client);
  }
}

//Async function to notify on client disconnected from server
void onDisconnect(void *arg, AsyncClient *client){
  debugI("\nsdkSample: onDisconnect: Async TCP Client Disconnected from Webserver");
}

//Setu function for the board
void setup()
{
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

    //Attach async callback functions for the client
    client_tcp->onData(handleData, client_tcp);
    client_tcp->onConnect(onConnect, client_tcp);
    client_tcp->onDisconnect(onDisconnect, client_tcp);

  }
  else {
    LOG("\nsdkSample: MakerID can not be NULL!");
  }
}

//Loop function for the board
void loop()
{
  debugI("\nAvalable free heap at the beginning of loop: %lu",ESP.getFreeHeap());
  //Check for Webserver availability to trigger the action
  if(server->isServerAvailable()){
    debugI("\nWebserver is Accessible using the URL: http://%s:%d/actions for accessing the rest endpoints - /qrcode /actions /pairing /action", (server->getBoardIP().toString()).c_str(),port);
    //Connect to server and perform operation based on device state
    client_tcp->connect((server->getBoardIP().toString()).c_str(), port);

    debugI("\nAvalable free heap at the end of loop: %lu",ESP.getFreeHeap());

    #ifndef DEBUG_DISABLED
     Debug.handle();
    #endif

    //Introduce delay of 2 mins
    delay(2*60*1000);

    //Close the client
    client_tcp->close(true);
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
