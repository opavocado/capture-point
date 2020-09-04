/*
  Button.h - Library for reading digital buttons with some input lag
  Created by Nicolas Verdicchio, December 15, 2019.
*/
#ifndef Button_h
#define Button_h

#include "Arduino.h"


class Button
{
  public:
    Button(int pin);
    void read(unsigned long currentTime);
    unsigned long int getCurrentStateDuration();
    int getCurrentState();
    int getPreviousState();
    bool hasStateChanged();
    void reset(unsigned long int currentTime);
    bool hasBeenPressed(int state, unsigned long duration);

  private:
    int pin;
    int highCount;
    int lowCount;
    int currentState;
    int previousState;
    int continuosCycles;
    unsigned long lastCycleTime;
    unsigned long const buttonReadTime = 100;

};

#endif