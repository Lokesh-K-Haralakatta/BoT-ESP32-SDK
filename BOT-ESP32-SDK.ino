#include <WiFi.h>

int touch_value = 100;
int LED_BUILTIN=2;
#define TOUTCH_PIN T0 // ESP32 Pin D4

int buttonPressed=1; // the reset button is NOT pressed


const char* ssid = "The Office Operators";
const char* password = "2019TOO!";

std::string makerID = "185d8549-0091-463e-90fe-eda6ae15dc91";
std::string deviceID = "edfc7678-cacf-44f0-a2c6-1be15abef444";
std::string privateKey = "MIIEpAIBAAKCAQEA60+EWKVdGO1PJVs6k41sk4g3ASB6IMr3dvkNUILzhNa/hyupM8gT+zbPAF7g1cWElbrhSh9yh0GS7o+mlaBtzPD3bBXxJ/QmqQVqHUgMBUv0gouhakQ6XO9wrcELfQjQ+dY0HOZxDOqc/ms61ZqPGr91FxLk2mRZS947CA55T8wi19hMtKwEiIjAgUC0EjzJNglP0EHlWLrjW2T05sNE+55uPu1JXtbUiDQjhQfPa3g3XECSdY2kC/mzui6e8oNZVuX4OwELwvCU28SoaWEtp6lklI+5LPSgs2PNrrA5SER1Mxd0gk992TAKzb45FTNp0H24ihvzGHEo7Vmy9TLl9QIDAQABAoIBAQDY7WBj3Gab8UHyfUzd9BWxWHTkzAzqqFJEUsrsexyGTPcaZKzCuON6xwdEHzOEv2dAJZ1FmQIbIN+Un8eBzvkKYualtQCxxYnSbdRqlB3I0EHkC1dOQQHQ5UfqVroBlZwZhlhagDKeJrd+cFo3yxm8Hd6S2+8wSixsHb4r6jP508GCpcb2MWZA7wOnpEVEmsUGhxaIcwBt7/ZQwKwEIfS0ZMpf9ZBVFHgxKdMFITKjPIzYC2TSYx6MaDitZnHVpNklzh2H18pT+iUq03GpshrVg3moXmr8lYcobUYqqonee4lLy9pX2aBGu94bh2koFb38jg5KaKapj42FaRTQlJvhAoGBAPkL86m9pFB8uKmlcOMeNxfITrN+d0cTtEjxLp6nSbWLLyu1bHycVTp0kQIfom49FjxQJNZZX5xwec+E/5FnbugkYnrZI+8CfcDKTDFBD5HEIDoNlknflZzCMa3Bs9FNtQ7rEA5Uvx73PvP5bOqbneUQJiv0sJ8ECGE+Io9elUPpAoGBAPHhYyYbY1xkxai0hCGkoRt5+VnRfZCgRLT/BOgQbMXoOztqWNziSDDN+RpTCQSJbD0WB+jQS7QGrClaLsOG9VibISJQ8Fx8F3xvKU2+CaIgq0iR3RptL62kl/JgOhFxJCb51aupQLGGFR5xJLCh2cv6S1NPu/VJu9iZfRYH2oYtAoGAb4tL2iWnN6l05/7CfebllBBSN5CYCcyofdcQI10X5WjueqwqZ/F5NtJItGPI04kmsWLP7PVgnme+FxUhhlWGVhEdzpn8vyslebjhwVm7iHUIXhgBzRqIHwrXR1SvnjooswLTUSfh5hOlLY70mmuSn6rI/fz9MmuS+ULkJcSeJCkCgYBIFnBCTy9RvgbavNIUZMhPNA8IodLWY62Wc9q2fw6r/QaKMqvBSP9S7FAtdEnfvEUx/3rj/BBT2xk8SdBeHb7JZZJ+c7jvF1y4jTi2HskoNKxXuIE4+wmWqtNOPTeRMzMfGp5/xrWtMtTgecABlHsgu7vvXHgcJpmBeF07T6PlEQKBgQDarPAngDNc1oDq/JvmRncbmP0ufnbZ1FYAVsX9OV2mrETskqJTYj3moFn0BY6MN06dZQlCRVo0GIufTV6SGS4ML4MBb5OzmzJHauU0GSFYj1XnKpT++tdK24M1uh+akodRWzPvygfZKUdvw2P2TBuFODJTBqzkaB8OzHNk5abfqA==";
std::string publicKey = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA60+EWKVdGO1PJVs6k41sk4g3ASB6IMr3dvkNUILzhNa/hyupM8gT+zbPAF7g1cWElbrhSh9yh0GS7o+mlaBtzPD3bBXxJ/QmqQVqHUgMBUv0gouhakQ6XO9wrcELfQjQ+dY0HOZxDOqc/ms61ZqPGr91FxLk2mRZS947CA55T8wi19hMtKwEiIjAgUC0EjzJNglP0EHlWLrjW2T05sNE+55uPu1JXtbUiDQjhQfPa3g3XECSdY2kC/mzui6e8oNZVuX4OwELwvCU28SoaWEtp6lklI+5LPSgs2PNrrA5SER1Mxd0gk992TAKzb45FTNp0H24ihvzGHEo7Vmy9TLl9QIDAQAB";


String i_actions = "";
String i_status = "";


void setup() {
    Serial.begin(115200);
    delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

  startBLE();
  getPairing();
  getActions();
  postAction("77189283-A963-4E2E-BD12-18D1681A00EE", "12.01") ;


pinMode(LED_BUILTIN, OUTPUT);

}

void loop() {

  // put your main code here, to run repeatedly:
  touch_value = touchRead(TOUTCH_PIN);
  if (touch_value < 40){
    postAction("77189283-A963-4E2E-BD12-18D1681A00EE", "12.01") ;
  }

  // read the state of the pushbutton value:
    buttonPressed=digitalRead(0);

  if (buttonPressed==0){
     postAction("77189283-A963-4E2E-BD12-18D1681A00EE", "0.90") ;
  }
  
  delay(1000);
  
}
