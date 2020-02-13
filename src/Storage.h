/*
  Storage.h - Class and Methods to retrieve and store configuration data for BoT Service
  Created by Lokesh H K, April 9, 2019.
  Released into the repository BoT-ESP32-SDK.
*/
#ifndef Storage_h
#define Storage_h
#include "BoTESP32SDK.h"
#include "QrCode.hpp"
#define JSON_CONFIG_FILE "/configuration.json"
#define PRIVATE_KEY_FILE "/private.key"
#define PUBLIC_KEY_FILE "/public.key"
#define API_KEY_FILE "/api.pem"
#define CA_CERT_FILE "/cacert.cer"
#define ACTIONS_FILE "/actions.json"
#define QRCODE_FILE "/qrcode.svg"
#define OFFLINE_ACTIONS_FILE "/offline.json"
#define NOT_LOADED 0
#define LOADED 1
#define DEVICE_STATE_ADDR 0
class KeyStore {
  public:
    static KeyStore* getKeyStoreInstance();
    void loadJSONConfiguration();
    void initializeEEPROM();
    void retrieveAllKeys();
    bool isJSONConfigLoaded();
    bool isPrivateKeyLoaded();
    bool isPublicKeyLoaded();
    bool isAPIKeyLoaded();
    bool isCACertLoaded();
    bool isQRCodeGeneratedandSaved();
    bool isDeviceMultipair();
    bool offlineActionsExist();
    const char* getWiFiSSID();
    const char* getWiFiPasswd();
    const char* getMakerID();
    const char* getDeviceID();
    const char* getAlternateDeviceID();
    const char* getDevicePrivateKey();
    const char* getDevicePublicKey();
    const char* getAPIPublicKey();
    const char* getCACert();
    const char* generateUuid4();
    void setHTTPS(const bool https);
    const bool getHTTPS();
    const char* getDeviceName();
    void setDeviceName(const char* dName);
    void setDeviceState(int);
    void resetDeviceState();
    const int getDeviceState();
    const char* getDeviceStatusMsg();
    std::vector <struct Action> retrieveActions();
    bool saveActions(std::vector <struct Action> aList);
    String *getDeviceInfo();
    bool generateAndSaveQRCode();
    bool resetQRCodeStatus();
    std::vector <struct OfflineActionMetadata> retrieveOfflineActions(bool removeFile = false);
    bool saveOfflineActions(std::vector <struct OfflineActionMetadata> aList);
    bool saveOfflineAction(const char* actionID, const char* value, const unsigned long paymentTime);
    bool clearOfflineActions();
    bool updateWiFiConfiguration(const char* ssid, const char* passwd);
  private:
    static KeyStore *store;
    String *wifiSSID;
    String *wifiPASSWD;
    String *https;
    String *multipair;
    String *makerID;
    String *deviceID;
    String *deviceName;
    String *deviceInfo;
    String *deviceStatus;
    String *altDeviceID;
    String *privateKey;
    String *publicKey;
    String *apiKey;
    String *caCert;
    String *qrCACert;
    String *uuidStr;
    byte jsonCfgLoadStatus;
    byte privateKeyLoadStatus;
    byte publicKeyLoadStatus;
    byte apiKeyLoadStatus;
    byte caCertLoadStatus;
    byte qrCACertLoadStatus;
    bool qrCodeStatus;
    void loadFileContents(const char* filePath, byte keyType);
    KeyStore();
    std::vector <struct Action> actionsList;
    std::vector <struct OfflineActionMetadata> offlineActionsList;
    bool saveQRCode(qrcodegen::QrCode qr);
    void clearActionsList();
    void clearOfflineActionsList();
};

#endif
