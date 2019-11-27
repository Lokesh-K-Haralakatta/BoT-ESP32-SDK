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
#define SSL_FINGERPRINT_SHA256 "56:78:5D:C2:85:74:44:05:DC:A2:DC:37:C8:66:0E:E5:91:DF:C7:3A:7D:EF:24:C4:F3:41:62:50:AC:83:E0:B9"
#define SSL_FINGERPRINT_SHA1 "76:E6:B6:DF:6D:3B:4D:2D:48:D1:B6:32:AD:D6:8E:80:53:3F:5F:88"

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
