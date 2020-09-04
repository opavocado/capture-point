#include "Arduino.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Serial debugging
#define SPrintln(a) (Serial.println(a))
#define SPrint(a) (Serial.print(a))
//#define SPrintln(a) (Serial.println(a))
//#define SPrint(a) (Serial.print(a))
// For final release replace line above with: #define SPrint(a)
// The precompiler will remove all serial print lines, thus improving performance

unsigned long lastLog = 0;

// LCD
#define TRANSITION_DELAY 1000
LiquidCrystal_I2C lcd(0x27,20,4);

const byte full_block[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
}; 

int lastProgressBlocks = 0;

unsigned long lastBlinkTime = 0;
bool isLineDisplayed = true;

// Blue Button
#define BUTTON_DELAY 10000
const int blueButton = 4;
int blueButtonCount = 0;    // Used to calculate how long it has been pressed
int blueButtonState = 0;  // Used to capture button state
int blueButtonLastState = 0;
bool blueButtonStateChanged = 0;

// Red Button
const int redButton = 2;
int redButtonCount = 0;     
int redButtonState = 0;  
int redButtonLastState = 0; 
bool redButtonStateChanged = 0;

// Buzzer
const int buzzer = 9;

// Onboard Led
const int ledPin =  13;

// Game State
enum SystemState {Ready, StandBy, RedCountdown, BlueCountdown, RedWon, BlueWon};
SystemState currentState;

// Teams
enum Team {Blue, Red};

// Timers
const unsigned long countdown = 25000; // 10 minutes == 600k miliseconds
unsigned long blueCurrentTimer = countdown;
unsigned long redCurrentTimer = countdown;

// Countdown progress
unsigned long lastCountdownAdjust = 0;  


// Capturing
const unsigned long requiredCaptureTime = 4000; // 5 seconds
unsigned long currentCaptureTimeStamp = 0;


void setup()
{
  Serial.begin(9600);

  lcdInit();  
  delay(TRANSITION_DELAY);

  // I/O Setup
  pinMode(blueButton, INPUT);
  pinMode(redButton, INPUT);

  pinMode(buzzer, OUTPUT);

  pinMode(ledPin, OUTPUT);

  // Ready status
  startReadyCycle();
}


void loop()
{
  //debugGeneral();
  
  // Capture Input
  readButtons();

  // Execute Cycle
  switch(currentState)
  {
    case Ready: 
                readyCycle();
                break;
    case StandBy:
                standByCycle();
                break;
    case RedCountdown:
                redCountdownCycle();
                break;
    case BlueCountdown:
                blueCountdownCycle();
                break;
    case RedWon:
                redWonCycle();
                break;
    case BlueWon:
                blueWonCycle();
                break;
  }

}

// LCD
void lcdInit() 
{
  // Start display
  lcd.init();
  lcd.clear();
  lcd.backlight();

  // Add custom chars
  lcd.createChar(0, full_block);

  // Startup animation
  lcd.setCursor(2,1);
  lcd.print("PUNTO DE CAPTURA");
  lcd.setCursor(6,3);
  lcd.print("MANKUZOS");
  delay(TRANSITION_DELAY);
}

void lcdBlinkReadyMessage()
{
  unsigned long currentTime = millis();
  
  if(currentTime - lastBlinkTime > 700) {
    if(isLineDisplayed) {
      // Hide the line
      lcd.setCursor(0,2);
      lcd.print("      Comenzar!    ");
    } else {
      // show the line
      lcd.setCursor(0,2);
      lcd.print(" X    Comenzar!   X");
    }
    
    isLineDisplayed = !isLineDisplayed;
    lastBlinkTime = currentTime;
  }
}

void lcdDisplayStandByMessage() 
{
  lcd.clear();
  /*
   * 01234567890123456789  
   *     NO CAPTURADO
   *      Azul: 10:00
   *      Rojo: 10:00   
   * |||||||||||||||||||   <-- fills while capturing
   */
  
  lcd.setCursor(4,0);
  lcd.print("NO CAPTURADO");
  
  lcd.setCursor(5,1);
  lcd.print("Azul: 10:00");

  lcd.setCursor(5,2);
  lcd.print("Rojo: 10:00");  
}

void lcdDisplayConflictMessage()
{
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("** CONFLICTO **");
    
    lcd.setCursor(0,1);
    lcd.print("SOLO PUEDE CAPTURAR");

    lcd.setCursor(0,2);
    lcd.print("UN EQUIPO A LA VEZ");
}

