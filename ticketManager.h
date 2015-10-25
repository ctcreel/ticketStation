#ifndef ticketManager_h
#define ticketManager_h

#include <Adafruit_PN532.h>

class ticketManager {
  private:
    Adafruit_PN532 *nfc;
    uint8_t success;
    uint8_t data[16];
    uint8_t uid[7];
    uint8_t uidLength;

    void addTicketDate(void);

  public:
    ticketManager(void);
    uint8_t getCardType(void);
    uint8_t setCardType(uint8_t type);
    uint8_t waitForCard(void);
    uint8_t getTickets(void);
    unsigned long getLastSwipeDate(void);
    uint8_t addTickets(uint8_t t = 1);
    uint8_t removeTickets(uint8_t t = 1);
};

#endif
