#include "lcdManager.h"
#include <LiquidCrystal.h>
#include <Adafruit_RGBLCDShield.h>

boolean lcdSectorRGBShieldFactory::initialized;

const unsigned int lcdSector::getX(void) {return x;}
const unsigned int lcdSector::getY(void) {return y;}
const unsigned int lcdSector::getWidth(void) {return width;}
const unsigned int lcdSector::getPageHeight(void) {return page->getWidth();}
const unsigned int lcdSector::getPageWidth(void) {return page->getHeight();}
const char *lcdSector::getBuffer(void) {return buffer;}

lcdSector::lcdSector(
    const unsigned int x1,
    const unsigned int y1,
    const unsigned int w,
    lcdPage *p,
    const unsigned int j
  ) {
  page = p;
  p->addSector(this);
  x = x1;
  y = y1;
  width = w;
  justified = j;
  buffer = new char[w+1];
  itoa(w,widthString,10);
  for(unsigned int i = 0; i < w+1; i++) buffer[i] = 0;
}

void lcdSector::activePrint(void) {
  if(page->pageActive()) {
    Serial.print("Printing \"");
    Serial.print(buffer);
    Serial.print("\" using format string - ");
    Serial.println(format);
    print();
  }
}

void lcdSector::generateFormat(char *f) {
  strcpy(format, "%");
  if(!justified) {
    strcat(format,"-");
  }
  strcat(format,widthString);
  strcat(format,f);
}

void lcdSector::update(const char *v) {
  generateFormat("s");
  snprintf(buffer,width+1,format,v);
  activePrint();
}

void lcdSector::update(const char v) {
  generateFormat("c");
  snprintf(buffer,width+1,format,v);
  activePrint();
}

void lcdSector::update(const int v) {
  generateFormat("d");
  snprintf(buffer,width+1,format,v);
  activePrint();
}

void lcdSector::update(const unsigned int v) {
  generateFormat("u");
  snprintf(buffer,width+1,format,v);
  activePrint();
}

void lcdSector::update(const long v) {
  generateFormat("ld");
  snprintf(buffer,width+1,format,v);
  activePrint();
}

void lcdSector::update(const unsigned long v) {
  generateFormat("lu");
  snprintf(buffer,width+1,format,v);
  activePrint();
}

void lcdSector::update(float v) {
  generateFormat("f");
  snprintf(buffer,width+1,format,v);
  activePrint();
}

void lcdSector::update(double v) {
  generateFormat("L");
  snprintf(buffer,width+1,format,v);
  activePrint();
}

const unsigned int lcdPage::getWidth(void) {return width;}
const unsigned int lcdPage::getHeight(void) {return height;}

void lcdPage::activatePage(void) {
  if(!activePage) {
    printPage();
  }
  activePage = true;
}

void lcdPage::deactivatePage(void) {
  activePage = false;
}

void lcdPage::printPage(void) {
  Serial.println(activePage);
  sectorList *current = sectors;
  while(current) {
    current->sector->print();
    current = current->next;
  }
}

lcdPage::lcdPage(const unsigned int w, const unsigned int h) {
  width = w;
  height = h;
  sectors = 0;
  activePage = false;
}

void lcdPage::addSector(lcdSector *s) {
  sectorList *current = sectors;
  sectors = new sectorList;
  sectors->sector = s;
  sectors->next = current;
}

const unsigned int lcdPage::numSectors(void) {
  sectorList *current = sectors;
  unsigned int n=0;
  while(current) {
    current = current->next;
    n++;
  }
  return n;
}

boolean lcdPage::pageActive(void) {
  return activePage;
}


lcdManager::lcdManager(const unsigned int w, const unsigned int h, lcdFactory *f) {
  width = w;
  height = h;
  pages = 0;
  factory = f;
}

void lcdManager::addPage(const unsigned int id) {
  pageList *current = pages;
  pages = new pageList;
  pages->page = new lcdPage(width, height);
  pages->pageID = id;
  pages->next = current;
  Serial.println(id);
  if(!current) {
    pages->page->activatePage();
  }
}

void lcdManager::activatePage(const unsigned int id) {
  pageList *current = pages;
  pageList *currentlyActive = 0;
  pageList *pageWithID = 0;
  while(current) {
    if(current->pageID == id) {
      pageWithID = current;
    }

    if(current->page->pageActive()) {
      currentlyActive = current;
    }

    current = current->next;
  }
  
  if(pageWithID && !pageWithID->page->pageActive()) {
    currentlyActive->page->deactivatePage();
    pageWithID->page->activatePage();
  }
}

int lcdManager::activePage(void) {
  pageList *current = pages;
  while(current) {
    if(current->page->pageActive()) {
      return current->pageID;
    }
    current = current->next;
  }
  
  return -1;
}

lcdSector *lcdManager::addSector(
      const unsigned int x,
      const unsigned int y,
      const unsigned int w,
      const unsigned int id) {
  pageList *current = pages;
  while(current) {
    if(current->pageID == id) {
      return factory->createSector(x,y,w,current->page);
    } else {
      current = current->next;
    }
  }

  return 0;
}

unsigned int lcdManager::numSectors(unsigned int id) {
  pageList *current = pages;
  while(current) {
    if(current->pageID == id) {
      return current->page->numSectors();
    } else {
      current = current->next;
    }
  }

  return 0;
}

unsigned int lcdManager::numPages(void) {
  pageList *current = pages;
  unsigned int n = 0;
  while(current) {
    current = current->next;
    n++;
  }

  return n;
}

void lcdManager::turnOff(void) {
  factory->turnOff();
}

void lcdManager::turnOn(void) {
  factory->turnOn();
}

lcdSectorRGBShieldFactory::lcdSectorRGBShieldFactory(
  const unsigned int width, 
  const unsigned int height,
  Adafruit_RGBLCDShield *l) {
  if(!initialized) {
    lcd = l;
    initialized = true;
    lcd->begin(width,height);
  }
}

void lcdSectorRGBShieldFactory::turnOff(void) {
  lcd->noDisplay();
  lcd->setBacklight(0x0);
}

void lcdSectorRGBShieldFactory::turnOn(void) {
  lcd->display();
  lcd->setBacklight(0x7);
}

lcdSector *lcdSectorRGBShieldFactory::createSector(
    const unsigned int x, 
    const unsigned int y,
    const unsigned int w,
    lcdPage *p) {
  return new lcdSectorRGBShield(x,y,w,p,lcd);
}

lcdSectorRGBShield::lcdSectorRGBShield(
  const unsigned int x, 
  const unsigned int y,
  const unsigned int w, 
  lcdPage *p,
  Adafruit_RGBLCDShield *l) : lcdSector(x,y,w,p) {
  lcd = l;
}

void lcdSectorRGBShield::print(void) {
  lcd->setCursor(getX(),getY());
  lcd->print(getBuffer());
}