void lcdDisplayRedCapturedMessage() 
{
  char currentCountdown[20];
  lcd.clear();
  /*
   * 01234567890123456789  
   *   PUNTO CAPTURADO
   *      Azul: 10:00
   *   >> Rojo: 10:00 <<
   * |||||||||||||||||||   <-- fills while capturing
   */
  
  lcd.setCursor(1,0);
  lcd.print("PUNTO CAPTURADO");
  
  lcd.setCursor(4,1);
  lcd.print("Azul: ");
  getCurrentCountdown(Blue, currentCountdown);
  lcd.print(currentCountdown);

  lcd.setCursor(1,2);
  lcd.print(">> Rojo: ");
  getCurrentCountdown(Red, currentCountdown);
  lcd.print(currentCountdown);
}

// Generates LCD printable string of current timer based on team
void getCurrentCountdown(Team currentTeam, char* currentCountdown) 
{
  unsigned long currentTimer;
  unsigned int currentSeconds;
  unsigned int currentMinutes;
  unsigned int displaySeconds;
  
  if(currentTeam == Blue) {
    currentTimer = blueCurrentTimer;
  } else {
    currentTimer = redCurrentTimer;
  }

  // Calculate minutes and seconds
  currentSeconds = (currentTimer)/1000; // actual total value in seconds
  currentMinutes = currentSeconds/60;
  displaySeconds = currentSeconds%60;   // these are the remaining seconds after substracting the minutes from the total value

  sprintf(currentCountdown,"%02u:%02u",currentMinutes, displaySeconds);
}

void lcdDisplayUpdateRedTimer() 
{
  char currentCountdown[20];
  lcd.setCursor(1,2);
  lcd.print(">> Rojo: ");
  getCurrentCountdown(Red, currentCountdown);
  lcd.print(currentCountdown);
}

void lcdDisplayBlueCapturedMessage() 
{
  char currentCountdown[20];
  lcd.clear();
  /*
   * 01234567890123456789  
   *   PUNTO CAPTURADO
   *   >> Azul: 10:00
   *      Rojo: 10:00 
   * |||||||||||||||||||   <-- fills while capturing
   */
  
  lcd.setCursor(1,0);
  lcd.print("PUNTO CAPTURADO");
  
  lcd.setCursor(1,1);
  lcd.print(">> Azul: ");
  getCurrentCountdown(Blue, currentCountdown);
  lcd.print(currentCountdown);

  lcd.setCursor(4,2);
  lcd.print("Rojo: ");
  getCurrentCountdown(Red, currentCountdown);
  lcd.print(currentCountdown);
}

void lcdDisplayUpdateBlueTimer() 
{
  char currentCountdown[20];
  lcd.setCursor(1,1);
  lcd.print(">> Azul: ");
  getCurrentCountdown(Blue, currentCountdown);
  lcd.print(currentCountdown);
}

void lcdClearProgressBar() 
{
  lcd.setCursor(0,3);
  lcd.print("                    ");
}

// Controls
void readButtons() 
{
  // Read current state and check if it changed
  blueButtonState = digitalRead(blueButton);
  blueButtonStateChanged = blueButtonLastState != blueButtonState;
  
  redButtonState = digitalRead(redButton);
  redButtonStateChanged = redButtonLastState != redButtonState;

  // Adjust counters
  if(blueButtonState == HIGH) {
    blueButtonCount++;
  } else {
    blueButtonCount = 0;
    
  }

  if(redButtonState == HIGH) {
    redButtonCount++;
  } else {
    redButtonCount = 0;
  }

  // Save last state
  blueButtonLastState = blueButtonState;
  redButtonLastState = redButtonState;
}

void clearReadInputs()
{
  blueButtonCount = 0;
  redButtonCount = 0;
}

// Cycles
void readyCycle() 
{
  if(blueButtonCount >= BUTTON_DELAY) { // TODO use milis instead of counter
    // switch to next state
    startStandByCycle();
    delay(TRANSITION_DELAY);
  }
  lcdBlinkReadyMessage();
}

