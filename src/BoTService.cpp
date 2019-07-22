/*
  BoTService.cpp - BoTService Class and Methods to make Secure HTTP Calls to BoT Service.
  Created by Lokesh H K, April 12, 2019
  Released into the repository BoT-ESP32-SDK.
*/

#include "BoTService.h"
BoTService* BoTService::bot = NULL;

BoTService* BoTService :: getBoTServiceInstance(){
  if(bot == NULL){
    bot = new BoTService();
  }

  return bot;
}

BoTService :: BoTService(){
  hostURL = new char[strlen(HOST)+1];
  strcpy(hostURL,HOST);

  uriPath = new char[strlen(URI)+1];
  strcpy(uriPath,URI);

  https = true;

  wifiClient = NULL;
  httpClient = NULL;
  fullURI = NULL;
  botResponse = NULL;
  encodedJWTPayload = NULL;
  store = KeyStore :: getKeyStoreInstance();
}

BoTService :: ~BoTService(){
  if(uriPath != NULL) {
    delete uriPath;
    uriPath = NULL;
  }

  if(hostURL != NULL) {
    delete hostURL;
    hostURL = NULL;
  }

  if(botResponse != NULL){
    delete botResponse;
    botResponse = NULL;
  }

  freeObjects();
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
  debugD("\nBoTService :: encodeJWT: headerAndPayload contents: %s", (char*)headerAndPayload);

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
    debugE("\nBoTService :: encodeJWT: Failed to mbedtls_pk_parse_key: %d (-0x%x): %s", rc, -rc, mbedtlsError(rc));
    return "";
  }
  debugD("\nBoTService :: encodeJWT: Signing Key is parsed");

  mbedtls_rsa_context *rsa;
  rsa = mbedtls_pk_rsa(pk_context);
  debugD("\nBoTService :: encodeJWT: RSA context loaded");

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
  debugD("\nBoTService :: encodeJWT: mbedtls_ctr_drbg_seed is completed");

  uint8_t digest[32];
  rc = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), headerAndPayload, strlen((char*)headerAndPayload), digest);
  if (rc != 0) {
    debugE("\nBoTService :: encodeJWT: Failed to mbedtls_md: %d (-0x%x): %s", rc, -rc, mbedtlsError(rc));
    return "";
  }
  debugD("\nBoTService :: encodeJWT: mbedtls_md is completed");

  size_t retSize;
  uint8_t oBuf[500];
  rc = mbedtls_pk_sign(&pk_context, MBEDTLS_MD_SHA256, digest, sizeof(digest), oBuf, &retSize, mbedtls_ctr_drbg_random, &ctr_drbg);
  if (rc != 0) {
    debugE("\nBoTService :: encodeJWT: Failed to mbedtls_pk_sign: %d (-0x%x): %s", rc, -rc, mbedtlsError(rc));
    return "";
  }
  debugD("\nBoTService :: encodeJWT: mbedtls_pk_sign is completed");

  char base64Signature[600];
  base64url_encode((unsigned char *)oBuf, retSize, base64Signature);

  if(encodedJWTPayload != NULL){
    free(encodedJWTPayload);
    encodedJWTPayload = NULL;
    debugD("\nBoTService :: encodeJWT: Freed previous encodedPayload memory");
  }

  encodedJWTPayload = (char*)malloc(strlen((char*)headerAndPayload) + 1 + strlen((char*)base64Signature) + 1);
  sprintf(encodedJWTPayload, "%s.%s", headerAndPayload, base64Signature);
  debugD("\nBoTService :: encodeJWT: encoded return data is ready");

  delay(100);

  //Free memory allocated for mbedtls structures
  mbedtls_ctr_drbg_free(&ctr_drbg);
  mbedtls_entropy_free(&entropy);
  mbedtls_pk_free(&pk_context);
  debugD("\nBoTService :: encodeJWT: Freed memory allocated for mbedtls structures");

  return encodedJWTPayload;
}

