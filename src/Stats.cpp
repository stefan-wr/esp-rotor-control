#include <Arduino.h>
#include <Preferences.h>
#include <Stats.h>
#include <stdint.h>

#define STATS_PREFS_KEY "statsPrefs"

namespace Stats {

    Preferences stats_prefs;

    // *************
    // Counter Class
    // *************

    // Constructor
    Counter::Counter(const char *key) {
        _key = key;
        _factor_key = (String) _key + "Factor";
        _load();
    }

    // => Load value from PREFS
    void Counter::_load() {
        bool prefs_exists = stats_prefs.begin(STATS_PREFS_KEY, true);
        _value = stats_prefs.getULong(_key, 1UL);
        _factor = stats_prefs.getInt(_factor_key.c_str(), 0);
        stats_prefs.end();
        if(!prefs_exists) {
            Serial.print("{[Stats] Creating counter {");
            Serial.print(_key);
            Serial.println("} in PREFS.");
            _save();
        }
    }

    // => Save value to PREFS
    bool Counter::_save() {
        if(stats_prefs.begin(STATS_PREFS_KEY, false)) {
            stats_prefs.putULong(_key, _value);
            stats_prefs.putInt(_factor_key.c_str(), _factor);
            stats_prefs.end();
            return true;
        } 
        Serial.print("{[Stats] Failed to save counter {");
        Serial.print(_key);
        Serial.println("} in PREFS.");
        return false;
    }

    // => Reset counter
    void Counter::reset() {
        _value = 0;
        _factor = 0;
        _save();
    }

    // => Add to value
    bool Counter::add(uint32_t summand) {
        if (UINT32_MAX - summand < _value) {
            _factor++;
        }
        _value += summand;
        return _save();
    }

    // => Print counter to Serial
    void Counter::printToSerial() const {
        Serial.print(_value);
        if (_factor > 0) {
            Serial.print(" + ");
            Serial.print(UINT32_MAX);
            if (_factor > 1) {
                Serial.print(" * ");
                Serial.print(_factor);
            }
        }
    }

    void Counter::printlnToSerial() const {
        printToSerial();
        Serial.print("\n\r");
    }
}
