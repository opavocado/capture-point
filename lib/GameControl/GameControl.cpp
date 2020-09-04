/*
  GameControl.h - Library for controlling the Capture Point game
  Created by Nicolas Verdicchio, December 8, 2019.
*/
#include "Arduino.h"
#include "GameControl.h"

GameControl::GameControl()
{
  blueCurrentTimer = countdown;
  redCurrentTimer = countdown;
  currentCaptureTimeStamp = 0;
  lastCountdownAdjust = 0; 
  currentCaptureTeam = None;
}

unsigned long GameControl::getCurrentTimer(Team team)
{
    if(team == Blue) {
        return blueCurrentTimer;
    } else {
        return redCurrentTimer;
    }
}

unsigned long GameControl::getCurrentCaptureTimeStamp() 
{
  return currentCaptureTimeStamp;
}

void GameControl::restartCountdowns()
{
    blueCurrentTimer = countdown;
    redCurrentTimer = countdown;
    lastCountdownAdjust = 0;
}

void GameControl::setCurrentCaptureTimeStamp(unsigned long timeStamp)
{
  currentCaptureTimeStamp = timeStamp;
}

void GameControl::clearCurrentCaptureTimeStamp(unsigned long currentTime)
{
  currentCaptureTimeStamp = 0;
  lastCountdownAdjust = currentTime;
}

unsigned long GameControl::getRequiredCaptureTime()
{
  return requiredCaptureTime;
}

bool GameControl::hasCaptureCompleted(unsigned long currentTime)
{
  unsigned long captureTime = currentTime - currentCaptureTimeStamp;

  // Check if capture start time has been recorded
  if(currentCaptureTimeStamp == 0) {
    currentCaptureTimeStamp = currentTime;
    return false;
  }

  return captureTime >= requiredCaptureTime;
}

void GameControl::updateTimer(Team team, unsigned long currentTime)
{
    unsigned long deltaTime = currentTime - lastCountdownAdjust;

    // TODO REFACTOR - probably create Timer and instance one for each team
    unsigned long teamCurrentTimer;
    
    // Get team's timer    
    if(team == Team::Blue) {
      teamCurrentTimer = blueCurrentTimer;
    } else {
      teamCurrentTimer = redCurrentTimer;
    }

    if(deltaTime <= teamCurrentTimer) {
      teamCurrentTimer = teamCurrentTimer - deltaTime;
    } else {
      // Most of the time, being the steps in irregular times, deltaTime might be greater than the remaining amount in redCurrenTimer.
      // In order to avoid teamCurrentTimer going over to its max value, we simply assign it 0.
      teamCurrentTimer = 0;
    }

    // Update team's timer (also should be refactored when Timer is created as an object)
    if(team == Team::Blue) {
      blueCurrentTimer = teamCurrentTimer;
    } else {
      redCurrentTimer = teamCurrentTimer;
    }

    // Update execution time of this cycle
    lastCountdownAdjust = currentTime;
}

void GameControl::captured(Team team, unsigned long currentTime)
{
  currentCaptureTeam = team;
  lastCountdownAdjust = currentTime;
}