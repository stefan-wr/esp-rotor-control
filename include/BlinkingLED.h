#ifndef BLINKINGLED_H
#define BLINKINGLED_H

#include <Arduino.h>
#include <Timer.h>

class BlinkingLED {
private:
    uint8_t _pin;
    uint8_t _normal;
    unsigned long _interval;
    bool _isLocked;
    int _n_blinks = 0;
    int _prev_n_blinks = 0;
    bool _continous = false;
    Timer _tickTimer;

public:
    // Constructor
    BlinkingLED(const uint8_t &pin, const uint8_t &normal, const unsigned long &interval);

    // => Set blink interval
    void setInterval(const unsigned long &interval);

    // **********

    // => Set number of times LED should blink, non-blocking
    void blink(const int &n);

    // => Set number of times LED should blink with given interval, non-blocking
    void blink(const int &n, const unsigned long &interval);

    // => Blink LED n number of times with given interval. Blocking.
    // Does not stop async blinking.
    void blinkBlocking(const int &n, unsigned long interval = 0ul) const;

    // **********

    // => Start blinking continously with current interval
    void startBlinking();

    // => Start blinking continously with given interval
    void startBlinking(const unsigned long &interval);

    // => Stop all blinking, set default state
    void stopBlinking();

    // **********

    // => Turn on LED, stops blinking
    void on();

    // => Turn off LED, stops blinking
    void off();

    // => Set LED to default state, stops blinking
    void normal();

    // => Toggle LED, does not stop blinking
    void toggle() const;

    // **********

    // => Invert current and default LED status
    void invert();

    // => Stop all non-blocking blinking
    void reset();

    // => Block blinking
    void lock();
    
    // => Allow blinking
    void unlock();

    // **********

    // => Async LED blinking, to be called from main loop
    // Does nothing if LED was turned on manually
    void tick();

    // **********

    // Helpers
    int read() const { return digitalRead(_pin); }
    void write(const uint8_t &state) const { digitalWrite(_pin, state); }
    void setNormal(const uint8_t &normal) { _normal = normal; }
    uint8_t getNormal() const { return _normal; }
    unsigned long getInterval() const { return _interval; }
    unsigned long getTimeLeft() const { return _interval * _n_blinks; }
};

#endif //BLINKINGLED_H
