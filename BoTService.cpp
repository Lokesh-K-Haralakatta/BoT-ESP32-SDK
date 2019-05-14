/*
  BoTService.cpp - BoTService Class and Methods to make Secure HTTP Calls to BoT Service.
  Created by Lokesh H K, April 12, 2019
  Released into the repository BoT-ESP32-SDK.
*/

#include "BoTService.h"

BoTService :: BoTService(){
  hostURL = new char[strlen(HOST)+1];
  strcpy(hostURL,HOST);

  uriPath = new char[strlen(URI)+1];
  strcpy(uriPath,URI);

  https = true;

  wifiClient = NULL;
  httpClient = NULL;
  store = KeyStore :: getKeyStoreInstance();
}

BoTService :: ~BoTService(){
  delete uriPath;
  delete hostURL;
}

const char* BoTService :: mbedtlsError(int errnum) {
  char buffer[200];
  mbedtls_strerror(errnum, buffer, sizeof(buffer));
  return buffer;
}

String BoTService :: encodeJWT(const char* header, const char* payload) {
  char base64Header[100];
  store->retrieveAllKeys();
  base64url_encode(
    (unsigned char *)header,   // Data to encode.
    strlen(header),            // Length of data to encode.
    base64Header);             // Base64 encoded data.

  char base64Payload[100];
  base64url_encode(
    (unsigned char *)payload,  // Data to encode.
    strlen(payload),           // Length of data to encode.
    base64Payload);            // Base64 encoded data.

  uint8_t headerAndPayload[800];
  sprintf((char*)headerAndPayload, "%s.%s", base64Header, base64Payload);
  LOG("\nBoTService :: encodeJWT: headerAndPayload contents: %s", (char*)headerAndPayload);

  mbedtls_pk_context pk_context;
  mbedtls_pk_init(&pk_context);
  int rc = mbedtls_pk_parse_key(
             &pk_context,
             //(unsigned char *)SIGNING_KEY,
             //strlen((const char*)SIGNING_KEY) + 1,
             (unsigned char*) store->getDevicePrivateKey(),
             strlen((const char*)store->getDevicePrivateKey())+1,
             nullptr,
             0);
  if (rc != 0) {
    LOG("\nBoTService :: encodeJWT: Failed to mbedtls_pk_parse_key: %d (-0x%x): %s", rc, -rc, mbedtlsError(rc));
    return "";
  }
  LOG("\nBoTService :: encodeJWT: Signing Key is parsed");

  mbedtls_rsa_context *rsa;
  rsa = mbedtls_pk_rsa(pk_context);
  LOG("\nBoTService :: encodeJWT: RSA context loaded");

  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_ctr_drbg_init(&ctr_drbg);
  mbedtls_entropy_init(&entropy);

  const char* pers = "MyEntropy";
  mbedtls_ctr_drbg_seed(
    &ctr_drbg,
    mbedtls_entropy_func,
    &entropy,
    (const unsigned char*)pers,
    strlen(pers));
  LOG("\nBoTService :: encodeJWT: mbedtls_ctr_drbg_seed is completed");

  uint8_t digest[32];
  rc = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), headerAndPayload, strlen((char*)headerAndPayload), digest);
  if (rc != 0) {
    LOG("\nBoTService :: encodeJWT: Failed to mbedtls_md: %d (-0x%x): %s", rc, -rc, mbedtlsError(rc));
    return "";
  }
  LOG("\nBoTService :: encodeJWT: mbedtls_md is completed");

  size_t retSize;
  uint8_t oBuf[500];
  rc = mbedtls_pk_sign(&pk_context, MBEDTLS_MD_SHA256, digest, sizeof(digest), oBuf, &retSize, mbedtls_ctr_drbg_random, &ctr_drbg);
  if (rc != 0) {
    LOG("\nBoTService :: encodeJWT: Failed to mbedtls_pk_sign: %d (-0x%x): %s", rc, -rc, mbedtlsError(rc));
    return "";
  }
  LOG("\nBoTService :: encodeJWT: mbedtls_pk_sign is completed");

  char base64Signature[600];
  base64url_encode((unsigned char *)oBuf, retSize, base64Signature);
  char* retData = (char*)malloc(strlen((char*)headerAndPayload) + 1 + strlen((char*)base64Signature) + 1);
  sprintf(retData, "%s.%s", headerAndPayload, base64Signature);
  LOG("\nBoTService :: encodeJWT: encoded return data is ready");

  delay(100);
  mbedtls_pk_free(&pk_context);

  return retData;
}

