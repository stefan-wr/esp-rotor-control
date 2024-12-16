#ifndef TIMER_H
#define TIMER_H

// A simple timer class that can be checked for expiration in the main loop.
// When it was checked and the timer has passed, the timer is restarted.
// The number of times the timer was checked and it was passed is counted.
class Timer {
private:
    unsigned long interval_ms;
    unsigned long start_ms;

public:
    Timer();
    Timer(unsigned long interval);
    
    // N of times, the timer was checked and returned true
    int n_passed = 0;

    // => Start/Restart timer
    void start();

    // => Reset counter
    void reset();

    // => Change interval of timer and restart
    void changeInterval(unsigned long new_interval);
    
    // => Return wether timer has expired. Restart timer if true.
    bool passed(const bool &restart = true);    
};

#endif //TIMER_H
