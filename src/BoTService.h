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

#define HOST "iot.bankingofthings.io"
#define URI ""
#define HTTP_PORT 80
#define HTTPS_PORT 443
#define SSL_FINGERPRINT_SHA256 "FB:89:FB:DF:92:0C:AD:CB:65:B0:FD:5A:51:32:C4:94:C7:D9:C1:50:92:FA:3C:F0:B6:F4:3B:2D:8E:38:AE:F8"

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
