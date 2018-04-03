#ifndef Timer_h
#define Timer_h

class Timer {
  public:
    Timer(double);
    double bpm;
    uint8_t subDivision = 1;
    bool tick();
    void setBPM(double);
  private:
    unsigned long interval;
    unsigned long previousMillis = 0;
};

#endif
