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
      uint8_t notes[2] = {0, 0};
      stepEvent *next;
    } stepEvent;
    uint8_t bank;
    void addNewStepToTail(uint8_t _sequenceNumber);
  public:
    uint8_t sequence_length;
    EventSequence(uint8_t len) : sequence_length(len) {bank = 0;};
    void step();
    void toggleBank();
    stepEvent *head, *tail, *current;
    void setSync(uint16_t val);
    uint16_t getSync();
};

#endif
