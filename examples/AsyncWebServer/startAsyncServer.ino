/*
  startAsyncServer.ino - Example sketch program to show the usage for BoT-ESP32-SDK Server.
  Created by Lokesh H K, April 7, 2019.
  Released into the repository BoT-ESP32-SDK.
*/
#include <Webserver.h>

Webserver *server;
KeyStore* store;
HTTPClient* httpClient;

//Webserver Port
const int port = 3001;

void setup()
{
  store = KeyStore :: getKeyStoreInstance();
  store->loadJSONConfiguration();
  store->initializeEEPROM();

  //Override HTTPS
  store->setHTTPS(true);

  //Get WiFi Credentials from given configuration
  //const char* WIFI_SSID = store->getWiFiSSID();
  //const char* WIFI_PASSWD = store->getWiFiPasswd();

  //Provide custom WiFi Credentials
  const char* WIFI_SSID = "LJioWiFi";
  const char* WIFI_PASSWD = "adgjmptw";

  //Instantiate Webserver by using the custom WiFi credentials
  bool loadConfig = false;
  int debugILevel = BoT_DEBUG;
  server = new Webserver(loadConfig,WIFI_SSID, WIFI_PASSWD,debugILevel);

  //Enable board to connect to WiFi Network
  server->connectWiFi();

  //Instantiate HTTPClient Instance
  httpClient = new HTTPClient();

}

void loop()
{
  //Proceed further if board connects to WiFi Network
  if(server->isWiFiConnected()){
   if(server->isServerAvailable()){
    debugI("\nstartAsyncServer: Async Webserver is running on ESP-32 board, submit requests externally");
    /*
      //Retrieve defined actions for given makerID
      httpClient->begin((server->getBoardIP()).toString(),3001,"/actions");
      //Set HTTP Call timeout as 2 mins
      httpClient->setTimeout(2*60*1000);

      int httpCode = httpClient->GET();
      String payload = httpClient->getString();
      httpClient->end();

      if(httpCode == 200){
        debugI("\nstartAsyncServer: Actions Retrieved from BoT Service: %s", payload.c_str());
      }
      else {
        debugI("\nstartAsyncServer: Actions Retrieval failed with httpCode - %d", httpCode);
      }

      //For device state as NEW, /pairing will wait for maxPairAttempts and return the response as Not Paired
      //Make sure to have new unpaired deviceID in configuration file flashed to ESP32 board
      store->setDeviceState(DEVICE_NEW);
      httpClient->begin((server->getBoardIP()).toString(),port,"/pairing");
      //Set HTTP Call timeout as 2 mins
      httpClient->setTimeout(2*60*1000);

      httpCode = httpClient->GET();
      payload = httpClient->getString();
      httpClient->end();

      if(httpCode == 200){
         if(store->getDeviceState() == DEVICE_ACTIVE){
           debugI("\nstartAsyncServer: Device Activation Successful");
         }
         else
           debugI("\nstartAsyncServer: Device Not Activated, try again");
     }
     else {
        debugI("\nstartAsyncServer: Calling /pairing failed with httpCode - %d", httpCode);
     }

     //Generate QRCode for the device hitting /qrcode end point
     httpClient->begin((server->getBoardIP()).toString(),3001,"/qrcode");
     //Set HTTP Call timeout as 2 mins
     httpClient->setTimeout(2*60*1000);

     int httpCode = httpClient->GET();
     httpClient->end();

     if(httpCode == 200){
       debugI("\nstartAsyncServer: QRCode generation successful");
     }
     else {
       debugI("\nstartAsyncServer: QRCode generation failed with httpCode - %d", httpCode);
     } */
    }
    else {
      if(server->isWiFiConnected()){
        debugI("\nstartAsyncServer: Webserver not available, Starting the server");
        //Start the Async Webserver on ESP32 board to serve external requests
        server->startServer();
      }
      else {
        //Enable board to connect to WiFi Network
        LOG("\nsdkSample: Board not connected to WiFi, reconnecting board to WiFi network");
        server->connectWiFi();
      }
    }
 }
 else {
   LOG("\nstartAsyncServer: ESP-32 board not connected to WiFi Network, try again");
   //Enable board to connect to WiFi Network
   server->connectWiFi();
 }

 #ifndef DEBUG_DISABLED
   Debug.handle();
 #endif

 delay(1*60*1000);
}
