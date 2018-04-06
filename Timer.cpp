#include "Arduino.h"
#include "Timer.h"

Timer::Timer(double _bpm, uint8_t _clkTrig1, uint8_t _clkTrig2, uint8_t _clkOut) {
  setBPM(_bpm);
  if (_clkTrig1 != NULL) {
    clkTrig1 = _clkTrig1;
    pinMode(clkTrig1, INPUT);
  }
  if (_clkTrig2 != NULL) {
    clkTrig2 = _clkTrig2;
    pinMode(clkTrig2, INPUT);
  }
  if (_clkOut != NULL) {
    clkOut = _clkOut;
    pinMode(clkOut, OUTPUT);
  }
}

bool Timer::tick() {
  bool tick = false;
  if (useInternalCLK == false) {
    // read from trigger 1
    trig1State = digitalRead(clkTrig1);
    if (oldTrig1State != trig1State) {
      oldTrig1State = trig1State;
      clkState = ! clkState;
      tick = true;
    }
  } else {
    unsigned long now = millis();
    if ((now - previousMillis) > interval) {
      previousMillis =  now;
      clkState = ! clkState;
      tick = true;
    }
  }
  // send trigger out
  if (clkState == true) {
    digitalWrite(clkOut, HIGH);
  } else {
    digitalWrite(clkOut, LOW);
  }
  return tick;
}

void Timer::setBPM(double _bpm) {
  bpm = _bpm;
  interval = (60000.00 * subDivision) / bpm;
}

