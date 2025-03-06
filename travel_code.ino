#include <SPI.h>
#include <MFRC522.h>

constexpr uint8_t RST_PIN = D2;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = D4;     // Configurable, see typical pin layout above

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

String tag;  // To store the UID of the scanned card
String storedTag = "1289613863";  // The tag that allows deduction

float initialAmount = 1000.00;  // Initial balance
float deductionAmount = 10.00; // Amount to deduct on each successful scan

void setup() {
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
}

void loop() {
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  if (rfid.PICC_ReadCardSerial()) {
    tag = "";

    // Convert UID bytes to string for comparison
    for (byte i = 0; i < rfid.uid.size; i++) {
      tag += String(rfid.uid.uidByte[i], DEC);
    }

    Serial.println("Read Tag: " + tag);

    // Check if the tag matches the storedTag
    if (tag == storedTag) {
      // If the balance is greater than or equal to the deduction amount
      if (initialAmount >= deductionAmount) {
        initialAmount -= deductionAmount;  // Deduct the amount
        Serial.println("Access Granted!");
        Serial.print("Amount Deducted: Rs.");
        Serial.println(deductionAmount);
        Serial.print("Remaining Amount: Rs.");
        Serial.println(initialAmount);
      } else {
        Serial.println("Insufficient funds!");
      }
    } else {
      Serial.println("Access Denied!");
    }

    // Halt the RFID card and stop encryption
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
}