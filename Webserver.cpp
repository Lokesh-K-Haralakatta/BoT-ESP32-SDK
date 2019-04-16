/*
  Webserver.cpp - Webserver Class methods for conecting to given WiFi Network and
                  Starting Async Webserver to serve the BoT requests
  Created by Lokesh H K, April 7, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include "Webserver.h"

Webserver :: Webserver(bool loadConfig, const char *ssid, const char *passwd){
  ledPin = 2;
  port = 3001;
  WiFi_SSID = NULL;
  WiFi_Passwd = NULL;
  server = NULL;
  store = NULL;
  serverStatus = NOT_STARTED;
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial.begin(115200);

  if(loadConfig == false){
    WiFi_SSID = new String(ssid);
    WiFi_Passwd = new String(passwd);
  }
}

void Webserver :: connectWiFi(){
  if(WiFi_SSID == NULL || WiFi_Passwd == NULL){
    store = KeyStore::getKeyStoreInstance();
    store->loadJSONConfiguration();
    WiFi_SSID = new String(store->getWiFiSSID());
    WiFi_Passwd = new String(store->getWiFiPasswd());
  }

  LOG("\nWebserver :: connectWiFi: Connecting to WiFi SSID: %s", WiFi_SSID->c_str());
  if(WiFi_SSID != NULL && WiFi_Passwd != NULL){
    //WiFi.disconnect(true);
    WiFi.begin(WiFi_SSID->c_str(), WiFi_Passwd->c_str());

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        LOG("\nWebserver :: connectWiFi: Trying to Connect to WiFi SSID: %s\n", WiFi_SSID->c_str());
    }

    LOG("\nWebserver :: connectWiFi: Board Connected to WiFi SSID: %s, assigned IP: ", WiFi_SSID->c_str());
    Serial.print(getBoardIP());
    blinkLED();
  }
}

IPAddress Webserver :: getBoardIP(){
  if(isWiFiConnected() == true){
    return WiFi.localIP();
  }
}

void Webserver::blinkLED()
{
  digitalWrite(ledPin, LOW);
  delay(1000);
  digitalWrite(ledPin, HIGH);
  delay(1000);
}

bool Webserver :: isWiFiConnected(){
   return (WiFi.status() == WL_CONNECTED)?true:false;
}

void Webserver :: startServer(){
   if(isWiFiConnected() == true){
     LOG("\nWebserver :: startServer: Starting the Async Webserver...");

     server = new AsyncWebServer(port);

     server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        //request->send(200, "text/plain", "Banking of Things ESP-32 SDK Webserver");
        AsyncJsonResponse * response = new AsyncJsonResponse();
        response->addHeader("Server","ESP-32 Dev Module Async Web Server");
        JsonObject& root = response->getRoot();
        root["heap"] = ESP.getFreeHeap();
        root["ssid"] = WiFi.SSID();
        response->setLength();
        request->send(response);
      });

      server->begin();
      serverStatus = STARTED;
      LOG("\nWebserver :: startServer: BoT Async Webserver started on ESP-32 board at port: %d, \nAccessible using the URL: http://", port);
      Serial.print(getBoardIP());
      LOG(":%d/\n",port);
   }
   else {
     LOG("\nWebserver :: startServer: ESP-32 board not connected to WiFi Network");
   }
}

bool Webserver :: isServerAvailable(){
   if(isWiFiConnected() && serverStatus == STARTED)
      return true;
   else {
      digitalWrite(ledPin, LOW);
      return false;
  }
}
