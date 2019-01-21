#include <mbedtls/pk.h>
#include <mbedtls/error.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include "key.h"

static char* mbedtlsError(int errnum) {
  static char buffer[200];
  mbedtls_strerror(errnum, buffer, sizeof(buffer));
  return buffer;
} // mbedtlsError

String encodeJWT(char* header, char* payload) {
  char base64Header[100];
  //  const char header[] = "{\"alg\":\"RS256\",\"typ\":\"JWT\"}";
  base64url_encode(
    (unsigned char *)header,   // Data to encode.
    strlen(header),            // Length of data to encode.
    base64Header);             // Base64 encoded data.

  //  time_t now;
  //  time(&now);
  //  uint32_t iat = now;              // Set the time now.
  //  uint32_t exp = iat + 60 * 60;    // Set the expiry time.

  //  char payload[100];
  //  sprintf(payload, "{\"iat\":%d,\"exp\":%d,\"aud\":\"%s\"}", "22", "33", "dsfgdsfgdsf");

  char base64Payload[100];
  base64url_encode(
    (unsigned char *)payload,  // Data to encode.
    strlen(payload),           // Length of data to encode.
    base64Payload);            // Base64 encoded data.

  uint8_t headerAndPayload[800];
  sprintf((char*)headerAndPayload, "%s.%s", base64Header, base64Payload);

  mbedtls_pk_context pk_context;
  mbedtls_pk_init(&pk_context);
  Serial.println("1113");
  int rc = mbedtls_pk_parse_key(
             &pk_context,
             SIGNING_PUB_KEY,
             strlen((const char*)SIGNING_PUB_KEY) + 1,
             nullptr,
             0);
  if (rc != 0) {
    printf("Failed to mbedtls_pk_parse_key: %d (-0x%x): %s\n", rc, -rc, mbedtlsError(rc));
    return "";
  }
  Serial.println("222");
  mbedtls_rsa_context *rsa;

  rsa = mbedtls_pk_rsa(pk_context);

  Serial.println("3333");
  uint8_t oBuf[500];

  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_ctr_drbg_init(&ctr_drbg);
  mbedtls_entropy_init(&entropy);

  const char* pers = "MyEntropy";
  Serial.println("333");
  mbedtls_ctr_drbg_seed(
    &ctr_drbg,
    mbedtls_entropy_func,
    &entropy,
    (const unsigned char*)pers,
    strlen(pers));
  Serial.println("4444");


  uint8_t digest[32];
  rc = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), headerAndPayload, strlen((char*)headerAndPayload), digest);
  if (rc != 0) {
    printf("Failed to mbedtls_md: %d (-0x%x): %s\n", rc, -rc, mbedtlsError(rc));
    return "";
  }

  Serial.println("5555");


  size_t retSize;
  rc = mbedtls_pk_sign(&pk_context, MBEDTLS_MD_SHA256, digest, sizeof(digest), oBuf, &retSize, mbedtls_ctr_drbg_random, &ctr_drbg);
  if (rc != 0) {
    printf("Failed to mbedtls_pk_sign: %d (-0x%x): %s\n", rc, -rc, mbedtlsError(rc));
    return "";
  }


  Serial.println("6666");


  char base64Signature[600];

  base64url_encode((unsigned char *)oBuf, retSize, base64Signature);

  char* retData = (char*)malloc(strlen((char*)headerAndPayload) + 1 + strlen((char*)base64Signature) + 1);

  sprintf(retData, "%s.%s", headerAndPayload, base64Signature);
  Serial.println("7777");
  delay(100);
  mbedtls_pk_free(&pk_context);
  return retData;
}