String* BoTService :: get(const char* endPoint){
  store->loadJSONConfiguration();
  store->retrieveAllKeys();
  https = store->getHTTPS();

  fullURI = new String(uriPath);
  fullURI->concat(endPoint);
  debugD("\nBoTService :: get: URI: %s", fullURI->c_str());

  if(botResponse != NULL){
    delete botResponse;
    botResponse = NULL;
    debugD("\nBoTService :: get: botResponse memory released from previous GET call");
  }

  if(WiFi.status() == WL_CONNECTED){
    debugD("\nBoTService :: get: WiFi Status: Connected");

    wifiClient = new WiFiClientSecure();
    httpClient = new HTTPClient();

    if(https){
      const char* caCert = store->getCACert();
      if( caCert != NULL){
        //LOG("\nCA Certificate Contents: \n%s\n",caCert);
        debugD("\nBoTService :: get: CACert set to wifiClient");
        wifiClient->setCACert(caCert);
      }
      else {
        debugW("\nBoTService :: get: store->getCACert returned NULL, hence turning off https");
        https = false;
      }
    }

    bool httpClientBegin = false;
    if(https){
      httpClientBegin = httpClient->begin(*wifiClient,hostURL,HTTPS_PORT,fullURI->c_str(),true);
      if(httpClientBegin){
        debugI("\nBoTService :: get: HTTPClient initialized for HTTPS");
        if(!wifiClient->connect((char*)HOST, HTTPS_PORT)){
          debugE("\nBoTService :: get: wifiSecureClient connection to %s:%d failed",HOST, HTTPS_PORT);
          freeObjects();
          botResponse = new String("wifiSecureClient connection to server failed, can not verify SSL Finger Print");
          return botResponse;
        }
        else {
          debugI("\nBoTService :: get: wifiSecureClient connection to %s:%d successful",HOST, HTTPS_PORT);
          bool ssl_fg_verify = wifiClient->verify((char*)SSL_FINGERPRINT_SHA256, (char*)HOST);
          debugD("\nBoTService :: get: Return value from wifiClient->verify : %u",ssl_fg_verify);
          if(ssl_fg_verify == true)
            debugI("\nBoTService :: get: SSL Finger Print Verification Succeeded...");
          else {
            debugE("\nBoTService :: get: SSL Finger Print Verification Failed...");
            freeObjects();
            botResponse = new String("SSL Finger Print Verification Failed in BoTService GET");
            return botResponse;
          }
        }
      }
      else
        debugE("\nBoTService :: get: HTTPClient initialization failed for HTTPS");
    }
    else {
      httpClientBegin = httpClient->begin(hostURL,HTTP_PORT,fullURI->c_str());
      if(httpClientBegin)
        debugI("\nBoTService :: get: HTTPClient initialized for HTTP");
      else
        debugW("\nBoTService :: get: HTTPClient initialization failed for HTTP");
    }

    if(httpClientBegin){
      httpClient->addHeader("makerID", store->getMakerID());
      httpClient->addHeader("deviceID", store->getDeviceID());

      debugD("\nBoTService :: get: Making httpClient->GET call");
      int httpCode = httpClient->GET();
      debugD("\nBoTService :: get: httpCode from httpClient->GET(): %d",httpCode);

      String* payload = new String(httpClient->getString());
      httpClient->end();

      //Deallocate memory allocated for objects
      freeObjects();

      if(httpCode >= 0) {

        debugI("\nBoTService :: get: HTTP GET with endPoint %s, return code: %d", endPoint, httpCode);

        if(httpCode == HTTP_CODE_OK) {
          int firstDoTIdx = payload->indexOf(".");
          if (firstDoTIdx != -1) {
            int secDoTIdx = payload->indexOf(".",firstDoTIdx+1);
            String* encodedPayload = new String(payload->substring(firstDoTIdx+1 , secDoTIdx));
            delete payload;
            botResponse = decodePayload(encodedPayload);
            delete encodedPayload;
            debugD("\nBoTService :: get: botResponse: \n%s\n",botResponse->c_str());
            return(botResponse);
          }
          else {
            debugE("\nBoTService :: get: Response body is not as expected:\n%s\n",payload->c_str());
            botResponse = new String("Response body is not as expected");
            return botResponse;
          }
        }
        else {
          debugE("\nBoTService :: get: HTTP GET with endpoint %s failed with status code: %d", endPoint,httpCode);
          botResponse = new String("HTTP GET failed with error message: ");
          botResponse->concat(httpClient->errorToString(httpCode));
          return botResponse;
        }
      }
      else {
        debugE("\nBoTService :: get: HTTP GET with endPoint %s failed with status code: %d", endPoint,httpCode);
        botResponse = new String(httpClient->errorToString(httpCode));
        return botResponse;
      }
    }
    else {
      debugE("\nBoTService :: get: httpClient->begin failed....");
      //Deallocate memory allocated for objects
      freeObjects();
      botResponse = new String("httpClient->begin failed....");
      return botResponse;
    }
  }
  else {
    LOG("\nBoTService :: get: Board Not Connected to WiFi...");
    botResponse = new String("Board Not Connected to WiFi...");
    return botResponse;
  }
}

