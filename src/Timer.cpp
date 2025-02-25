#include <Arduino.h>
#include <Timer.h>

// A simple timer class that can be checked for expiration in the main loop.
// When it was checked and the timer has passed, the timer is restarted.
// The number of times the timer was checked and it was passed is counted.
Timer::Timer(): interval_ms(0) {
    start_ms = millis();
}

Timer::Timer(unsigned long interval): interval_ms(interval) {
    start_ms = millis();
}

// => Start/Restart timer
void Timer::start() {
    start_ms = millis();
}

// => Reset counter
void Timer::reset() {
    n_passed = 0;
}

// => Change interval of timer and restart
void Timer::changeInterval(unsigned long new_interval) {
    interval_ms = new_interval;
    start();
}

// => Test wether timer has passed. Restart timer if it did.
bool Timer::passed(const bool restart) {
    unsigned long current_ms = millis();

    // Timer has passed after interval, or after millis() have overflown.
    if (current_ms - start_ms >= interval_ms || current_ms < start_ms) {
        if (restart) {
            start_ms = current_ms;
            n_passed += 1;
        }
        return true;
    } else {
        return false;
    }
}
