#include "EventSequence.h"
#include "Arduino.h"
#include <avr/io.h>
#include <avr/interrupt.h>


void EventSequence::addNewStepToTail(uint8_t _sequenceNumber) {
  stepEvent *temp = new stepEvent;
  temp->sequenceNumber = _sequenceNumber;
  temp->next = NULL;
  if (head == NULL) {
    temp->prev = NULL;
    head = temp;
    tail = temp;
    current = head;
  } else {
    tail->next = temp;
    temp->prev = tail;
    tail = temp;
  }
  temp = NULL;
}

void EventSequence::step() {
  if (current == NULL) {
    addNewStepToTail(1);
  } else {
    uint8_t i = current->sequenceNumber;
    if (i < sequence_length) {
      if (current->next == NULL) {
        addNewStepToTail(++i);
        current = tail;
      } else {
        current = current->next;
      }
    } else {
      current = head;  
    }
  }
  // update knobs and stuff
}

void EventSequence::toggleBank() {
  if (bank == 0) { bank = 1;}
  else {bank = 0;}
}

void EventSequence::addNote2CurrentStep(uint8_t _note) {
  int i;
  for (i = 0; i < POLY; i++) {
    if (current->notes[bank][i] == _note) {
      break; // don't double record notes
    }
    if (current->notes[bank][i] == NULL) {
      current->notes[bank][i] = _note;
      break;
    }
  }
}

void EventSequence::setSync(uint16_t val){
  current->notes[bank][0] = val;
}

uint16_t EventSequence::getSync(){
  return current->notes[bank][0];
}
