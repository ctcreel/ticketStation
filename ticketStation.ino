#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <Adafruit_RGBLCDShield.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <LiquidCrystal.h>
#include <ticketManager.h>
#include <lcdManager.h>

ticketManager *tm;
lcdManager *screen;
lcdSector *lineOne;
lcdSector *lineTwo;
lcdSector *lineThree;
lcdSector *lineFour;
lcdSector *lineFive;
lcdSector *lineSix;
boolean isAuthorized;
boolean spendTickets;
boolean idle;
unsigned long lastActive;
unsigned int tickets;
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

void setup(void) {
  Serial.begin(19200);
  tm = new ticketManager;
  setTime(1445181196);
  lastActive = now();
  idle=true;
  tickets = 0;

  Serial.println("Creating manager");
  screen = new lcdManager(16,2,new lcdSectorRGBShieldFactory(16,2,&lcd));
  screen->addPage(0);
  lineOne = screen->addSector(0,0,16,0);
  lineTwo = screen->addSector(0,1,16,0);
  screen->addPage(1);
  lineThree = screen->addSector(0,0,16,1);
  lineFour = screen->addSector(0,1,16,1);
  screen->addPage(2);
  lineFive = screen->addSector(0,0,16,2);
  lineSix = screen->addSector(0,1,16,2);
  reset();
  Serial.println("Let's get started");
}

void loop(void) {
  if(tm->waitForCard()) {
    activity();
    if(screen->activePage() == 0) {
      screenOneActions();
    } else if(screen->activePage() == 1) {
      screenTwoActions();
    } else if(screen->activePage() == 2) {
      screenThreeActions();
    }
  }

  checkButtons();
  checkIdle();
}

void screenOneActions(void) {
  if(tm->getCardType() == 1) {
    authorize();
    lineOne->update("Authorized...");
    lineTwo->update("Add a ticket");
  } else if(authorized()) {
    addTickets();
  } else {
    displayTickets();
  }
}

void screenTwoActions(void) {
  if(tm->getCardType() != 1) {
    if(tickets > 0 && spendTickets) {
      unsigned int t = tapSlowly(lineFour);
      if(t != 255) {
        if(tm->subtractTickets(tickets)) {
          tickets = 0;
          spendTickets = false;
          lineThree->update("Purchased!");
          lineFour->update("Congratulations!");
          tapSlowly(lineFour);
        } else {
          spendTickets = false;
          tickets = 0;
          lineThree->update("Keep saving");
          lineFour->update("your tickets");
        }
      }
    }
  }
}

void screenThreeActions(void) {
  if(tm->getCardType() == 1 && !authorized()) {
    authorize();
    lineFive->update("Authorized.");
    lineSix->update("Swipe Blank");
  } else if(tm->getCardType() == 0 && authorized()) {
    tm->setCardType(1);
    lineFive->update("Created new");
    lineSix->update("master");
    Alarm.delay(3000);
    lineFive->update("Authorized...");
    lineSix->update("Swipe Blank");
  } else if(tm->getCardType() != 0 && authorized()) {
    lineFive->update("Card is not");
    lineSix->update("a blank.");
    Alarm.delay(3000);
    lineFive->update("Authorized...");
    lineSix->update("Swipe Blank");
  } else if(tm->getCardType() == 0 && !authorized()) {
    lineFive->update("First swipe");
    lineSix->update("a master card");
    Alarm.delay(3000);
    lineFive->update("New access card");
    lineSix->update("Please authorize");
  }
}

unsigned int tapSlowly(lcdSector *l) {
  unsigned long t = tm->getTickets();
  unsigned int currentPage = screen->activePage();
  if(t == 255) {
    l->update("Tap slowly...");
  } else {
    l->update(t);
    Alarm.delay(3);
  }
  return t;
}

void activity(void) {
  lastActive = now();
  screen->turnOn();
  idle = false;
}

void checkIdle(void) {
  unsigned long elapsed = now() - lastActive;
  
  if(elapsed > 10 && elapsed < 20) {
    screen->activatePage(0);
    reset();
    idle = true;
  }

  if(now() - lastActive >= 20) {
    screen->turnOff();
  }
}

void reset(void) {
  resetAuthorization();
  spendTickets = false;
  tickets = 0;
  lineOne->update("Waiting...");
  lineTwo->update("Tap slowly...");
  lineThree->update("Select # tickets");
  lineFour->update("0");
  lineFive->update("New access card");
  lineSix->update("Please authorize");
}

void resetAuthorization(void) {
    isAuthorized = false;
}

void authorize(void) {
    isAuthorized = true;
}

boolean authorized(void) {
    return isAuthorized;
}

void addTickets(void) {
  if(screen->activePage() == 0) {
    if(authorized()) {
      tm->addTickets();
      lineOne->update("Added ticket!");
      tapSlowly(lineTwo);
    }
  }
}

void displayTickets(void) {
  if(screen->activePage() == 0) {
    lineOne->update("Your tickets...");
    tapSlowly(lineTwo);
  }
}

void checkButtons(void) {
  uint8_t buttons = lcd.readButtons();
  if(buttons) {
    activity();
    if(buttons & BUTTON_RIGHT) {
      screen->activatePage(screen->activePage() + 1 % 3);
      reset();
    } else if(buttons & BUTTON_LEFT) {
      screen->activatePage(screen->activePage() == 0 ? 2 : screen->activePage() % 3);
      reset();
    } else if(buttons & BUTTON_UP) {
      if(screen->activePage() == 1) {
        tickets++;
        spendTickets = false;
        lineThree->update("Select # tickets");
        lineFour->update(String(tickets).c_str());
      }
    } else if(buttons & BUTTON_DOWN) {
      if(screen->activePage() == 1) {
        if(tickets > 0) {
          tickets--;
          spendTickets = false;
          lineThree->update("Select # tickets");
          lineFour->update(String(tickets).c_str());
        }
      }
    } else if(buttons & BUTTON_SELECT) {
      if(screen->activePage() == 1 && tickets > 0) {
        lineThree->update("Tap card to buy");
        spendTickets = true;
      }
    }
  }
}
