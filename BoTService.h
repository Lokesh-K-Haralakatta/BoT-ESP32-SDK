/*
  BoTService.h - BoTService Class and Methods to make Secure HTTP Calls to BoT Service.
  Created by Lokesh H K, April 12, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#ifndef BoTEService_h
#define BoTEService_h

#include "BoTESP32SDK.h"
#include "base64url.h"
#include "Storage.h"
class BoTService {
  public:
    BoTService(const char* host, const char* uri, const int port);
    ~BoTService();
    String get(const char* endPoint);
    String post(const char* endPoint, const char* payload);
  private:
    char* hostURL;
    char* uriPath;
    int PORT;
    HTTPClient* httpClient;
    KeyStore* store;
    const char* mbedtlsError(int errnum);
    String encodeJWT(const char* header, const char* payload);
    String decodePayload(String encodedPayload);
};

#endif
