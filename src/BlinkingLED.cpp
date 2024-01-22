#include <Arduino.h>
#include <BlinkingLED.h>
#include <Timer.h>

BlinkingLED::BlinkingLED(const uint8_t &pin, const uint8_t &normal, const unsigned long &interval) {
    _pin = pin;
    _normal = normal;
    _interval = interval;
    _tickTimer = new Timer(_interval);
    pinMode(_pin, OUTPUT);
    write(_normal);
}

// => Set blink interval
void BlinkingLED::setInterval(const unsigned long &interval) {
    _interval = interval;
    _tickTimer->changeInterval(_interval);
}

// **********

// => Set number of times LED should blink, non-blocking
void BlinkingLED::blink(const int &n) {
    reset();
    _n_blinks = n * 2;
}

// => Set number of times LED should blink with given interval, non-blocking
void BlinkingLED::blink(const int &n, const unsigned long &interval) {
    setInterval(interval);
    blink(n);
}

// => Blink LED n number of times with given interval. Blocking.
// Does not stop async blinking.
void BlinkingLED::blinkBlocking(const int &n, unsigned long interval) {
    for (int i = 0; i < n * 2; i++) {
        toggle();
        if (interval) {
            delay(interval);
        } else {
            delay(_interval);
        }
    }
}

// **********

// => Start blinking continously with current interval
void BlinkingLED::startBlinking() {
    _continous = true;
}

// => Start blinking continously with given interval
void BlinkingLED::startBlinking(const unsigned long &interval) {
    setInterval(interval);
    startBlinking();
}

// => Stop all blinking, set default state
void BlinkingLED::stopBlinking() {
    normal();
}

// **********

// => Turn on LED, stops blinking
void BlinkingLED::on() {
    reset();
    write(HIGH);
}

// => Turn off LED, stops blinking
void BlinkingLED::off() {
    reset();
    write(LOW);
}

// => Set LED to default state, stops blinking
void BlinkingLED::normal() {
    reset();
    write(_normal);
}

// => Toggle LED, does not stop blinking
void BlinkingLED::toggle() {
    write(!read());
}

// **********

// => Invert current and default LED status
void BlinkingLED::invert(){
    toggle();
    _normal = !_normal;
}

// => Stop all non-blocking blinking
void BlinkingLED::reset() {
    _n_blinks = 0;
    _continous = false;
}

// => Block blinking
void BlinkingLED::lock() {
    _isLocked = true;
}

// => Allow blinking, resets blinking
void BlinkingLED::unlock() {
    reset();
    _isLocked = false;
}

// **********

// => Async LED blinking, to be called from main loop
// Does nothing if LED was turned on manually
void BlinkingLED::tick() {
    if (_isLocked) {
        return;
    }

    if (_tickTimer->passed()) {

        if (_n_blinks > 0 || _continous) {
            write(!read());
            _n_blinks--;
            return;
        }

        if (_n_blinks == 0 && _prev_n_blinks) {
            write(_normal);
        }

        _prev_n_blinks = _n_blinks;
    }
}
