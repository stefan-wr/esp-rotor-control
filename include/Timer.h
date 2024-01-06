#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

class Timer {
private:
    unsigned long interval_ms;
    unsigned long start_ms;
    
public:
    Timer(unsigned long interval);
    int n_passed = 0;
    void start();
    bool passed();
    void reset();
};

#endif //TIMER_H
