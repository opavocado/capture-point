/*
  GameControl.h - Library for controlling the Capture Point game
  Created by Nicolas Verdicchio, December 8, 2019.
*/
#ifndef GameControl_h
#define GameControl_h

#include "Arduino.h"

#define COUNTDOWN_LENGTH 600000; // 10 minutes == 600k miliseconds

class GameControl
{
  public:
    GameControl();
    enum Team {Blue, Red, None};
    void restartCountdowns();
    void clearCurrentCaptureTimeStamp(unsigned long currentTime);
    bool hasCaptureCompleted(unsigned long currentTime);

    
    // Getters / Setters
    unsigned long getCurrentTimer(Team team);
    unsigned long getCurrentCaptureTimeStamp();
    void setCurrentCaptureTimeStamp(unsigned long timeStamp);
    unsigned long getRequiredCaptureTime();
    void updateTimer(Team team, unsigned long currentTime);
    void captured(Team team, unsigned long currentTime);
    

  private:
    // Timers
    const unsigned long countdown = COUNTDOWN_LENGTH;
    unsigned long blueCurrentTimer;
    unsigned long redCurrentTimer;
    const unsigned long requiredCaptureTime = 3000; // 3 seconds
    unsigned long currentCaptureTimeStamp;
    unsigned long lastCountdownAdjust; 
    Team currentCaptureTeam;
    


};

#endif