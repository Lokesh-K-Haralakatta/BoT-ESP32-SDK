/*
  Storage.h - Class and Methods to retrieve and store configuration data for BoT Service
  Created by Lokesh H K, April 9, 2019.
  Released into the repository BoT-ESP32-SDK.
*/
#ifndef Storage_h
#define Storage_h
#include "BoTESP32SDK.h"
#define JSON_CONFIG_FILE "/configuration.json"
#define PRIVATE_KEY_FILE "/private.key"
#define PUBLIC_KEY_FILE "/public.key"
#define API_KEY_FILE "/api.pem"
#define CA_CERT_FILE "/cacert.cer"
#define ACTIONS_FILE "/actions.json"
#define UUID_GEN_CA_CERT "/uuidGenCaCert.cer"
#define NOT_LOADED 0
#define LOADED 1
#define DEVICE_STATE_ADDR 0
class KeyStore {
  public:
    static KeyStore* getKeyStoreInstance();
    void loadJSONConfiguration();
    void initializeEEPROM();
    bool isJSONConfigLoaded();
    void retrieveAllKeys();
    bool isPrivateKeyLoaded();
    bool isPublicKeyLoaded();
    bool isAPIKeyLoaded();
    bool isCACertLoaded();
    bool isUUIDGenCACertLoaded();
    const char* getWiFiSSID();
    const char* getWiFiPasswd();
    const char* getMakerID();
    const char* getDeviceID();
    const char* getAlternateDeviceID();
    const char* getDevicePrivateKey();
    const char* getDevicePublicKey();
    const char* getAPIPublicKey();
    const char* getCACert();
    const char* getUUIDGenCACert();
    void setHTTPS(const bool https);
    const bool getHTTPS();
    void setDeviceState(int);
    void resetDeviceState();
    const int getDeviceState();
    std::vector <struct Action> retrieveActions();
    bool saveActions(std::vector <struct Action> aList);
  private:
    static KeyStore *store;
    String *wifiSSID;
    String *wifiPASSWD;
    String *https;
    String *makerID;
    String *deviceID;
    String *altDeviceID;
    String *privateKey;
    String *publicKey;
    String *apiKey;
    String *caCert;
    String *uuidCACert;
    byte jsonCfgLoadStatus;
    byte privateKeyLoadStatus;
    byte publicKeyLoadStatus;
    byte apiKeyLoadStatus;
    byte caCertLoadStatus;
    byte uuidGenCACertLoadStatus;
    void loadFileContents(const char* filePath, byte keyType);
    KeyStore();
    std::vector <struct Action> actionsList;
};

#endif