String BoTService :: get(const char* endPoint){
  store->loadJSONConfiguration();
  store->retrieveAllKeys();
  https = store->getHTTPS();

  String *fullURI = new String(uriPath);
  fullURI->concat(endPoint);
  LOG("\nBoTService :: get: URI: %s", fullURI->c_str());

  if(WiFi.status() == WL_CONNECTED){
    LOG("\nBoTService :: get: WiFi Status: Connected");

    wifiClient = new WiFiClientSecure();
    httpClient = new HTTPClient();

    if(https){
      if(store->getCACert() != NULL){
        wifiClient->setCACert(store->getCACert());
        LOG("\nBoTService :: get: CACert set to wifiClient");
      }
      else {
        LOG("\nBoTService :: get: store->getCACert returned NULL, hence turning off https");
        https = false;
      }
    }

    bool httpClientBegin = false;
    if(https){
      httpClientBegin = httpClient->begin(*wifiClient,hostURL,HTTPS_PORT,fullURI->c_str(),true);
      if(httpClientBegin)
        LOG("\nBoTService :: get: HTTPClient initialized for HTTPS");
      else
        LOG("\nBoTService :: get: HTTPClient initialization failed for HTTPS");
    }
    else {
      httpClientBegin = httpClient->begin(hostURL,HTTP_PORT,fullURI->c_str());
      if(httpClientBegin)
        LOG("\nBoTService :: get: HTTPClient initialized for HTTP");
      else
        LOG("\nBoTService :: post: HTTPClient initialization failed for HTTP");
    }

    if(httpClientBegin){
      httpClient->addHeader("makerID", store->getMakerID());
      httpClient->addHeader("deviceID", store->getDeviceID());

      LOG("\nBoTService :: get: Making httpClient->GET call");
      int httpCode = httpClient->GET();
      LOG("\nBoTService :: get: httpCode from httpClient->GET(): %d",httpCode);

      const char* errorMSG = httpClient->errorToString(httpCode).c_str();

      String payload = httpClient->getString();
      httpClient->end();

      delete httpClient;
      delete wifiClient;
      delete fullURI;

      if(httpCode > 0) {

        LOG("\nBoTService :: get: HTTP GET with endPoint %s, return code: %d", endPoint, httpCode);

        if(httpCode == HTTP_CODE_OK) {
          int firstDoTIdx = payload.indexOf(".");
          if (firstDoTIdx != -1) {
            int secDoTIdx = payload.indexOf(".",firstDoTIdx+1);
            String encodedPayload = payload.substring(firstDoTIdx+1 , secDoTIdx);
            return(decodePayload(encodedPayload));
          }
        }
        else {
          LOG("\nBoTService :: get: HTTP GET with endpoint %s, failed, error: %s", endPoint, errorMSG);
          return errorMSG;
        }
      }
      else {
        LOG("\nBoTService :: get: HTTP GET with endPoint %s, failed, error: %s", endPoint, errorMSG);
        return errorMSG;
      }
    }
    else {
      LOG("\nBoTService :: get: httpClient->begin failed....");
      return "httpClient->begin failed....";
    }
  }
  else {
    LOG("\nBoTService :: get: Board Not Connected to WiFi...");
    return "Board Not Connected to WiFi...";
  }
}

