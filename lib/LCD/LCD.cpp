/*
  lcd->h - Library for controlling the LCD for CP
  Created by Nicolas Verdicchio, December 8, 2019.
*/

#include "Arduino.h"
#include "LCD.h"
#include <LiquidCrystal_I2C.h>
#include <GameControl.h>

LCD::LCD(int address, int width, int height)
{
    lcd = new LiquidCrystal_I2C(address, width, height);
    this->addCustomChars();

    lastProgressBlocks = 0;
    lastBlinkTime = 0;
    isLineDisplayed = true;
    turnedOnTime = 0;
    isBacklightOn = false;
}

void LCD::addCustomChars()
{
    uint8_t full_block[8] = {
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111
    }; 

    lcd->createChar(0, full_block);
}

void LCD::init(unsigned long currentTime) 
{
    // Start display
    lcd->init();
    this->clear(currentTime);

    // Startup animation
    lcd->setCursor(2,1);
    lcd->print("PUNTO DE CAPTURA");
    lcd->setCursor(6,3);
    lcd->print("MANKUZOS");
}

void LCD::setGameControl(GameControl* gc) 
{
    gameControl = gc;
}

void LCD::blinkReadyMessage(unsigned long currentTime)
{
  this->turnOnBacklight(currentTime);
  if(currentTime - lastBlinkTime > 700) {
      if(isLineDisplayed) {
          // Hide the line
          lcd->setCursor(0,2);
          lcd->print("      Comenzar!    ");
      } else {
          // show the line
          lcd->setCursor(0,2);
          lcd->print(" X    Comenzar!   X");
      }

      isLineDisplayed = !isLineDisplayed;
      lastBlinkTime = currentTime;
  }
    
}

void LCD::displayStandByMessage(unsigned long currentTime) 
{
  this->clear(currentTime);
  /*
   * 01234567890123456789  
   *     NO CAPTURADO
   *      Azul: 10:00
   *      Rojo: 10:00   
   * |||||||||||||||||||   <-- fills while capturing
   */
  
  lcd->setCursor(4,0);
  lcd->print("NO CAPTURADO");
  
  lcd->setCursor(5,1);
  lcd->print("Azul: 10:00");

  lcd->setCursor(5,2);
  lcd->print("Rojo: 10:00");  
}

void LCD::displayConflictMessage(unsigned long currentTime)
{
    this->clear(currentTime);
    lcd->setCursor(2,0);
    lcd->print("** CONFLICTO **");
    
    lcd->setCursor(0,1);
    lcd->print("SOLO PUEDE CAPTURAR");

    lcd->setCursor(0,2);
    lcd->print("UN EQUIPO A LA VEZ");
}

void LCD::displayRedCapturedMessage(unsigned long currentTime) 
{
  lastProgressBlocks = 0;
  char currentCountdown[20];
  this->clear(currentTime);
  /*
   * 01234567890123456789  
   *   PUNTO CAPTURADO
   *      Azul: 10:00
   *   >> Rojo: 10:00 <<
   * |||||||||||||||||||   <-- fills while capturing
   */
  
  lcd->setCursor(2,0);
  lcd->print("PUNTO CAPTURADO");
  
  lcd->setCursor(4,1);
  lcd->print("Azul: ");
  this->getCurrentCountdown(GameControl::Blue, currentCountdown);
  lcd->print(currentCountdown);

  lcd->setCursor(1,2);
  lcd->print(">> Rojo: ");
  this->getCurrentCountdown(GameControl::Red, currentCountdown);
  lcd->print(currentCountdown);
}

void LCD::getCurrentCountdown(GameControl::Team currentTeam, char* currentCountdown) 
{
  unsigned long currentTimer;
  unsigned int currentSeconds;
  unsigned int currentMinutes;
  unsigned int displaySeconds;
  
  currentTimer = gameControl->getCurrentTimer(currentTeam);

  // Calculate minutes and seconds
  currentSeconds = (currentTimer)/1000; // actual total value in seconds
  currentMinutes = currentSeconds/60;
  displaySeconds = currentSeconds%60;   // these are the remaining seconds after substracting the minutes from the total value

  sprintf(currentCountdown,"%02u:%02u",currentMinutes, displaySeconds);
}

void LCD::displayUpdateRedTimer(unsigned long currentTime) 
{
  char currentCountdown[20];
  bool hasAMinuteStarted = (gameControl->getCurrentTimer(GameControl::Red)/1000)%60 == 00;
  if(hasAMinuteStarted) {
    this->turnOnBacklight(currentTime);
  }
  lcd->setCursor(1,2);
  lcd->print(">> Rojo: ");
  this->getCurrentCountdown(GameControl::Red, currentCountdown);
  lcd->print(currentCountdown);
}