void standByCycle() 
{
  
  // Conflict - Both Capturing
  if(blueButtonCount > 200 && redButtonCount > 200) {
    
    // Display conflict message
    lcdDisplayConflictMessage();

    // Clear current capture time stamp
    currentCaptureTimeStamp = 0;
    clearReadInputs();
    delay(TRANSITION_DELAY);

    lcdDisplayStandByMessage();
    
  } else {
    // Red Capturing
    if(redButtonCount > 200) {
      capturing(Red);
    }
    
    // Blue Capturing
    if(blueButtonCount > 200) {
      capturing(Blue);
    }
  }

  // Capturing process has been stopped and any of the buttons have been released
  checkIfCaptureWasCanceled();

  if(blueButtonStateChanged || redButtonStateChanged) { // TODO REFACTOR - this logic is mixed with checkIfCaptureWasCanceled
    lcdDisplayStandByMessage();
  }
  
}

void redCountdownCycle() 
{
  unsigned long currentTime = millis();
 
  bool hasRedWon = redCurrentTimer <= 0;
  bool isBlueCapturing = blueButtonCount > 10;
  
  
  // Red won
  if(hasRedWon) {
    startRedWonCycle();
  } 

  // Blue is recapturing
  if(!hasRedWon && isBlueCapturing) {
    capturing(Blue);
  } 

  // Reduce Red Countdown
  if(!hasRedWon && !isBlueCapturing) {
    // Adjust timer
    unsigned long deltaTime = currentTime - lastCountdownAdjust;
    if(deltaTime <= redCurrentTimer) {
      redCurrentTimer = redCurrentTimer - deltaTime;
    } else {
      // Most of the time, being the steps in irregular times, deltaTime might be greater than the remaining amount in redCurrenTimer.
      // In order to avoid redCurrentTimer going over to its max value, we simply assign it 0.
      redCurrentTimer = 0;
    }

    lcdDisplayUpdateRedTimer();
  }

  // Update execution time of this cycle
  lastCountdownAdjust = currentTime;

  // Check whether the capture button was released
  checkIfCaptureWasCanceled();
  
}

void blueCountdownCycle() 
{
  unsigned long currentTime = millis();
 
  bool hasBlueWon = blueCurrentTimer <= 0;
  bool isRedCapturing = redButtonCount > 10;
  
  
  // Blue won
  if(hasBlueWon) {
    startBlueWonCycle();
  } 

  // Red is recapturing
  if(!hasBlueWon && isRedCapturing) {
    capturing(Red);
  } 

  // Reduce Blue Countdown
  if(!hasBlueWon && !isRedCapturing) {
    // Adjust timer
    unsigned long deltaTime = currentTime - lastCountdownAdjust;
    if(deltaTime <= blueCurrentTimer) {
      blueCurrentTimer = blueCurrentTimer - deltaTime;
    } else {
      // Most of the time, being the steps in irregular times, deltaTime might be greater than the remaining amount in redCurrenTimer.
      // In order to avoid redCurrentTimer going over to its max value, we simply assign it 0.
      blueCurrentTimer = 0;
    }

    lcdDisplayUpdateBlueTimer();
  }

  // Update execution time of this cycle
  lastCountdownAdjust = currentTime;

  // Check whether the capture button was released
  checkIfCaptureWasCanceled();
}

void redWonCycle() 
{
  if(blueButtonCount >= BUTTON_DELAY) {
    startReadyCycle();
  }
}

void blueWonCycle() 
{
  if(blueButtonCount >= BUTTON_DELAY) {
    startReadyCycle();
  }
}


// Start Cycles
void startReadyCycle() 
{
  SPrintln("Start Ready Cycle");
  // Set Current State
  currentState = Ready;

  // Prepare Variables
  blueCurrentTimer = countdown;
  redCurrentTimer = countdown;
  lastCountdownAdjust = 0;
  
  // Draw Ready message
  lcd.clear();
  /*
   * 01234567890123456789
   *   PUNTO DE CAPTURA  
   *    Tiempo: 10 min
   *  X    Comenzar!   X   <-- blinking
   * |||||||||||||||||||   <-- fills while holding both buttons
   */
  
  lcd.setCursor(2,0);
  lcd.print("PUNTO DE CAPTURA");
  
  lcd.setCursor(3,1);
  lcd.print("Tiempo: 10 min");

  lcd.setCursor(0,2);
  lcd.print(" X    Comenzar!   X");
}

void startStandByCycle()
{
  SPrintln("Start Stand By Cycle");
  clearReadInputs();
  // Set Current State
  currentState = StandBy;

  // Draw Stand By message
  lcdDisplayStandByMessage();
}

