#ifndef Timer_h
#define Timer_h

class Timer {
  public:
    Timer(double _bpm, uint8_t _clkTrig1, uint8_t _clkTrig2, uint8_t _clkOut);
    bool tick();
    void setBPM(double);
  private:
    bool useInternalCLK = false;
    bool clkState;
    uint8_t oldTrig1State = 0;
    uint8_t trig1State = 0;
    uint8_t subDivision = 1;
    double bpm;
    uint8_t clkTrig1;
    uint8_t clkTrig2;
    uint8_t clkOut;
    unsigned long interval;
    unsigned long previousMillis = 0;
};

#endif
