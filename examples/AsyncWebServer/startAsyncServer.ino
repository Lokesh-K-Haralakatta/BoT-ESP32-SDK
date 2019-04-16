/*
  startAsyncServer.ino - Example sketch program to show the usage for BoT-ESP32-SDK Server.
  Created by Lokesh H K, April 7, 2019.
  Released into the repository BoT-ESP32-SDK.
*/
#include <Webserver.h>
#define WIFI_SSID "LJioWiFi"
#define WIFI_PASSWD "adgjmptw"

//Webserver server(false,WIFI_SSID, WIFI_PASSWD);
Webserver server(true);

void setup()
{
  server.connectWiFi();
  if(server.isWiFiConnected()){
    server.startServer();
  }
  else {
    LOG("\nESP-32 board not connected to WiFi Network");
  }

}

void loop()
{
  if(server.isServerAvailable()){
    server.blinkLED();
  }

}
