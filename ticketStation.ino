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
boolean authorized;
unsigned long lastSwiped;
unsigned long lastPressed;

void setup(void) {
  Serial.begin(115200);
  tm = new ticketManager;
  setTime(1445181196);
  screen = new lcdManager(16,2,new lcdSectorRGBShieldFactory(16,2));
  screen->addPage(0);
  lineOne = screen->addSector(0,0,16,0);
  lineTwo = screen->addSector(0,1,16,0);
  screen->addPage(1);
  lineThree = screen->addSector(0,0,16,0);
  lineFour = screen->addSector(0,1,16,0);
  reset();
}

void loop(void) {
  if(tm->waitForCard()) {
    if(tm->getCardType() == 1) {
      authorize();
    } else if(authorized) {
      addTickets();
    } else {
      displayTickets();
    }
    cardSwiped();
  }
  reset();
}

void cardSwiped(void) {
  lastSwiped = now();
  delay(1000);
}

void reset(void) {
  if(now() - lastSwiped > 10) {
    authorized = false;
    lineOne->update("Waiting...");
    lineTwo->update("Swipe card now..");
  }
}

void authorize(void) {
  authorized = true;
  lineOne->update("Authorized...");
  lineTwo->update("Add a ticket");
}

void addTickets(void) {
  if(authorized) {
    tm->addTickets();
    authorized = false;
    lineOne->update("Added ticket!");
    lineTwo->update(tm->getTickets());
  }
}

void displayTickets(void) {
  lineOne->update("Your tickets...");
  lineTwo->update(tm->getTickets());
}
