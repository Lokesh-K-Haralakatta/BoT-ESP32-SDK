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

#define HOST "api-dev.bankingofthings.io"
#define URI "/bot_iot"
#define HTTP_PORT 80
#define HTTPS_PORT 443

class BoTService {
  public:
    BoTService();
    ~BoTService();
    String get(const char* endPoint);
    String post(const char* endPoint, const char* payload);
  private:
    char* hostURL;
    char* uriPath;
    int port;
    bool https;
    WiFiClientSecure* wifiClient;
    HTTPClient* httpClient;
    KeyStore* store;
    const char* mbedtlsError(int errnum);
    String encodeJWT(const char* header, const char* payload);
    String decodePayload(String encodedPayload);
};

#endif
