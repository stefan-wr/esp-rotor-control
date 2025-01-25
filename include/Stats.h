#ifndef STATS_H
#define STATS_H

#include <Arduino.h>

namespace Stats {

    // *************
    // Counter Class
    // *************
    class Counter {
    private:
        const char *_key;
        String _factor_key;
        uint32_t _value = 0;
        int _factor = 0;
        
        // => Load value from PREFS
        void _load();
        // => Save value to PREFS
        bool _save();
    public:
        // Constructor
        Counter(const char *key);

        // => Reset counter
        void reset();

        // Getters and setters
        uint32_t get() const { return _value; }
        uint32_t value() const { return _value; }
        uint32_t getFactor() const {return _factor; }
        uint32_t factor() const  { return _factor; }

        // => Print counter to Serial
        void printToSerial() const;
        void printlnToSerial() const;

        // => Add to value
        bool add(uint32_t summand);
    };
}

#endif //STATS_H
