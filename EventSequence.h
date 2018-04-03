#ifndef EventSequence_h
#define EventSequence_h

#include "Arduino.h"
#include <avr/io.h>
#include <avr/interrupt.h>


#define POLY 1

// a two bank sequencer with variable loop length,
// can store <POLY> number of notes at each event
// TODO:  REVERSE MODE | EDOM ESREVER

class EventSequence {
  private:
    // sequence step
    typedef struct stepEvent{
      uint8_t sequenceNumber;
      uint8_t notes[2][1] = {{0}, {0}};
      stepEvent *next;
      stepEvent *prev; // for reverse
    } stepEvent;
    bool forwards;
    uint8_t bank;
    void addNewStepToTail(uint8_t _sequenceNumber);
    void playAllNotesAtCurrentStep();
    void stopAllNotesAtCurrentStep();
  public:
    uint8_t sequence_length;
    EventSequence(uint8_t len) : sequence_length(len) {bank = 0; forwards = true;};
    void step();
    void toggleBank();
    stepEvent *head, *tail, *current;
    void addNote2CurrentStep(uint8_t note);
    void setPhase1(uint16_t val);
    void setPhase2(uint16_t val);
    void setDecay1(uint16_t val);
    void setDecay2(uint16_t val);
    void setSync(uint16_t val);
    uint16_t getPhase1();
    uint16_t getPhase2();
    uint16_t getDecay1();
    uint16_t getDecay2();
    uint16_t getSync();
};

#endif
