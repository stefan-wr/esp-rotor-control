#include <ADS1115.h>

// ADS1115 constructor
ADS1115::ADS1115() {}

// => Initialise ADC
void ADS1115::init() {
  ads.setGain(GAIN_ONE);

  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS1115.");
  }
}

// => Read ADC channel c in SingleEnded mode
int ADS1115::readADC(const int c) { return ads.readADC_SingleEnded(c); }

// => Read ADC channel c in SingleEnded mode, return in volts
float ADS1115::readADCVolts(const int c) {
  return ads.computeVolts(ads.readADC_SingleEnded(c));
}
