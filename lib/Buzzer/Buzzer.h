/*
  Buzzer.h - Library for controlling a Buzzer (hopefully) painlessly
  Created by Nicolas Verdicchio, December 12, 2019.
*/
#ifndef Buzzer_h
#define Buzzer_h

#include "Arduino.h"
#include "Pitches.h"
#include "Tone.h"
#include "LinkedList.h"

class Buzzer
{
  public:
    Buzzer(int pin);
    void update(unsigned long currentTime);
    void beep();
    void twoTonesUp();
    void playCapturing();
    void playCaptured();
    void playGameOver();
    void stopPlaying();

  private:
    int pin;
    LinkedList <Tone> toneQueue;
    unsigned long lastPlayedToneTime;
    unsigned long currentToneDuration;
};

#endif