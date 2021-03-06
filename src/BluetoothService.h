/*
BluetoothService.h - Contains class and methods for BLE communication with the companion application
Created by Lokesh H K, April 29, 2019.
Released into the repository BoT-ESP32-SDK.
*/

#ifndef BluetoothService_h
#define BluetoothService_h

#include "BoTESP32SDK.h"
#include "Storage.h"

#define SERVICE_UUID        "729BE9C4-3C61-4EFB-884F-B310B6FFFFD1"
#define DEVICE_CHARACTERISTIC_UUID "CAD1B513-2DA4-4609-9908-234C6D1B2A9C"
#define DEVICE_INFO_CHARACTERISTIC_UUID "CD1B3A04-FA33-41AA-A25B-8BEB2D3BEF4E"
#define DEVICE_NETWORK_CHARACTERISTIC_UUID "C42639DC-270D-4690-A8B3-6BA661C6C899"
#define CONFIGURE_CHARACTERISTIC_UUID "32BEAA1B-D20B-47AC-9385-B243B8071DE4"

class BoTServerCallbacks;

class BluetoothService {
  public:
          BluetoothService();
          void initializeBLE(const char* deviceName=NULL);
          void deInitializeBLE();
          bool isBLEClientConnected();
          static void setClientConnected(bool status);
          static void updateWiFiConfig(const char* ssid, const char* passwd);
  private:
          char *deviceName;
          static bool clientConnected;
          static KeyStore *store;
          BLEServer *bleServer;
          BLEService *bleService;
          BLECharacteristic *bleDeviceCharacteristic;
          BLECharacteristic *bleDeviceInfoCharacteristic;
          BLECharacteristic *bleNetworkCharacteristic;
          BLECharacteristic *bleConfigureCharacteristic;
          friend class BoTServerCallbacks;
};

class BoTServerCallbacks: public BLEServerCallbacks {
  friend class BluetoothService;
  void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) {
    debugI("\nBoTServerCallbacks :: onConnect: BLE Client Connected...");
    BluetoothService :: setClientConnected(true);
    //pServer->getAdvertising()->start();
    //debugI("\nBoTServerCallbacks :: onConnect: Started advertising again...");
    //pServer->updateConnParams(param->connect.remote_bda, 10*1000, 30*1000, 1*1000, 30*1000);
    //debugI("\nBoTServerCallbacks :: onConnect: Updated connection parameters...");
  };

  void onDisconnect(BLEServer* pServer) {
    debugI("\nBoTServerCallbacks :: onDisconnect: BLE Client Disconnected...");
    //BluetoothService :: setClientConnected(false);
    //pServer->getAdvertising()->start();
    //debugI("\nBoTServerCallbacks :: onDisconnect: Started advertising again...");
  }
};

class ConfigureCharacteristicsCallbacks: public BLECharacteristicCallbacks {
    friend class BluetoothService;
    void onWrite(BLECharacteristic *pCharacteristic) {
      String* receivedString = new String(pCharacteristic->getValue().c_str());
      debugD("\n\nConfigureCharacteristicsCallbacks :: onWrite: Received value: %s", receivedString->c_str());
      //Check whether it's due to skipping the wiFi Config on application
      if(receivedString->indexOf("Skip") != -1){
        debugD("\nConfigureCharacteristicsCallbacks :: onWrite: WiFi Config on FINN Application is skipped ...");
      }
      else {
        debugD("\nConfigureCharacteristicsCallbacks :: onWrite: WiFi config details provided on FINN Application...");
        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(receivedString->c_str());
        String* SSID = new String(root.get<const char*>("SSID"));
        String* PASSWD = new String(root.get<const char*>("PWD"));
        debugI("\nConfigureCharacteristicsCallbacks :: onWrite: SSID: %s", SSID->c_str());
        //debugD("\nConfigureCharacteristicsCallbacks :: onWrite: PASSWD: %s", PASSWD->c_str());
        //Reset ESP32 Board to connect to given WiFi Credentials
        WiFi.disconnect();
        WiFi.begin(SSID->c_str(), PASSWD->c_str());
        while(WiFi.waitForConnectResult() != WL_CONNECTED) {
          debugE("\nConfigureCharacteristicsCallbacks :: onWrite: Connecting to SSID: %s failed! Rebooting...", SSID->c_str());
          delay(5000);
          ESP.restart();
        }
        //Update the provided WiFi config details onto SPIFFS
        debugI("\nConfigureCharacteristicsCallbacks :: onWrite: Board connected to SSID: %s", SSID->c_str());
        BluetoothService :: updateWiFiConfig(SSID->c_str(),PASSWD->c_str());
        //Release memory used
        delete receivedString;
        delete SSID;
        delete PASSWD;
        jsonBuffer.clear();
      }
      BluetoothService :: setClientConnected(false);
    }
};

#endif
