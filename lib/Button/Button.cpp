
#include "Arduino.h"
#include "Button.h"

Button::Button(int pin)
{
    this->pin = pin;
    pinMode(pin, INPUT);
    
    lastCycleTime = 0;
    highCount = 0;
    lowCount = 0;
    currentState = LOW;
    previousState = LOW;
}

// Cycle every buttonReadTime (ms) - Calculate average state for that period
// The way this works, adds 'buttonReadTime' input lag
void Button::read(unsigned long currentTime)
{
    bool hasCycleEnded = (currentTime - lastCycleTime) > buttonReadTime;

    if(hasCycleEnded)
    {
        // Update button last state (the previous average result)
        previousState = currentState;
        
        // Update button current value (the current average result)
        if(highCount > lowCount) {
            currentState = HIGH;
        } else {
            currentState = LOW;
        }
        
        // Reset counters 
        highCount = 0;
        lowCount = 0;

        // Count how many cycles has the state remained the same
        if(currentState == previousState) {
            continuosCycles++;
        } else {
            continuosCycles = 0;
        }

        lastCycleTime = currentTime;

    }
    
    //  Calculate current cycle's average
    int signalRead = digitalRead(pin);

    if(signalRead == HIGH) {
        highCount++;
    } else {
        lowCount++;
    }
    
}

void Button::reset(unsigned long int currentTime)
{
    highCount = 0;
    lowCount = 0;
    currentState = LOW;
    lastCycleTime = currentTime;
}

unsigned long int Button::getCurrentStateDuration()
{
    return continuosCycles * buttonReadTime;
}
    
int Button::getCurrentState()
{
    return currentState;
}

int Button::getPreviousState()
{
    return previousState;
}

bool Button::hasStateChanged()
{
    return currentState != previousState;
}

bool Button::hasBeenPressed(int state, unsigned long duration)
{
    bool buttonIsHigh = this->getCurrentState() == state;
    bool buttonHasBeenPressed = this->getCurrentStateDuration() >= duration;
    
    return buttonIsHigh && buttonHasBeenPressed;
}