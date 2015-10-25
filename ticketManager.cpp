#include "ticketManager.h"
#include <Time.h>

// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield

#define TICKETS 0
#define YEAR 1
#define MONTH 2
#define DAY 3
#define HOUR 4
#define MINUTE 5
#define SECOND 6
#define CARD_TYPE 15

ticketManager::ticketManager(void) {
  nfc = new Adafruit_PN532(PN532_IRQ, PN532_RESET);
  nfc->begin();
  uint32_t versiondata = nfc->getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }  
  // configure board to read RFID tags
  nfc->SAMConfig();
  
  Serial.println("Waiting Card ...");
}

uint8_t ticketManager::getCardType(void) {
    success = nfc->mifareclassic_ReadDataBlock(4, data);
    return success ? data[CARD_TYPE] : -1;
}

uint8_t ticketManager::setCardType(uint8_t type) {
  success = nfc->mifareclassic_ReadDataBlock(4, data);
  if(success) {
    data[CARD_TYPE] = type;
    success = nfc->mifareclassic_WriteDataBlock(4, data);
  }
  return success ? data[CARD_TYPE] : -1;
}

uint8_t ticketManager::waitForCard(void) {

  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength,1000);
  
  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc->PrintHex(uid, uidLength);
      
    // Now we need to try to authenticate it for read/write access
    // Try with the factory default KeyA: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
    Serial.println("Trying to authenticate block 4 with default key value");
    uint8_t key[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  
    // Start with block 4 (the first block of sector 1) since sector 0
    // contains the manufacturer data and it's probably better just
    // to leave it alone unless you know what you're doing
    success = nfc->mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, key);
  }

  if (success) {
      Serial.println("Sector 1 (Blocks 4..7) has been authenticated");
    }
  return success;
}

uint8_t ticketManager::getTickets(void) {
  success = nfc->mifareclassic_ReadDataBlock(4, data);
  return success ? data[0] : -1;
}

void ticketManager::addTicketDate(void) {
  data[YEAR] = year() - 2000;
  data[MONTH] = month();
  data[DAY] = day();
  data[HOUR] = hour();
  data[MINUTE] = minute();
  data[SECOND] = second();
}

unsigned long ticketManager::getLastSwipeDate(void) {
  success = nfc->mifareclassic_ReadDataBlock(4, data);
  unsigned long timeEpoch;
  if(success) {
    TimeElements t;
    t.Year = data[1] + 2000 - 1970;
    t.Month = data[2];
    t.Day = data[3];
    t.Hour = data[4];
    t.Minute = data[5];
    t.Second = data[6];
    timeEpoch = makeTime(t);
  }
  return success ? timeEpoch : 0;
}

uint8_t ticketManager::addTickets(uint8_t t) {
  success = nfc->mifareclassic_ReadDataBlock(4, data);
  if(success) {
    data[0] += t;
    addTicketDate();
    success = nfc->mifareclassic_WriteDataBlock(4, data);
  }
  return success ? data[0] : -1;
}

uint8_t ticketManager::removeTickets(uint8_t t) {
  success = nfc->mifareclassic_ReadDataBlock(4, data);
  if(success && data[0] > 0) {
    data[0] -= t;
    addTicketDate();
    success = nfc->mifareclassic_WriteDataBlock(4, data);
  }
  return success ? data[0] : -1;
}