String BoTService :: decodePayload(String encodedPayload){
  int payloadLength = encodedPayload.length() + 1;
  unsigned char decoded[payloadLength];
  String ss(encodedPayload);

  base64url_decode((char *)ss.c_str(), ss.length() , decoded);
  LOG("\nBoTService :: decodePayload: Decoded String: %s", decoded);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(decoded);
  String botValue = root["bot"];
  LOG("\nBoTService :: decodePayload: response value: %s",botValue.c_str());

  return botValue;
}

String BoTService :: post(const char* endPoint, const char* payload){

  char* jwtHeader = "{\"alg\":\"RS256\",\"typ\":\"JWT\"}";
  LOG("\nBoTService :: post: jwtHeader: %s", jwtHeader);
  LOG("\nBoTService :: post: Given Payload: %s", payload);

  String botJWT = encodeJWT(jwtHeader, payload);
  String body = (String)"{\"bot\": \"" + botJWT +   "\"}";

  LOG("\nBoTService :: post: body contents after encoding: %s", body.c_str());
  String *fullURI = new String(uriPath);
  fullURI->concat(endPoint);

  if(WiFi.status() == WL_CONNECTED){
    LOG("\nBoTService :: post: Everything good to make POST Call to BoT Service: %s", fullURI->c_str());

    wifiClient = new WiFiClientSecure();
    httpClient = new HTTPClient();
    https = store->getHTTPS();

    if(https){
      if(store->getCACert() != NULL){
        wifiClient->setCACert(store->getCACert());
        LOG("\nBoTService :: post: CACert set to wifiClient");
      }
      else {
        LOG("\nBoTService :: post: store->getCACert returned NULL, hence turning off https");
        https = false;
      }
    }

    bool httpClientBegin = false;
    if(https){
      httpClientBegin = httpClient->begin(*wifiClient,hostURL,HTTPS_PORT,fullURI->c_str(),true);
      if(httpClientBegin)
        LOG("\nBoTService :: post: HTTPClient initialized for HTTPS");
      else
        LOG("\nBoTService :: post: HTTPClient initialization failed for HTTPS");
    }
    else {
      httpClientBegin = httpClient->begin(hostURL,HTTP_PORT,fullURI->c_str());
      if(httpClientBegin)
        LOG("\nBoTService :: post: HTTPClient initialized for HTTP");
      else
        LOG("\nBoTService :: post: HTTPClient initialization failed for HTTP");
    }

    if(httpClientBegin){
      httpClient->addHeader("makerID", store->getMakerID());
      httpClient->addHeader("deviceID", store->getDeviceID());
      httpClient->addHeader("Content-Type", "application/json");
      httpClient->addHeader("Connection","keep-alive");
      httpClient->addHeader("Content-Length",String(body.length()));

      int httpCode = httpClient->POST(body);
      const char* errorMSG = httpClient->errorToString(httpCode).c_str();

      String payload = httpClient->getString();
      httpClient->end();

      delete httpClient;
      delete wifiClient;
      delete fullURI;

      if(httpCode > 0) {

        LOG("\nBoTService :: post: HTTP POST with endPoint %s, return code: %d", endPoint, httpCode);

        if(httpCode == HTTP_CODE_OK) {
          int firstDoTIdx = payload.indexOf(".");
          if (firstDoTIdx != -1) {
            int secDoTIdx = payload.indexOf(".",firstDoTIdx+1);
            String encodedPayload = payload.substring(firstDoTIdx+1 , secDoTIdx);
            return(decodePayload(encodedPayload));
          }
        }
        else {
          LOG("\nBoTService :: post: HTTP POST with endpoint %s, failed, error: %s", endPoint, errorMSG);
          return String(errorMSG);
        }
      }
      else {
        LOG("\nBoTService :: post: HTTP POST with endPoint %s, failed, error: %s", endPoint, errorMSG);
        return String(errorMSG);
      }
    }
    else {
      LOG("\nBoTService :: post: httpClient->begin failed....");
      return "httpClient->begin failed....";
    }
  }
  else {
    LOG("\nBoTService :: post: Board Not Connected to WiFi...");
    return String("Board Not Connected to WiFi...");
  }
}
