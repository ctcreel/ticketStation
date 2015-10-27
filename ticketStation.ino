#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <Adafruit_RGBLCDShield.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <LiquidCrystal.h>
#include "ticketManager.h"
#include "lcdManager.h"

ticketManager *tm;
lcdManager *screen;
lcdSector *lineOne;
lcdSector *lineTwo;
lcdSector *lineThree;
lcdSector *lineFour;
lcdSector *lineFive;
lcdSector *lineSix;
boolean authorized;
boolean spendTickets;
boolean idle;
unsigned long lastSwiped;
unsigned long lastPressed;
unsigned int tickets;
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

void setup(void) {
  Serial.begin(19200);
  tm = new ticketManager;
  setTime(1445181196);
  lastSwiped = now();
  lastPressed = now();
  idle=true;
  tickets = 0;

  Serial.println("Creating manager");
  screen = new lcdManager(16,2,new lcdSectorRGBShieldFactory(16,2,&lcd));
  Serial.println("Adding first page");
  screen->addPage(0);
  lineOne = screen->addSector(0,0,16,0);
  lineTwo = screen->addSector(0,1,16,0);
  lineOne->update("Waiting...");
  lineTwo->update("Tap card now");
  Serial.println("Adding second page");
  screen->addPage(1);
  lineThree = screen->addSector(0,0,16,1);
  lineFour = screen->addSector(0,1,16,1);
  lineThree->update("Select # tickets");
  lineFour->update("0");
  
  Serial.println(screen->numSectors(0));
  Serial.println(screen->numSectors(1));
  Serial.println(screen->numPages());
  reset();
  Serial.println("Let's get started");
}

void loop(void) {
  if(tm->waitForCard()) {
    cardSwiped();
    if(screen->activePage() == 0) {
      screenOneActions();
    } else if(screen->activePage() == 1) {
      screenTwoActions();
    }
  }

  checkButtons();
  checkIdle();
}

void screenOneActions(void) {
  if(tm->getCardType() == 1) {
    authorize();
  } else if(authorized) {
    addTickets();
  } else {
    displayTickets();
  }
}

unsigned int swipeSlowly(lcdSector *l) {
  unsigned long t = tm->getTickets();
  unsigned int currentPage = screen->activePage();
  if(t == 255) {
    l->update("Tap slowly...");
  } else {
    l->update(t);
  }
  return t;
}

void screenTwoActions(void) {
  if(tm->getCardType() != 1) {
    if(tickets > 0 && spendTickets) {
      unsigned int t = swipeSlowly(lineFour);
      if(t != 255) {
        if(tm->subtractTickets(tickets)) {
          tickets = 0;
          spendTickets = false;
          lineThree->update("Purchased!");
          swipeSlowly(lineFour);
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

void cardSwiped(void) {
  lastSwiped = now();
  screen->turnOn();
  idle = false;
}

void checkIdle(void) {
  if(now() - lastSwiped > 5 && now() - lastPressed > 5 && !idle) {
    screen->activatePage(0);
    reset();
    idle = true;
  }

  if(now() - lastSwiped > 15 && now() - lastPressed > 15 && idle) {
    screen->turnOff();
  }
}

void reset(void) {
  authorized = false;
  spendTickets = false;
  tickets = 0;
  lineOne->update("Waiting...");
  lineTwo->update("Tap card now");
  lineThree->update("Select # tickets");
  lineFour->update("0");
}

void authorize(void) {
    authorized = true;
    lineOne->update("Authorized...");
    lineTwo->update("Add a ticket");
}

void addTickets(void) {
  if(screen->activePage() == 0) {
    if(authorized) {
      tm->addTickets();
      authorized = false;
      lineOne->update("Added ticket!");
      swipeSlowly(lineTwo);
    }
  }
}

void displayTickets(void) {
  if(screen->activePage() == 0) {
    lineOne->update("Your tickets...");
    swipeSlowly(lineTwo);
  }
}

void checkButtons(void) {
  uint8_t buttons = lcd.readButtons();
  if(buttons) {
    lastPressed = now();
    idle = false;
    if(buttons & BUTTON_RIGHT) {
      screen->activatePage(!screen->activePage());
      reset();
    } else if(buttons & BUTTON_LEFT) {
      screen->activatePage(!screen->activePage());
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
