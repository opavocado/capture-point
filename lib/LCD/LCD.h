/*
  LCD.h - Library for controlling the LCD for CP
  Created by Nicolas Verdicchio, December 8, 2019.
*/
#ifndef LCD_h
#define LCD_h
#define BACKLIGHT_ON_TIME 10000 // 10 sec

#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
#include <GameControl.h>

class LCD
{
  public:
    LCD(int address, int width, int height);
    void init(unsigned long currentTime);
    void setGameControl(GameControl* gc);
    void update(unsigned long currentTime);

    // LCD Display Messages
    void blinkReadyMessage(unsigned long currentTime);
    void displayStandByMessage(unsigned long currentTime);
    void displayConflictMessage(unsigned long currentTime);
    void displayRedCapturedMessage(unsigned long currentTime);
    void getCurrentCountdown(GameControl::Team currentTeam, char* currentCountdown);
    void displayUpdateRedTimer(unsigned long currentTime);
    void displayBlueCapturedMessage(unsigned long currentTime);
    void displayUpdateBlueTimer(unsigned long currentTime);
    void clearProgressBar(unsigned long currentTime);
    void displayReadyMessage(unsigned long currentTime);
    void displayRedWonMessage(unsigned long currentTime);
    void displayBlueWonMessage(unsigned long currentTime);
    void updateProgressBar(unsigned long currentTime);
    void turnOnBacklight(unsigned long currentTime);

  private:
    void addCustomChars();
    void clear(unsigned long currentTime);
    LiquidCrystal_I2C* lcd;
    
    int lastProgressBlocks;
    unsigned long lastBlinkTime;
    bool isLineDisplayed;
    GameControl* gameControl;
    
    // Backlight control
    unsigned long turnedOnTime;
    bool isBacklightOn;
};

#endif