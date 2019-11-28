/*
  startAsyncServer.ino - Example sketch program to show the usage for BoT-ESP32-SDK Server.
  Created by Lokesh H K, April 7, 2019.
  Released into the repository BoT-ESP32-SDK.

  Sketch starts the AsyncWebServer on the ESP-32 board and waits for client requests to serve

  Use of both Webserver and Webclient on the same ESP-32 board is not recommended
  Use one ESP-32 board as a server and other one as a client / some other external client

  Sample assumes that We have valid private-public key pair for the device, api public key.
  Also we have valid makerID, deviceID, actionID defined.
  All these information is provided through files present in data directory of the sample.
  Update the required configuration, key-pair for the device in the data directory
  Make sure to flash all the files present in data directory onto ESP32 board using Arduino IDE.

  Change Partition Scheme to No OTA (Large APP) in Arduino IDE -> Tools after connecting ESP-32 board
  to avoid the error message saying "Sketch is too big" before compiling and uploading Sketch to ESP-32

  Here is the sketch flow:

  1. Gets makerID from the provided configuration
  2. Initializes the configuration, internally it waits for pairing and device gets activated
  3. Starts the Async Webserver on port 3001 on ESP32 board
  4. Webserver provides the endpoints /qrcode, /actions, /pairing and /action/actionID for direct interaction
  5. Waits for client requests to respond on the defined end points
*/

#include <Webserver.h>

//Custom WiFi Credentials
#define WIFI_SSID "FINN"
#define WIFI_PASSWD "Id4S7719G99XG1R"

Webserver *server;
KeyStore* store = KeyStore::getKeyStoreInstance();
AsyncWebSocket ws("/ws");

//Webserver Port
const int port = 3001;

//MakerId
char* makerID = NULL;

//Setup function for the board
void setup()
{
  //Load the given configuration details from the SPIFFS
  store->loadJSONConfiguration();

  //Initialize EEPROM to load previous device state if any
  store->initializeEEPROM();

  //Continue if makerID is available
  if(store->getMakerID() != NULL){
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
    LOG("\nstartAsyncServer: MakerID can not be NULL!");
  }
}

//Loop function for the board
void loop()
{
  debugI("\nstartAsyncServer: Avalable free heap at the beginning of loop: %lu",ESP.getFreeHeap());
  //Continue if makerID is available
  if(store->getMakerID() != NULL){
    //Check for Webserver availability
    if(server->isServerAvailable()){
      //Cleanup earlier leftover client connections
      ws.cleanupClients();

      debugI("\nstartAsyncServer: Webserver is Accessible using the URL: http://%s:%d",
                                                   (server->getBoardIP().toString()).c_str(),port);
      debugI("\nstartAsyncServer: Avialable rest endpoints - /qrcode /actions /pairing /activate /action");

      #ifndef DEBUG_DISABLED
       Debug.handle();
      #endif

      //Introduce delay of 2 mins
      delay(2*60*1000);
    }
    else {
      if(server->isWiFiConnected()){
        debugI("\nstartAsyncServer: Webserver not available, Starting the server");
        //Start the Async Webserver on ESP32 board to serve external requests
        server->startServer();

        #ifndef DEBUG_DISABLED
          Debug.handle();
        #endif
      }
      else {
        //Enable board to connect to WiFi Network
        LOG("\nstartAsyncServer: Board not connected to WiFi, reconnecting board to WiFi network");
        server->connectWiFi();
      }
      ////Introduce delay of 2 secs
      delay(2000);
   }
  }
  else {
    LOG("\nstartAsyncServer: MakerID can not be NULL!");
  }

 debugI("\nstartAsyncServer: Available free heap at the end of loop: %lu",ESP.getFreeHeap());
}
