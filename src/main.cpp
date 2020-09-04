#include "Arduino.h"
#include "Wire.h"
#include <LCD.h>
#include "GameControl.h"
#include "Buzzer.h"
#include "Button.h"

// Serial debugging
#define SPrintln(a) (Serial.println(a))
#define SPrint(a) (Serial.print(a))
//#define SPrintln(a)
//#define SPrint(a) 
// For final release replace line above with: #define SPrint(a)
// The precompiler will remove all serial print lines, thus improving performance

#define TRANSITION_DELAY 1000

// Buttons
#define BUTTON_DELAY 500
#define BLUE_BUTTON_PIN 4
#define RED_BUTTON_PIN 2

// Buzzer
#define BUZZER_PIN 8

// Main Objects
GameControl* gameControl;
LCD* lcd;
Buzzer* buzzer;
Button* blueButton;
Button* redButton;

unsigned long lastLog = 0;
unsigned long currentTime = 0;

// Onboard Led
const int ledPin =  13;

// Game State
enum SystemState {Ready, StandBy, RedCountdown, BlueCountdown, RedWon, BlueWon};
SystemState currentState;
 

/// Temp declarations
void startReadyCycle();
void readButtons();
void readyCycle();
void standByCycle();
void redCountdownCycle();
void blueCountdownCycle();
void redWonCycle();
void blueWonCycle();
void startStandByCycle();
void capturing(GameControl::Team capturingTeam);
void checkIfCaptureWasCanceled();
void startRedWonCycle();
void checkIfCaptureWasCanceled();
void startBlueWonCycle();
/////////////////////



void setup()
{
  currentTime = millis();

  Serial.begin(9600);

  // GameControl
  gameControl = new GameControl();
  
  // LCD
  lcd = new LCD(0x27,20,4);
  lcd->setGameControl(gameControl);
  lcd->init(currentTime);

  // I/O Setup
  blueButton = new Button(BLUE_BUTTON_PIN);
  redButton = new Button(RED_BUTTON_PIN);

  pinMode(ledPin, OUTPUT);

  // Buzzer
  buzzer = new Buzzer(BUZZER_PIN);
  buzzer->beep();
  buzzer->update(currentTime);

  delay(TRANSITION_DELAY * 2);
  // Ready status
  startReadyCycle();
}


void loop()
{
  //debugGeneral();
  currentTime = millis();

  // Capture Sensors
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

  // Execute Actuators
  buzzer->update(currentTime);
  lcd->update(currentTime);
}


// Controls
void readButtons() 
{
  redButton->read(currentTime);
  blueButton->read(currentTime);
}

void clearReadInputs()
{
  blueButton->reset(currentTime);
  blueButton->reset(currentTime);
}

// Cycles
void readyCycle() 
{
  if(blueButton->hasBeenPressed(HIGH, BUTTON_DELAY) || redButton->hasBeenPressed(HIGH, BUTTON_DELAY)) { 
    // switch to next state
    buzzer->twoTonesUp();
    startStandByCycle();
    delay(TRANSITION_DELAY);
  } else {
    lcd->blinkReadyMessage(currentTime);
  }
}

void standByCycle() 
{
  
  // Conflict - Both Capturing
  if(blueButton->getCurrentState() == HIGH && redButton->getCurrentState() == HIGH) {
    
    // Display conflict message
    lcd->displayConflictMessage(currentTime);

    // Clear current capture time stamp
    gameControl->clearCurrentCaptureTimeStamp(currentTime);
    clearReadInputs();
    delay(TRANSITION_DELAY);

    lcd->displayStandByMessage(currentTime);
    
  } else {
    // Red Capturing
    if(redButton->getCurrentState() == HIGH) {
      capturing(GameControl::Red);
    }
    
    // Blue Capturing
    if(blueButton->getCurrentState() == HIGH) {
      capturing(GameControl::Blue);
    }
  }

  // Capturing process has been stopped and any of the buttons have been released
  if(currentState != BlueCountdown && currentState != RedCountdown) {
    checkIfCaptureWasCanceled();
    
    if(blueButton->hasStateChanged() || redButton->hasStateChanged()) {
      lcd->displayStandByMessage(currentTime);
    }
  }
  
}

void redCountdownCycle()  // TODO REFACTOR MERGE WITH blueCoutdownCycle
{
  unsigned long redCurrentTimer = gameControl->getCurrentTimer(GameControl::Red);
  
  bool hasRedWon = redCurrentTimer <= 0;
  bool isBlueCapturing = blueButton->getCurrentState() == HIGH;
  
  // Check whether the capture button was released
   if(currentState != BlueCountdown) {
    checkIfCaptureWasCanceled();
  }

  // Red won
  if(hasRedWon) {
    startRedWonCycle();
  } 

  // Blue is recapturing
  if(!hasRedWon && isBlueCapturing) {
    capturing(GameControl::Blue);
  } 

  // Reduce Red Countdown
  if(!hasRedWon && !isBlueCapturing) {
    // Adjust timer
    gameControl->updateTimer(GameControl::Red, currentTime);

    lcd->displayUpdateRedTimer(currentTime);
  }

}

