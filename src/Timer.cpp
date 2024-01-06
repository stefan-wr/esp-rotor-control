#include <Timer.h>

Timer::Timer(unsigned long interval): interval_ms(interval) {
    start_ms = millis();
}

void Timer::start() {
    start_ms = millis();
}

void Timer::reset() {
    n_passed = 0;
}

bool Timer::passed() {
    unsigned long current_ms = millis();

    // Timer has passed after interval, or after millis() have overflown.
    if (current_ms - start_ms >= interval_ms || current_ms < start_ms) {
        start_ms = current_ms;
        n_passed += 1;
        return true;
    } else {
        return false;
    }
}
