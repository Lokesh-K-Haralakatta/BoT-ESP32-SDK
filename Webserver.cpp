/*
  Webserver.cpp - Webserver Class methods for conecting to given WiFi Network and
                  Starting Async Webserver to serve the BoT requests
  Created by Lokesh H K, April 7, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include "Webserver.h"
#ifndef DEBUG_DISABLED
  RemoteDebug Debug;
#endif

Webserver :: Webserver(bool loadConfig, const char *ssid, const char *passwd, const int logLevel){
  ledPin = 2;
  port = 3001;
  WiFi_SSID = NULL;
  WiFi_Passwd = NULL;
  server = NULL;
  store = NULL;
  config = NULL;
  ble = NULL;
  debugLevel = logLevel;
  serverStatus = NOT_STARTED;
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial.begin(115200);

  if(loadConfig == false){
    WiFi_SSID = new String(ssid);
    WiFi_Passwd = new String(passwd);
  }
  store = KeyStore::getKeyStoreInstance();
}

void Webserver :: connectWiFi(){
  if(WiFi_SSID == NULL || WiFi_Passwd == NULL){
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
        LOG("\nWebserver :: connectWiFi: Trying to Connect to WiFi SSID: %s", WiFi_SSID->c_str());
    }

    LOG("\nWebserver :: connectWiFi: Board Connected to WiFi SSID: %s, assigned IP: %s", WiFi_SSID->c_str(), (getBoardIP().toString()).c_str());
    blinkLED();
    //Remote Debug Setup if DEBUG ENABLED
    #ifndef DEBUG_DISABLED
      //Initialise RemoteDebug instance based on provided log level
      switch(debugLevel){
        case BoT_DEBUG: Debug.begin("BoT-ESP-32",Debug.DEBUG); break;
        case BoT_INFO: Debug.begin("BoT-ESP-32",Debug.INFO); break;
        case BoT_WARNING: Debug.begin("BoT-ESP-32",Debug.WARNING); break;
        case BoT_ERROR: Debug.begin("BoT-ESP-32",Debug.ERROR); break;
        default: Debug.begin("BoT-ESP-32",Debug.INFO); break;
      }
      //Set required properties for Debug
      Debug.setResetCmdEnabled(true);
      Debug.setSerialEnabled(true);
      Debug.showProfiler(true);
      Debug.showColors(true);
    #endif
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

bool Webserver :: isDevicePaired(){
  //Check pairing status for the device
  PairingService* ps = new PairingService();
  String psResponse = ps->getPairingStatus();
  delete ps;

  if((psResponse.indexOf("true")) != -1)
    return true;
  else
    return false;
}

void Webserver :: startServer(){
   if(isWiFiConnected() == true){
     debugD("\nWebserver :: startServer: Starting the Async Webserver...");

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
        root["qrCodeEndPoint"] = "/qrcode";
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

      server->on("/qrcode", HTTP_GET, [](AsyncWebServerRequest *request){
         ControllerService cs;
         cs.getQRCode(request);
      });

      AsyncCallbackJsonWebHandler* actionHandler = new AsyncCallbackJsonWebHandler("/actions", [](AsyncWebServerRequest *request, JsonVariant &json) {
        ControllerService cs;
        cs.triggerAction(request,json);
      });
      server->addHandler(actionHandler);

      server->begin();
      serverStatus = STARTED;
      debugI("\nWebserver :: startServer: BoT Async Webserver started on ESP-32 board at port: %d, \nAccessible using the URL: http://%s:%d/", port,(getBoardIP().toString()).c_str(),port);

      //Device is already paired, then device initialization is skipped
      //Otherwise waits till device gets paired using FINN APP either by
      //BLE client connects and key exchanges happen or by QR Code
      if(isDevicePaired()){
        debugI("\nWebserver :: startServer: Device is already paired, no need to initialize and configure");
      }
      else {
        debugI("\nWebserver :: startServer: Device is not paired yet, needs initialization");
        config->initialize();
        debugD("\nWebserver :: startServer: Free Heap before BLE Init: %u", ESP.getFreeHeap());
        ble->initializeBLE();
        bool bleClientConnected = false;
        unsigned long startTime = millis();
        //Wait till device gets paired from FINN APP for 2 mins through BLE
        do {
          delay(2000);
          bleClientConnected = ble->isBLEClientConnected();
          if(!bleClientConnected)
            debugI("\nWebserver :: startServer: Waiting for BLE Client to connect...");
          else
            debugI("\nWebserver :: startServer: BLE Client connected to BLE Server...");
        }while(!bleClientConnected && (millis() - startTime)<(2*60*1000));

        //Wait till BLE Client disconnects from BLE Server
        while(bleClientConnected){
          debugI("\nWebserver :: startServer: Waiting for BLE Client to disconnect from BLE Server");
          bleClientConnected = ble->isBLEClientConnected();
          delay(2000);
        }

        //Release memory used by BLE Service once BLE Client gets disconnected
        if(!bleClientConnected) ble->deInitializeBLE();
        debugD("\nWebserver :: startServer: Free Heap after BLE deInit: %u", ESP.getFreeHeap());

        //If device does not get paired through BLE,
        //wait now till it gets paired through QR Code if QR Code is available
        if(store->isQRCodeGeneratedandSaved()){
          while(!isDevicePaired()){
            debugI("\nWebserver :: startServer: Waiting for device pairing through QR Code...");
            delay(2000);
          }
          //Proceed with configuring the device
          config->configureDevice();
        }
        else
          debugW("\nWebserver :: startServer: QR Code not available for the device, try again!!!");

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
