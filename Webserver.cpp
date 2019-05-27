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
  config = NULL;
  ble = NULL;
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
    store->initializeEEPROM();
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

     config = new ConfigurationService();

     ble = new BluetoothService();

     server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        //request->send(200, "text/plain", "Banking of Things ESP-32 SDK Webserver");
        AsyncJsonResponse * response = new AsyncJsonResponse();
        response->addHeader("Server","ESP-32 Dev Module Async Web Server");
        JsonObject& root = response->getRoot();
        root["actionsEndPoint"] = "/actions";
        root["pairingEndPoint"] = "/pairing";
        response->setLength();
        request->send(response);
      });

      server->on("/actions", HTTP_GET, [](AsyncWebServerRequest *request){
         ControllerService cs;
         cs.getActions(request);
      });

      server->on("/pairing", HTTP_GET, [](AsyncWebServerRequest *request){
         ControllerService cs;
         cs.pairDevice(request);
      });

      AsyncCallbackJsonWebHandler* actionHandler = new AsyncCallbackJsonWebHandler("/actions", [](AsyncWebServerRequest *request, JsonVariant &json) {
        ControllerService cs;
        cs.triggerAction(request,json);
      });
      server->addHandler(actionHandler);

      server->begin();
      serverStatus = STARTED;
      LOG("\nWebserver :: startServer: BoT Async Webserver started on ESP-32 board at port: %d, \nAccessible using the URL: http://", port);
      Serial.print(getBoardIP());
      LOG(":%d/\n",port);

      //Check pairing status for the device
      PairingService* ps = new PairingService();
      String psResponse = ps->getPairingStatus();
      delete ps;

      //Device is already paired, then device initialization is skipped
      //Otherwise waits till BLE client connects and key exchanges happen followed by device configuration
      if((psResponse.indexOf("true")) != -1){
        LOG("\nWebserver :: startServer: Device is already paired, no need to initialize and configure");
      }
      else {
        LOG("\nWebserver :: startServer: Device is not paired yet, needs initialization");
        LOG("\nWebserver :: startServer: Free Heap before BLE Init: %u", ESP.getFreeHeap());
        ble->initializeBLE();
        bool bleClientConnected = false;
        //Wait till BLE Client connects to BLE Server
        do {
          delay(2000);
          bleClientConnected = ble->isBLEClientConnected();
          if(!bleClientConnected)
            LOG("\nWebserver :: startServer: Waiting for BLE Client to connect and exchange keys");
          else
            LOG("\nWebserver :: startServer: BLE Client connected to BLE Server");
        }while(!bleClientConnected);

        //Wait till BLE Client disconnects from BLE Server
        while(bleClientConnected){
          LOG("\nWebserver :: startServer: Waiting for BLE Client to disconnect from BLE Server");
          bleClientConnected = ble->isBLEClientConnected();
          delay(2000);
        }

        //Release memory used by BLE Service once BLE Client gets disconnected
        if(!bleClientConnected) ble->deInitializeBLE();
        LOG("\nWebserver :: startServer: Free Heap after BLE deInit: %u", ESP.getFreeHeap());

        //Proceed with device initialization and followed by configuring
        config->initialize();
        config->configureDevice();
     }
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