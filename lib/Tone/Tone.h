/*
  Tone.h - Basically, just tone and a duration. Used in Buzzer.h
  Created by Nicolas Verdicchio, December 12, 2019.
*/
#ifndef Tone_h
#define Tone_h

#define BASE_TONE_DURATION 1000

#include "Arduino.h"


class Tone
{
  public:
    Tone();
    Tone(int note, float duration);
    int getNote();
    float getDuration();

  private:
    int note;
    float duration;
};

#endif