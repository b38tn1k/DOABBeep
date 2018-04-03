#include "Arduino.h"
#include "Timer.h"

Timer::Timer(double _bpm) {
  setBPM(_bpm);
}

bool Timer::tick() {
  unsigned long now = millis();
  if ((now - previousMillis) > interval) {
    previousMillis =  now;
    return true;
  } else {
  return false;
  }
}

void Timer::setBPM(double _bpm) {
  bpm = _bpm;
  interval = (60000.00 * subDivision) / bpm;
}

