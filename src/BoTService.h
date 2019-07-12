/*
  BoTService.h - BoTService Class and Methods to make Secure HTTP Calls to BoT Service.
  Created by Lokesh H K, April 12, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#ifndef BoTService_h
#define BoTService_h

#include "BoTESP32SDK.h"
#include "base64url.h"
#include "Storage.h"

#define HOST "api.bankingofthings.io"
#define URI "/bot_iot"
#define HTTP_PORT 80
#define HTTPS_PORT 443
#define SSL_FINGERPRINT_SHA256 "85:76:3F:1D:FF:FD:E3:79:1E:52:CE:50:77:6B:7B:50:A1:5A:E0:F0:6A:80:48:19:EC:A9:7A:B2:2C:E3:49:B5"
#define SSL_FINGERPRINT_SHA1 "3E:A2:2B:BF:FB:38:A6:76:9A:30:D6:95:1B:F0:A9:BB:9A:84:7D:D6"

class BoTService {
  public:
    static BoTService* getBoTServiceInstance();
    String* get(const char* endPoint);
    String* post(const char* endPoint, const char* payload);
  private:
    static BoTService *bot;
    char* hostURL;
    char* uriPath;
    char* encodedJWTPayload;
    int port;
    bool https;
    WiFiClientSecure* wifiClient;
    HTTPClient* httpClient;
    KeyStore* store;
    String *fullURI;
    String *botResponse;
    const char* mbedtlsError(int errnum);
    String encodeJWT(const char* header, const char* payload);
    String* decodePayload(String* encodedPayload);
    void freeObjects();
    BoTService();
    ~BoTService();
};

#endif
