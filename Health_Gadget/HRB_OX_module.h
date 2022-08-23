#ifndef HRB_OX_MODULE_H
#define HRB_OX_MODULE_H

#include "MAX30105.h"

#define SAMPLE_BUFFER_SIZE 100
#define SAMPLE_DUMPING_SIZE 25

class HRBandO2Module {
    public:
        
        static const int8_t RATE_SIZE = 4;
        
        HRBandO2Module();

        void setUpModule();

        
        void spo2ConfigSetUp();
        void spo2Loop();
        void spo2Loop(unsigned long duration);

        void heartBeatConfigSetup();
        float heartBeatStepLoop();


        int getAveHeartRadio();
        int getSpo2(bool * isValidSpo2 = nullptr);

        void wakeup();        
        void shotDown();
    private:
        MAX30105 particleSensor;

        // ----- Heart Radio -----
        byte rates[RATE_SIZE]; //Array of heart rates
        byte rateSpot = 0;
        long lastBeat = 0; //Time at which the last beat occurred

        float beatsPerMinute;
        int beatAvg;

        // ----- SPO2 -----
        int32_t spo2 = 0; //SPO2 value
        int8_t validSPO2 = 0; //indicator to show if the SPO2 calculation is valid

        #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
        //Arduino Uno doesn't have enough SRAM to store 100 samples of IR led data and red led data in 32-bit format
        //To solve this problem, 16-bit MSB of the sampled data will be truncated. Samples become 16-bit data.
        uint16_t irBuffer[SAMPLE_BUFFER_SIZE]; //infrared LED sensor data
        uint16_t redBuffer[SAMPLE_BUFFER_SIZE];  //red LED sensor data
        #else
        uint32_t irBuffer[SAMPLE_BUFFER_SIZE]; //infrared LED sensor data
        uint32_t redBuffer[SAMPLE_BUFFER_SIZE];  //red LED sensor data
        #endif
        void dumpSamples();
        bool spo2LoopStepLoop(bool);
    


};

#endif