void LCD::displayBlueCapturedMessage(unsigned long currentTime) 
{
  lastProgressBlocks = 0;
  char currentCountdown[20];
  this->clear(currentTime);
  /*
   * 01234567890123456789  
   *   PUNTO CAPTURADO
   *   >> Azul: 10:00
   *      Rojo: 10:00 
   * |||||||||||||||||||   <-- fills while capturing
   */
  
  lcd->setCursor(2,0);
  lcd->print("PUNTO CAPTURADO");
  
  lcd->setCursor(1,1);
  lcd->print(">> Azul: ");
  this->getCurrentCountdown(GameControl::Blue, currentCountdown);
  lcd->print(currentCountdown);

  lcd->setCursor(4,2);
  lcd->print("Rojo: ");
  this->getCurrentCountdown(GameControl::Red, currentCountdown);
  lcd->print(currentCountdown);
}

void LCD::displayUpdateBlueTimer(unsigned long currentTime) 
{
  char currentCountdown[20];
  bool hasAMinuteStarted = (gameControl->getCurrentTimer(GameControl::Blue)/1000)%60 == 00;
  if(hasAMinuteStarted) {
    this->turnOnBacklight(currentTime);
  }
  lcd->setCursor(1,1);
  lcd->print(">> Azul: ");
  this->getCurrentCountdown(GameControl::Blue, currentCountdown);
  lcd->print(currentCountdown);
}

void LCD::clearProgressBar(unsigned long currentTime) 
{
  this->turnOnBacklight(currentTime);
  lastProgressBlocks = 0;
  lcd->setCursor(0,3);
  lcd->print("                    ");
}

void LCD::displayReadyMessage(unsigned long currentTime)
{
    this->clear(currentTime);
    /*
    * 01234567890123456789
    *   PUNTO DE CAPTURA  
    *    Tiempo: 10 min
    *  X    Comenzar!   X   <-- blinking
    * |||||||||||||||||||   <-- fills while holding both buttons
    */
    
    lcd->setCursor(2,0);
    lcd->print("PUNTO DE CAPTURA");
    
    lcd->setCursor(3,1);
    lcd->print("Tiempo: 10 min");

    lcd->setCursor(0,2);
    lcd->print(" X    Comenzar!   X");
}

void LCD::displayRedWonMessage(unsigned long currentTime) 
{
    char currentCountdown[20];
    // Get other team's remaining countdown
    this->getCurrentCountdown(GameControl::Blue, currentCountdown);
    this->clear(currentTime);
    /*
    * 01234567890123456789
    *    EQUIPO GANADOR
    *    >>>> ROJO <<<< 
    * 
    * Tiempo Azul: XX:YY
    */
    lcd->setCursor(3,0);
    lcd->print("EQUIPO GANADOR");

    lcd->setCursor(3,1);
    lcd->print(">>>> ROJO <<<<");

    lcd->setCursor(0,3);
    lcd->print("Tiempo Azul: ");
    lcd->print(currentCountdown);
}

void LCD::displayBlueWonMessage(unsigned long currentTime)
{
    char currentCountdown[20];
    // Get other team's remaining countdown
    this->getCurrentCountdown(GameControl::Red, currentCountdown);
    this->clear(currentTime);
    /*
    * 01234567890123456789
    *    EQUIPO GANADOR
    *    >>>> Azul <<<< 
    * 
    * Tiempo Rojo: XX:YY
    */

    lcd->setCursor(3,0);
    lcd->print("EQUIPO GANADOR");

    lcd->setCursor(3,1);
    lcd->print(">>>> Azul <<<<");

    lcd->setCursor(0,3);
    lcd->print("Tiempo Rojo: ");
    lcd->print(currentCountdown);
}

void LCD::updateProgressBar(unsigned long currentTime)
{
  this->turnOnBacklight(currentTime);
  // Calculate progress
    unsigned long captureTime = (currentTime - gameControl->getCurrentCaptureTimeStamp());
    float progress = (float) captureTime / gameControl->getRequiredCaptureTime();

    // Calculate current amount of blocks to display
    int progressBlocks = round(progress * 20);

    // Print only when there's a new progress block to add (instead of printing every loop)
    if((progressBlocks > lastProgressBlocks) && (progressBlocks < 20)) {
        for(int i = lastProgressBlocks; i <= progressBlocks; i++) {
          lcd->setCursor(i,3);
          lcd->write(0);
        }
        lastProgressBlocks = progressBlocks;
    }
    if(progressBlocks >= 20) {
      // 100% reached, reset
      lastProgressBlocks = 0;
    }
}

void LCD::clear(unsigned long currentTime)
{
  lcd->setCursor(0,0);
  lcd->print("                                                                                ");
  this->turnOnBacklight(currentTime);
}

void LCD::update(unsigned long currentTime)
{
  if(isBacklightOn && (currentTime - turnedOnTime) > BACKLIGHT_ON_TIME) {
    lcd->noBacklight();
    isBacklightOn = false;
  }
}

void LCD::turnOnBacklight(unsigned long currentTime)
{
  lcd->backlight();
  isBacklightOn = true;
  turnedOnTime = currentTime;
}