String* BoTService :: decodePayload(String* encodedPayload){
  int payloadLength = encodedPayload->length() + 1;
  unsigned char* decoded = new unsigned char[payloadLength];
  //String ss(encodedPayload);

  base64url_decode((char *)encodedPayload->c_str(), encodedPayload->length() , decoded);
  debugD("\nBoTService :: decodePayload: Decoded String: %s", decoded);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(decoded);
  String* botValue = new String(root.get<const char*>("bot"));
  delete decoded;
  jsonBuffer.clear();
  debugD("\nBoTService :: decodePayload: botValue: %s",botValue->c_str());

  return botValue;
}

String* BoTService :: post(const char* endPoint, const char* payload){

  char* jwtHeader = "{\"alg\":\"RS256\",\"typ\":\"JWT\"}";
  debugD("\nBoTService :: post: jwtHeader: %s", jwtHeader);
  debugD("\nBoTService :: post: Given Payload: %s", payload);

  //Prepare Full URI
  fullURI = new String(uriPath);
  fullURI->concat(endPoint);

  if(botResponse != NULL){
    delete botResponse;
    botResponse = NULL;
    debugD("\nBoTService :: post: botResponse memory released from previous call");
  }

  if(WiFi.status() == WL_CONNECTED){
    debugD("\nBoTService :: post: Everything good to make POST Call to BoT Service: %s", fullURI->c_str());

    wifiClient = new WiFiClientSecure();
    httpClient = new HTTPClient();
    https = store->getHTTPS();

    if(https){
      const char* caCert = store->getCACert();
      if(caCert != NULL){
        wifiClient->setCACert(caCert);
        debugD("\nBoTService :: post: CACert set to wifiClient");
      }
      else {
        debugW("\nBoTService :: post: store->getCACert returned NULL, hence turning off https");
        https = false;
      }
    }

    bool httpClientBegin = false;
    if(https){
      httpClientBegin = httpClient->begin(*wifiClient,hostURL,HTTPS_PORT,fullURI->c_str(),true);
      if(httpClientBegin){
        debugI("\nBoTService :: post: HTTPClient initialized for HTTPS");
        if(!wifiClient->connect((char*)HOST, HTTPS_PORT)){
          debugE("\nBoTService :: post: wifiSecureClient connection to %s:%d failed",HOST, HTTPS_PORT);
          freeObjects();
          botResponse = new String("wifiSecureClient connection to server failed, can not verify SSL Finger Print");
          return botResponse;
        }
        else {
          debugI("\nBoTService :: post: wifiSecureClient connection to %s:%d successful",HOST, HTTPS_PORT);
          bool ssl_fg_verify = wifiClient->verify((char*)SSL_FINGERPRINT_SHA256, (char*)HOST);
          debugD("\nBoTService :: post: Return value from wifiClient->verify : %u",ssl_fg_verify);
          if(ssl_fg_verify == true)
            debugI("\nBoTService :: post: SSL Finger Print Verification Succeeded...");
          else {
            debugE("\nBoTService :: post: SSL Finger Print Verification Failed...");
            freeObjects();
            botResponse = new String("SSL Finger Print Verification Failed in BoTService POST");
            return botResponse;
          }
        }
      }
      else
        debugE("\nBoTService :: post: HTTPClient initialization failed for HTTPS");
    }
    else {
      httpClientBegin = httpClient->begin(hostURL,HTTP_PORT,fullURI->c_str());
      if(httpClientBegin)
        debugI("\nBoTService :: post: HTTPClient initialized for HTTP");
      else
        debugW("\nBoTService :: post: HTTPClient initialization failed for HTTP");
    }

    if(httpClientBegin){
      //Encode the given payload using jwtHeader
      String* botJWT = new String(encodeJWT(jwtHeader, payload));

      //Prepare body contents for Post
      String* body = new String("{\"bot\": \"");
      body->concat(botJWT->c_str());
      body->concat("\"}");

      delete botJWT;
      debugD("\nBoTService :: post: body contents after encoding: %s", body->c_str());

      httpClient->addHeader("makerID", store->getMakerID());
      httpClient->addHeader("deviceID", store->getDeviceID());
      httpClient->addHeader("Content-Type", "application/json");
      httpClient->addHeader("Connection","keep-alive");
      httpClient->addHeader("Content-Length",String(body->length()));

      int httpCode = httpClient->POST(body->c_str());
      delete body;

      String* payload = new String(httpClient->getString());
      httpClient->end();

      //Deallocate memory allocated for objects
      freeObjects();

      if(httpCode >= 0) {

        debugI("\nBoTService :: post: HTTP POST with endPoint %s, return code: %d", endPoint, httpCode);

        if(httpCode == HTTP_CODE_OK) {
          int firstDoTIdx = payload->indexOf(".");
          if (firstDoTIdx != -1) {
            int secDoTIdx = payload->indexOf(".",firstDoTIdx+1);
            String* encodedPayload = new String(payload->substring(firstDoTIdx+1 , secDoTIdx));
            delete payload;
            botResponse = decodePayload(encodedPayload);
            delete encodedPayload;
            return(botResponse);
          }
        }
        else {
          debugE("\nBoTService :: post: HTTP POST with endpoint %s failed with status code %d", endPoint,httpCode);
          botResponse = new String("HTTP POST failed with error message: ");
          botResponse->concat(httpClient->errorToString(httpCode));
          return botResponse;
        }
      }
      else {
        debugE("\nBoTService :: post: HTTP POST with endPoint %s failed with status code %d", endPoint, httpCode);
        botResponse = new String(httpClient->errorToString(httpCode));
        return botResponse;
      }
    }
    else {
      debugE("\nBoTService :: post: httpClient->begin failed....");
      //Deallocate memory allocated for objects
      freeObjects();
      botResponse = new String("httpClient->begin failed....");
      return botResponse;
    }
  }
  else {
    LOG("\nBoTService :: post: Board Not Connected to WiFi...");
    botResponse = new  String("Board Not Connected to WiFi...");
    return botResponse;
  }
}

void BoTService :: freeObjects(){
  if(httpClient != NULL) {
     debugD("\nBoTService :: freeObjects : Releasing HTTPClient Instance...");
     httpClient->end();
     delete httpClient;
     httpClient = NULL;
   }

  if(wifiClient != NULL) {
    debugD("\nBoTService :: freeObjects : Releasing WiFiClientSecure Instace...");
    wifiClient->stop();
    delete wifiClient;
    wifiClient = NULL;
  }

  if(fullURI != NULL) {
    delete fullURI;
    fullURI = NULL;
  }

  debugD("\nBoTService :: freeObjects : Objects memory freed");
}
