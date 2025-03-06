#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

constexpr uint8_t RST_PIN = D4;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = D8;     // Configurable, see typical pin layout above
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ D1, /* data=*/ D2, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display

#define WIFI_SSID "Chinni"
#define WIFI_PASSWORD "bhoomi9549"
#define API_KEY "AIzaSyCGXhY7CooyQbGbptGFIGwoC0qfqgZPKdI"
#define DATABASE_URL "https://rfid-smart-travel-card-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
String intValue;

void setup() {
  Serial.begin(115200);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522

  u8g2.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}
   
void loop() {
  if (!rfid.PICC_IsNewCardPresent())
    return;
  if (rfid.PICC_ReadCardSerial()) {
    String tag;
    for (byte i = 0; i < 4; i++) {
      tag += rfid.uid.uidByte[i];
    }
    Serial.println("Detected Card UID: " + tag);
    // Check if the detected UID is in the list of authorized UIDs
    delay(100);
    u8g2.clearBuffer();          // clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    u8g2.drawStr(0,10,"Verifying");  // write something to the internal memory
    u8g2.sendBuffer(); 
    delay(100);
    
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    if (Firebase.RTDB.setString(&fbdo, "smarttravel/check1", tag)){
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("Failed REASON: " + fbdo.errorReason());
    }
    // transfer internal memory to the display
    delay(4000);
    
    if (Firebase.RTDB.getString(&fbdo, "/smarttravel/access1"))
    {
      intValue = fbdo.stringData();
      String mySubString = intValue.substring(2, 3);
      Serial.println(intValue);
      Serial.println(mySubString);
      if (mySubString == "a")
      {
        u8g2.clearBuffer();          // clear the internal memory
        u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
        u8g2.drawStr(0,10,"Access Approved");  // write something to the internal memory
        u8g2.sendBuffer();          // transfer internal memory to the display
        delay(5000); 
        u8g2.clearBuffer();          // clear the internal memory
        u8g2.sendBuffer();          // transfer internal memory to the display
        delay(100); 
      }
      else if (mySubString == "b")
      {
        u8g2.clearBuffer();          // clear the internal memory
        u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
        u8g2.drawStr(0,10,"Access Denied");  // write something to the internal memory
        u8g2.drawStr(0,40,"Please check your");  // write something to the internal memory  
        u8g2.drawStr(0,50,"balance");  // write something to the internal memory
        u8g2.sendBuffer();          // transfer internal memory to the display
        delay(5000); 
        u8g2.clearBuffer();          // clear the internal memory
        u8g2.sendBuffer(); 
        delay(100);  
      }  
    delay(100);
    }
    else {
      Serial.println(fbdo.errorReason());
    }
    delay(100);
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();   
    }
  }
