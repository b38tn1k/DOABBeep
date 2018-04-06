// Auduino, the Lo-Fi granular synthesiser
//
// by Peter Knight, Tinker.it http://tinker.it
//
// Help:      http://code.google.com/p/tinkerit/wiki/Auduino
// More help: http://groups.google.com/group/auduino
//
// Analog in 0: Grain 1 pitch
// Analog in 1: Grain 2 decay
// Analog in 2: Grain 1 decay
// Analog in 3: Grain 2 pitch
// Analog in 4: Grain repetition frequency
//
// Digital 3: Audio out (Digital 11 on ATmega8)
//
// Changelog:
// 19 Nov 2008: Added support for ATmega8 boards
// 21 Mar 2009: Added support for ATmega328 boards
// 7 Apr 2009: Fixed interrupt vector for ATmega328 boards
// 8 Apr 2009: Added support for ATmega1280 boards (Arduino Mega)

// 2 Apr 2017: (James) Basic Step Sequencer

#include <avr/io.h>
#include <avr/interrupt.h>
#include "Arduino.h"
#include "EventSequence.h"
#include "Timer.h"
//EventSequence sequencer(64);
EventSequence sequencer(16);
Timer seqTimer(480);
Timer envTimer(50 * 480);

uint16_t syncPhaseAcc;
uint16_t syncPhaseInc;
uint16_t grainPhaseAcc;
uint16_t grainPhaseInc;
uint16_t grainAmp;
uint8_t grainDecay;
uint16_t grain2PhaseAcc;
uint16_t grain2PhaseInc;
uint16_t grain2Amp;
uint8_t grain2Decay;
uint8_t ampEnv;

// Map Analogue channels
#define SYNC_CONTROL         (4)
#define GRAIN_FREQ_CONTROL   (0)
#define GRAIN_DECAY_CONTROL  (2)
#define GRAIN2_FREQ_CONTROL  (3)
#define GRAIN2_DECAY_CONTROL (1)


// Changing these will also requires rewriting audioOn()

#if defined(__AVR_ATmega8__)
//
// On old ATmega8 boards.
//    Output is on pin 11
//
#define LED_PIN       13
#define LED_PORT      PORTB
#define LED_BIT       5
#define PWM_PIN       11
#define PWM_VALUE     OCR2
#define PWM_INTERRUPT TIMER2_OVF_vect
#elif defined(__AVR_ATmega1280__)
//
// On the Arduino Mega
//    Output is on pin 3
//
#define LED_PIN       13
#define LED_PORT      PORTB
#define LED_BIT       7
#define PWM_PIN       3
#define PWM_VALUE     OCR3C
#define PWM_INTERRUPT TIMER3_OVF_vect
#else
//
// For modern ATmega168 and ATmega328 boards
//    Output is on pin 3
//
#define PWM_PIN       3
#define PWM_VALUE     OCR2B
#define LED_PIN       13
#define LED_PORT      PORTB
#define LED_BIT       5
#define PWM_INTERRUPT TIMER2_OVF_vect
#endif

// Smooth logarithmic mapping
//
const uint16_t antilogTable[] PROGMEM = {
  64830,64132,63441,62757,62081,61413,60751,60097,59449,58809,58176,57549,56929,56316,55709,55109,
  54515,53928,53347,52773,52204,51642,51085,50535,49991,49452,48920,48393,47871,47356,46846,46341,
  45842,45348,44859,44376,43898,43425,42958,42495,42037,41584,41136,40693,40255,39821,39392,38968,
  38548,38133,37722,37316,36914,36516,36123,35734,35349,34968,34591,34219,33850,33486,33125,32768
};
uint16_t mapPhaseInc(uint16_t input) {
//  return (antilogTable[input & 0x3f]) >> (input >> 6);
    uint16_t b = pgm_read_word_near(antilogTable + (input & 0x3f));
  return b >> (input >> 6);
}

// Stepped chromatic mapping
//
const uint16_t midiTable[] PROGMEM = {
  0, 17,18,19,20,22,23,24,26,27,29,31,32,34,36,38,41,43,46,48,51,54,58,61,65,69,73,
  77,82,86,92,97,103,109,115,122,129,137,145,154,163,173,183,194,206,218,231,
  244,259,274,291,308,326,346,366,388,411,435,461,489,518,549,581,616,652,691,
  732,776,822,871,923,978,1036,1097,1163,1232,1305,1383,1465,1552,1644,1742,
  1845,1955,2071,2195,2325,2463,2610,2765,2930,3104,3288,3484,3691,3910,4143,
  4389,4650,4927,5220,5530,5859,6207,6577,6968,7382,7821,8286,8779,9301,9854,
  10440,11060,11718,12415,13153,13935,14764,15642,16572,17557,18601,19708,20879,
  22121,23436,24830,26306
};

void audioOn() {
#if defined(__AVR_ATmega8__)
  // ATmega8 has different registers
  TCCR2 = _BV(WGM20) | _BV(COM21) | _BV(CS20);
  TIMSK = _BV(TOIE2);
#elif defined(__AVR_ATmega1280__)
  TCCR3A = _BV(COM3C1) | _BV(WGM30);
  TCCR3B = _BV(CS30);
  TIMSK3 = _BV(TOIE3);
#else
  // Set up PWM to 31.25kHz, phase accurate
  TCCR2A = _BV(COM2B1) | _BV(WGM20);
  TCCR2B = _BV(CS20);
  TIMSK2 = _BV(TOIE2);
#endif
}

