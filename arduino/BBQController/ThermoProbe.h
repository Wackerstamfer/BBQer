#ifndef ThermoProbe_h
#define ThermoProbe_h


#define THERMISTORNOMINAL 100000 // resistance at 25 degrees C
#define TEMPERATURENOMINAL 25    // temp. for nominal resistance (almost always 25 C)
#define NUMSAMPLES 5             // how many samples to take and average, more takes longer, but is more 'smooth'
#define SERIESRESISTOR 100000    // the value of the 'other' resistor


class ThermoProbe {
    byte pin;
    uint16_t samples[NUMSAMPLES];
    int sampleIndex = 0;
    float savedTemp = 0.0;
    int savedAverageADC = 0;
    long resistanceAt25 = 100000;
    int betaCoefficient = 3950;
    unsigned long lastMs = 0;
    
  public:

    ThermoProbe(byte attachToPin, long resistanceAt25, int betaCoefficient) :
      pin(attachToPin), resistanceAt25(resistanceAt25), betaCoefficient(betaCoefficient) {
    }

    static int sortDesc(const void *cmp1, const void *cmp2) {
      // Need to cast the void * to int *
      int a = *((int *)cmp1);
      int b = *((int *)cmp2);
      // The comparison
      return a > b ? -1 : (a < b ? 1 : 0);
    }

    void setup() {
        pinMode(pin, INPUT);
        analogReference(EXTERNAL);
    }

    void loop() {

      if (millis() - lastMs > 100) {
        lastMs = millis();
  
        samples[sampleIndex] = analogRead(pin);
        sampleIndex++;
        if (sampleIndex >= NUMSAMPLES) {
          sampleIndex = 0;

          // sort
          qsort(samples, NUMSAMPLES, sizeof(samples[0]), sortDesc);
          
          // average all the samples out without 1 min and 1 max value
          // simple filter
          float average = 0;
          for (int i=1; i<NUMSAMPLES-1; i++) {
             average += samples[i];
          }
          average /= NUMSAMPLES-2;
         
	  savedAverageADC = average;

          // convert the value to resistance
          average = 1023 / average - 1;
          average = SERIESRESISTOR / average;

          float steinhart;
          steinhart = average / resistanceAt25;        // (R/Ro)
          steinhart = log(steinhart);                  // ln(R/Ro)
          steinhart /= betaCoefficient;                // 1/B * ln(R/Ro)
          steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
          steinhart = 1.0 / steinhart;                 // Invert
          steinhart -= 273.15;                         // convert to C

          savedTemp = steinhart < 0 ? 0 : steinhart;
        }
     
      }
    }

    float readCelsius() {
      return savedTemp;
    }

    int readADC() {
      return savedAverageADC;
    }
    
    void printState() {
        Serial.print("[PROBE ");
        Serial.print(pin);
        Serial.print("] -> ");
        Serial.print(readCelsius());
        Serial.println();
    }

};

#endif