void startRedCountdownCycle() 
{
  SPrintln("Start Red Countdown Cycle");
  clearReadInputs();
  currentState = RedCountdown;
  
  // Display capture message
  lcdDisplayRedCapturedMessage();
  delay(TRANSITION_DELAY);
  // Start Countdown Cycle
  lastCountdownAdjust = millis();
}

void startRedWonCycle() 
{
  char currentCountdown[20];
  
  // Set state to RedWon
  currentState = RedWon;
  
  // Draw Red Won message
  lcd.clear();
  /*
   * 01234567890123456789
   *    EQUIPO GANADOR
   *    >>>> ROJO <<<< 
   * 
   * Tiempo Azul: XX:YY
   */
  
  lcd.setCursor(3,0);
  lcd.print("EQUIPO GANADOR");
  
  lcd.setCursor(3,1);
  lcd.print(">>>> ROJO <<<<");

  lcd.setCursor(0,3);
  lcd.print("Tiempo Azul: ");
  getCurrentCountdown(Blue, currentCountdown);
  lcd.print(currentCountdown);
}

void startBlueCountdownCycle() 
{
  SPrintln("Start Blue Countdown Cycle");
  clearReadInputs();
  currentState = BlueCountdown;
  
  // Display capture message
  lcdDisplayBlueCapturedMessage();
  delay(TRANSITION_DELAY);
  // Start Countdown Cycle
  lastCountdownAdjust = millis();
}

void startBlueWonCycle() 
{
  char currentCountdown[20];
  
  // Set state to BlueWon
  currentState = BlueWon;
  
  // Draw Blue Won message
  lcd.clear();
  /*
   * 01234567890123456789
   *    EQUIPO GANADOR
   *    >>>> Azul <<<< 
   * 
   * Tiempo Rojo: XX:YY
   */
  
  lcd.setCursor(3,0);
  lcd.print("EQUIPO GANADOR");
  
  lcd.setCursor(3,1);
  lcd.print(">>>> Azul <<<<");

  lcd.setCursor(0,3);
  lcd.print("Tiempo Rojo: ");
  getCurrentCountdown(Red, currentCountdown);
  lcd.print(currentCountdown);
}

// Capturing
void capturing(Team capturingTeam) 
{
  SPrintln("\nCAPTURING");
  
  // Obtain current time
  unsigned long currentTime = millis();
  
  // Check if capture start time has been recorded
  if(currentCaptureTimeStamp == 0) {
    currentCaptureTimeStamp = currentTime;
//  SPrintln("currentCaptureTimeStamp SET!");
  }
  
  // Calculate progress
  unsigned long captureTime = (currentTime - currentCaptureTimeStamp);
  float progress = (float) captureTime / requiredCaptureTime;

//  SPrint("currentTime: ");
//  SPrint(currentTime);
//  SPrint(" currentCaptureTimeStamp: ");
//  SPrint(currentCaptureTimeStamp);  
//  SPrint(" captureTime: ");
//  SPrintln(captureTime);
    

   // If requiredCaptureTime is reached, switch to Countdown
  if(captureTime >= requiredCaptureTime) {
    lastProgressBlocks = 0;
    currentCaptureTimeStamp = 0;
    switch(capturingTeam) {
      case Blue:  startBlueCountdownCycle(); break;
      case Red:   startRedCountdownCycle(); break;
    }
    
  } else {
    // Increase progress in LCD
    // Every 0.05 (5%) is one
    int progressBlocks = round(progress / 0.05);

    
    //  SPrint("requiredCaptureTime: ");
    //  SPrint(requiredCaptureTime);
    //  SPrint(" progress: ");
    //  SPrint(progress);
    //  SPrint(" progressBlocks: ");
    //  SPrint(progressBlocks);
    //  SPrint(" lastProgressBlocks: ");
    //  SPrintln(lastProgressBlocks);
    //  SPrintln();

    // Print only when there's a new progress block to add (instead of printing every loop)
    if(progressBlocks > lastProgressBlocks && progressBlocks < 20) {
        for(int i = lastProgressBlocks; i <= progressBlocks; i++) {
          lcd.setCursor(i,3);
          lcd.write(0);
        }
        lastProgressBlocks = progressBlocks;
    }
  }


}
  

