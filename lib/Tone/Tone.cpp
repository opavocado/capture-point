#include "Arduino.h"
#include "Tone.h"

Tone::Tone() 
{
    note = 0;
    duration = 0;
}

Tone::Tone(int note, float duration) 
{
    this->note = note;
    this->duration = BASE_TONE_DURATION / duration;
}

int Tone::getNote()
{
    return note;
}

float Tone::getDuration()
{
    return duration;
}