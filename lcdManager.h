#ifndef lcdManager_h
#define lcdManager_h
#include <Arduino.h>
#include <Adafruit_RGBLCDShield.h>

class lcdSector;
class lcdPage;
class lcdManager;
class lcdFactory;

class lcdSector {
  
  friend lcdPage;
  
  private:
    char *buffer;
    char format[10];
    char widthString[5];
    unsigned int x, y, width;
    lcdPage *page;
    unsigned int justified;
    void generateFormat(char *);
    
  protected:
    const char *getBuffer(void);
    void activePrint(void);
    virtual void print(void) = 0;
    
  public:
    lcdSector(
      const unsigned int x,
      const unsigned int y,
      const unsigned int width,
      lcdPage *page,
      const unsigned int justified = 0 // 0 = left, !0 = right
      );
    const unsigned int getX(void);
    const unsigned int getY(void);
    const unsigned int getWidth(void);
    const unsigned int getPageHeight(void);
    const unsigned int getPageWidth(void);
    void update(const char *);
    void update(const char);
    void update(const int);
    void update(const unsigned int);
    void update(const long);
    void update(const unsigned long);
    void update(const float);
    void update(const double);
};

class lcdPage {
  
  friend lcdManager;
  friend lcdSector;
  
  private:
    struct sectorList {
      lcdSector *sector;
      sectorList *next;
    } *sectors;
    
    unsigned int width;
    unsigned int height;
    boolean activePage;
    
    void activatePage(void);
    void deactivatePage(void);
    void printPage(void);
    void addSector(lcdSector *);
    
  public:

  lcdPage(const unsigned int width, const unsigned int height);
  const unsigned int getWidth(void);
  const unsigned int getHeight(void);
  boolean pageActive(void);
  const unsigned int numSectors(void);
};

class lcdManager {
  
  private:
  
    struct pageList {
      lcdPage *page;
      int pageID;
      pageList *next;
    } *pages;

    unsigned int width, height;
    lcdFactory *factory;
    
  public:
    lcdManager(
      const unsigned int width,
      const unsigned int height,
      lcdFactory *factory);
    void addPage(const unsigned int pageID);
    void activatePage(const unsigned int pageID);
    int activePage(void);
    lcdSector *addSector(
      const unsigned int x,
      const unsigned int y,
      const unsigned int width,
      const unsigned int pageID);
    unsigned int numSectors(const unsigned int pageID);
    unsigned int numPages(void);
    void turnOff(void);
    void turnOn(void);
};

class lcdFactory {
  friend lcdManager;

  virtual void turnOff(void) = 0;
  virtual void turnOn(void) = 0;
  virtual lcdSector *createSector(
    const unsigned int x, 
    const unsigned int y,
    const unsigned int width,
    lcdPage *) = 0;
};

class lcdSectorRGBShieldFactory:public lcdFactory {
  private:
    static boolean initialized;
    Adafruit_RGBLCDShield *lcd;
    
  public:
  
  lcdSectorRGBShieldFactory(
    const unsigned int width,
    const unsigned int height,
    Adafruit_RGBLCDShield *lcd);
  void turnOff(void);
  void turnOn(void);
  lcdSector *createSector(
    const unsigned int x, 
    const unsigned int y,
    const unsigned int width,
    lcdPage *);
};

class lcdSectorRGBShield:public lcdSector {
  private:
    Adafruit_RGBLCDShield *lcd;
    void print(void);
    
  public:
    lcdSectorRGBShield(
      const unsigned int x, 
      const unsigned int y,
      const unsigned int width, 
      lcdPage *,
      Adafruit_RGBLCDShield *);
};

#endif