void setup() {
  Serial.begin(9600);
  pinMode(PWM_PIN,OUTPUT);
  audioOn();
  pinMode(LED_PIN,OUTPUT);
  grainPhaseInc  = 0;
  grainDecay     = 0;
  grain2PhaseInc = 0;
  grain2Decay    = 0;
  syncPhaseInc   = 0;
}

void loop() {
  if (seqTimer.tick() == true) {
    sequencer.step();    //TEST!!
    ampEnv = 0;
    if (sequencer.current->sequenceNumber % 1 == 0) {
      sequencer.setSync(50);
    }
    if (sequencer.current->sequenceNumber % 2 == 0) {
      sequencer.setSync(50);
    }
    if (sequencer.current->sequenceNumber % 3 == 0) {
      sequencer.setSync(0);
    }
    if (sequencer.current->sequenceNumber % 4 == 0) {
      sequencer.setSync(50);
    }
    if (sequencer.current->sequenceNumber % 5 == 0) {
      sequencer.setSync(0);
    }
    if (sequencer.current->sequenceNumber % 6 == 0) {
      sequencer.setSync(0);
    }
    if (sequencer.current->sequenceNumber % 7 == 0) {
      sequencer.setSync(50);
    }
    if (sequencer.current->sequenceNumber & 8 == 0) {
      sequencer.setSync(0);
    }
    if (sequencer.current->sequenceNumber % 9 == 0) {
      sequencer.setSync(0);
    }
    if (sequencer.current->sequenceNumber % 10 == 0) {
      sequencer.setSync(50);
    }
    if (sequencer.current->sequenceNumber % 11 == 0) {
      sequencer.setSync(62);
    }
    if (sequencer.current->sequenceNumber % 12 == 0) {
      sequencer.setSync(52);
    }
    if (sequencer.current->sequenceNumber % 13 == 0) {
      sequencer.setSync(64);
    }
    if (sequencer.current->sequenceNumber % 14 == 0) {
      sequencer.setSync(53);
    }
    if (sequencer.current->sequenceNumber % 15 == 0) {
      sequencer.setSync(65);
    }
    if (sequencer.current->sequenceNumber % 16 == 0) {
      sequencer.setSync(74);
    }
    Serial.println(sequencer.current->sequenceNumber);
    Serial.println(sequencer.getSync());
    Serial.println();
    
    // END TEST
  }
  if (sequencer.getSync() == 0) {
      grainPhaseInc  = 0;
      grainDecay     = 10000;
      grain2PhaseInc = 0;
      grain2Decay    = 10000;
      syncPhaseInc = 0;
    } else {
      syncPhaseInc = pgm_read_word_near(midiTable + sequencer.getSync());
      grainPhaseInc  = 512;
      grainDecay     = 250;
      grain2PhaseInc = 250;
      grain2Decay    = 125;
//      grainPhaseInc  = mapPhaseInc(1023 * sin(2.0/sequencer.current->sequenceNumber));
//      grainDecay     = 1023 * cos(2.0/(16 - sequencer.current->sequenceNumber));
//      grain2PhaseInc = mapPhaseInc(1023 * cos(2.0/(16 - sequencer.current->sequenceNumber)));
//      grain2Decay    = 1023 * sin(2.0/sequencer.current->sequenceNumber);
//      grainPhaseInc  = mapPhaseInc(analogRead(GRAIN_FREQ_CONTROL)) / 2;
//      grainDecay     = analogRead(GRAIN_DECAY_CONTROL) / 8;
//      grain2PhaseInc = mapPhaseInc(analogRead(GRAIN2_FREQ_CONTROL)) / 2;
//      grain2Decay    = analogRead(GRAIN2_DECAY_CONTROL) / 4;
    }
    if (envTimer.tick() == true) {
      if (ampEnv < 255) ampEnv += 1;
    }
    
}

SIGNAL(PWM_INTERRUPT)
{
  uint8_t value;
  uint16_t output;

  syncPhaseAcc += syncPhaseInc;
  if (syncPhaseAcc < syncPhaseInc) {
    // Time to start the next grain
    grainPhaseAcc = 0;
    grainAmp = 0x7fff;
    grain2PhaseAcc = 0;
    grain2Amp = 0x7fff;
    LED_PORT ^= 1 << LED_BIT; // Faster than using digitalWrite
  }
 
  // Increment the phase of the grain oscillators
  grainPhaseAcc += grainPhaseInc;
  grain2PhaseAcc += grain2PhaseInc;

  // Convert phase into a triangle wave
  value = (grainPhaseAcc >> 7) & 0xff;
  if (grainPhaseAcc & 0x8000) value = ~value;
  // Multiply by current grain amplitude to get sample
  output = value * (grainAmp >> 8);

  // Repeat for second grain
  value = (grain2PhaseAcc >> 7) & 0xff;
  if (grain2PhaseAcc & 0x8000) value = ~value;
  output += value * (grain2Amp >> 8);

  // Make the grain amplitudes decay by a factor every sample (exponential decay)
  grainAmp -= (grainAmp >> 8) * grainDecay;
  grain2Amp -= (grain2Amp >> 8) * grain2Decay;

  // Scale output to the available range, clipping if necessary
//  output = output -  ampEnv;
//  output >>= 12;
  output >>= 9;
  if (output > 255) output = 255;

  // Output to PWM (this is faster than using analogWrite) 
  PWM_VALUE = output;
}