void redCapturing() 
{
  SPrintln("\nRED CAPTURING");
  
  // Obtain current time
  unsigned long currentTime = millis();
  
  // Check if capture start time has been recorded
  if(currentCaptureTimeStamp == 0) {
    currentCaptureTimeStamp = currentTime;
//  SPrintln("currentCaptureTimeStamp SET!");
  }
  
  // Calculate progress
  unsigned long captureTime = (currentTime - currentCaptureTimeStamp);
  float progress = (float) captureTime / requiredCaptureTime;

//  SPrint("currentTime: ");
//  SPrint(currentTime);
//  SPrint(" currentCaptureTimeStamp: ");
//  SPrint(currentCaptureTimeStamp);  
//  SPrint(" captureTime: ");
//  SPrintln(captureTime);
    

   // If requiredCaptureTime is reached, switch to RedCountdown
  if(captureTime >= requiredCaptureTime) {
    lastProgressBlocks = 0;
    currentCaptureTimeStamp = 0;
    startRedCountdownCycle();
  } else {
    // Increase progress in LCD
    // Every 0.05 (5%) is one
    int progressBlocks = round(progress / 0.05);

    
    //  SPrint("requiredCaptureTime: ");
    //  SPrint(requiredCaptureTime);
    //  SPrint(" progress: ");
    //  SPrint(progress);
    //  SPrint(" progressBlocks: ");
    //  SPrint(progressBlocks);
    //  SPrint(" lastProgressBlocks: ");
    //  SPrintln(lastProgressBlocks);
    //  SPrintln();

    // Print only when there's a new progress block to add (instead of printing every loop)
    if(progressBlocks > lastProgressBlocks && progressBlocks < 20) {
        for(int i = lastProgressBlocks; i <= progressBlocks; i++) {
          lcd.setCursor(i,3);
          lcd.write(0);
        }
        lastProgressBlocks = progressBlocks;
    }
  }


}

// TODO REFACTOR - this method is exactly the same as redCapturing except that it calls startBlueCountdownCycle()
void blueCapturing() 
{
  SPrintln("\nBLUE CAPTURING");
  
  // Obtain current time
  unsigned long currentTime = millis();
  
  // Check if capture start time has been recorded
  if(currentCaptureTimeStamp == 0) {
    currentCaptureTimeStamp = currentTime;
//    SPrintln("currentCaptureTimeStamp SET!");
  }
  
  // Calculate progress
  unsigned long captureTime = (currentTime - currentCaptureTimeStamp);
  float progress = (float) captureTime / requiredCaptureTime;

//  SPrint("currentTime: ");
//  SPrint(currentTime);
//  SPrint(" currentCaptureTimeStamp: ");
//  SPrint(currentCaptureTimeStamp);  
//  SPrint(" captureTime: ");
//  SPrintln(captureTime);
    
  // Increase progress in LCD
  // Every 0.05 (5%) is one
  int progressBlocks = round(progress / 0.05);

//  SPrint("requiredCaptureTime: ");
//  SPrint(requiredCaptureTime);
//  SPrint(" progress: ");
//  SPrint(progress);
//  SPrint(" progressBlocks: ");
//  SPrint(progressBlocks);
//  SPrint(" lastProgressBlocks: ");
//  SPrintln(lastProgressBlocks);

  // Print only when there's a new progress block to add (instead of printing every loop)
  if(progressBlocks > lastProgressBlocks) {
      for(int i = lastProgressBlocks; i <= progressBlocks; i++) {
        lcd.setCursor(i,3);
        lcd.write(0);
      }
      lastProgressBlocks = progressBlocks;
  }

  // If requiredCaptureTime is reached, switch to BlueCountdown
  if(captureTime >= requiredCaptureTime) {
    lastProgressBlocks = 0;
    currentCaptureTimeStamp = 0;
    startBlueCountdownCycle();
  }
}

void checkIfCaptureWasCanceled()
{
  if(blueButtonStateChanged || redButtonStateChanged) {
    currentCaptureTimeStamp = 0;
    lastProgressBlocks = 0;
    lcdClearProgressBar();
  }
}

// Debug
void debugGeneral() {
  // Every half second print only
  unsigned long currentTime = millis();
  if(currentTime - lastLog > 1000) {
    lastLog = currentTime;
    SPrintln();
    SPrintln((String)"Current Time: " + currentTime);
    SPrint("Blue: ");
    SPrint((int)blueButtonCount);
    SPrint(" Red: ");
    SPrint((int)redButtonCount);
  }
}