void blueCountdownCycle() 
{
  unsigned long blueCurrentTimer = gameControl->getCurrentTimer(GameControl::Blue);
 
  bool hasBlueWon = blueCurrentTimer <= 0;
  bool isRedCapturing = redButton->getCurrentState() == HIGH;
  
  // Check whether the capture button was released
   if(currentState != RedCountdown) {
    checkIfCaptureWasCanceled();
  }
  
  // Blue won
  if(hasBlueWon) {
    startBlueWonCycle();
  } 

  // Red is recapturing
  if(!hasBlueWon && isRedCapturing) {
    capturing(GameControl::Red);
  } 

  // Reduce Blue Countdown
  if(!hasBlueWon && !isRedCapturing) {
    // Adjust timer
    gameControl->updateTimer(GameControl::Blue, currentTime);

    lcd->displayUpdateBlueTimer(currentTime);
  }

}

void redWonCycle() 
{
  if(blueButton->getCurrentState() == HIGH || redButton->getCurrentState() == HIGH) {
    startReadyCycle();
  }
}

void blueWonCycle() 
{
  if(blueButton->getCurrentState() == HIGH || redButton->getCurrentState() == HIGH) {
    startReadyCycle();
  }
}


// Start Cycles
void startReadyCycle() 
{
  SPrintln("Start Ready Cycle");
  // Shut up Buzzer (if restarting match)
  buzzer->stopPlaying();
  
  // Set Current State
  currentState = Ready;

  // Prepare Variables
  gameControl->restartCountdowns();
  
  // Draw Ready message
  lcd->displayReadyMessage(currentTime);
}

void startStandByCycle()
{
  SPrintln("Start Stand By Cycle");
  clearReadInputs();
  gameControl->clearCurrentCaptureTimeStamp(currentTime);
  // Set Current State
  currentState = StandBy;

  // Draw Stand By message
  lcd->displayStandByMessage(currentTime);
}

void startRedCountdownCycle() 
{
  SPrintln("Start Red Countdown Cycle");
  gameControl->clearCurrentCaptureTimeStamp(currentTime);
  clearReadInputs();
  currentState = RedCountdown;
  
  // Display capture message
  lcd->displayRedCapturedMessage(currentTime);
  delay(TRANSITION_DELAY);
  // Start Countdown Cycle
  gameControl->captured(GameControl::Red, currentTime);
}

void startRedWonCycle() 
{  
  // Set state to RedWon
  currentState = RedWon;
  
  // Draw Red Won message
  buzzer->playGameOver();
  lcd->displayRedWonMessage(currentTime);
}

void startBlueCountdownCycle() 
{
  SPrintln("Start Blue Countdown Cycle");
  gameControl->clearCurrentCaptureTimeStamp(currentTime);
  clearReadInputs();
  currentState = BlueCountdown;
  
  // Display capture message
  lcd->displayBlueCapturedMessage(currentTime);
  delay(TRANSITION_DELAY);
  // Start Countdown Cycle
  gameControl->captured(GameControl::Blue, currentTime);
}

void startBlueWonCycle() 
{
  // Set state to BlueWon
  currentState = BlueWon;
  
  // Draw Blue Won message
  buzzer->playGameOver();
  lcd->displayBlueWonMessage(currentTime);
}

// Capturing
void capturing(GameControl::Team capturingTeam) 
{
  //SPrintln("\nCAPTURING");
  buzzer->playCapturing();
  
  // If requiredCaptureTime is reached, switch to Countdown
  if(gameControl->hasCaptureCompleted(currentTime))
  {
    buzzer->playCaptured();
    switch(capturingTeam) {
      case GameControl::Blue:  startBlueCountdownCycle(); break;
      case GameControl::Red:   startRedCountdownCycle(); break;
    }
  } else {
    // If the capture has not been completed, update the progress bar
    lcd->updateProgressBar(currentTime);
  }

}
  
void checkIfCaptureWasCanceled()
{
  if(blueButton->hasStateChanged() || redButton->hasStateChanged()) {
    gameControl->clearCurrentCaptureTimeStamp(currentTime);
    buzzer->stopPlaying();
    lcd->clearProgressBar(currentTime);
  }
}

// Debug
void debugGeneral() {
  // Every half second print only
  if(currentTime - lastLog > 1000) {
    lastLog = currentTime;
    SPrintln();
    SPrintln((String)"Current Time: " + currentTime);
  }
}