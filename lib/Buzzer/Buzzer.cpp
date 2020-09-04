#include "Arduino.h"
#include "Buzzer.h"
#include "Pitches.h"

Buzzer::Buzzer(int pin)
{
    pinMode(pin, OUTPUT);
    toneQueue = LinkedList<Tone>();
    lastPlayedToneTime = 0;
    currentToneDuration = 0;
    this->pin = pin;
}

void Buzzer::update(unsigned long currentTime)
{
    // If last tone duration has conluded AND there's still something on the queue
    if(currentTime >= (lastPlayedToneTime + currentToneDuration) && toneQueue.size() > 0) {
        // Stop all sounds
        noTone(pin);
        
        // Obtain next tone
        Tone currentTone = toneQueue.shift();

        // Play next tone if it's note isn't silent
        int currentNote = currentTone.getNote();
        if(SILENT != currentNote) {
            tone(pin, currentNote, currentTone.getDuration());
        }
        
        // Update time stamp and note duration
        lastPlayedToneTime = currentTime;
        currentToneDuration = currentTone.getDuration();
        
        // Release memory of tone - is this needed? Tone wasn't generated with new
        //delete(&currentTone);
    }
}

// Default beep once for a second
void Buzzer::beep()
{
    toneQueue.add(Tone(NOTE_C7,8));
}

void Buzzer::twoTonesUp()
{
    toneQueue.add(Tone(NOTE_C6,8));
    toneQueue.add(Tone(SILENT,8));
    toneQueue.add(Tone(NOTE_G6,8));
}

void Buzzer::stopPlaying()
{
    noTone(pin);
    toneQueue.clear();
}

void Buzzer::playCapturing()
{
    // Add the capture alarm once   
    if(toneQueue.size() == 0) {
        toneQueue.add(Tone(NOTE_G6,2));
        toneQueue.add(Tone(NOTE_A6,2));        
    }
}

void Buzzer::playCaptured()
{
    this->stopPlaying();
    toneQueue.add(Tone(NOTE_A6,8));
    toneQueue.add(Tone(SILENT,8));
    toneQueue.add(Tone(NOTE_A6,8));
    toneQueue.add(Tone(SILENT,8));
    toneQueue.add(Tone(NOTE_A6,8));
}

void Buzzer::playGameOver()
{
    this->stopPlaying();
    for(int i = 0; i < 5; i++) {
        toneQueue.add(Tone(NOTE_B5,1));
        toneQueue.add(Tone(SILENT,1));
    }
    
}