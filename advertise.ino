/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/


//preserve this ORDER!!!!!
#define SERVICE_UUID        "729BE9C4-3C61-4EFB-884F-B310B6FFFFD1"
#define DEVICE_CHARACTERISTIC_UUID "CAD1B513-2DA4-4609-9908-234C6D1B2A9C"
#define DEVICE_INFO_CHARACTERISTIC_UUID "CD1B3A04-FA33-41AA-A25B-8BEB2D3BEF4E"
#define DEVICE_NETWORK_CHARACTERISTIC_UUID "C42639DC-270D-4690-A8B3-6BA661C6C899"
#define CONFIGURE_CHARACTERISTIC_UUID "32BEAA1B-D20B-47AC-9385-B243B8071DE4"


void startBLE(){
 
  Serial.println("Starting BLE work!");

  BLEDevice::init("botf1");
  BLEServer *pServer = BLEDevice::createServer();
  //this is to show in the list
  BLEService *pService = pServer->createService(SERVICE_UUID);


  BLECharacteristic *pDeviceCharacteristic = pService->createCharacteristic(
                                         DEVICE_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ
                                       );
  std::string val;
  val="{";
  val=val+"\"deviceID\":\""+deviceID+"\"";
  val=val+",\"makerID\":\""+makerID+"\"";
  val=val+",\"publicKey\":\""+publicKey+"\"";
  val=val+"}";
  Serial.println("Set characteristic:" + String(DEVICE_CHARACTERISTIC_UUID) + " to:" + String(val.c_str()));
  pDeviceCharacteristic->setValue(val);
  


  // 
  BLECharacteristic *pDeviceInfoCharacteristic = pService->createCharacteristic(
                                         DEVICE_INFO_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ
                                       );

  pDeviceInfoCharacteristic->setValue("{}");

  BLECharacteristic *pNetworkharacteristic = pService->createCharacteristic(
                                         DEVICE_NETWORK_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ
                                       );
  pNetworkharacteristic->setValue("{}");

  BLECharacteristic *pConfigureCharacteristic = pService->createCharacteristic(
                                         CONFIGURE_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pConfigureCharacteristic->setValue("{}");



  pService->start();

  // Start advertising
  pServer->getAdvertising()->addServiceUUID(pService->getUUID());
  pServer->getAdvertising()->start();
  
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}
