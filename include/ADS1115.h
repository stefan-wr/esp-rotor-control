#ifndef ADS1115_H
#define ADS1115_H

#include <Arduino.h>
#include <Adafruit_ADS1X15.h>

class ADS1115 {
  public:
    Adafruit_ADS1115 ads;

    ADS1115();
    void init();
    int readADC(const int c);
    float readADCVolts(const int c);
};

#endif //ADS1115_H
