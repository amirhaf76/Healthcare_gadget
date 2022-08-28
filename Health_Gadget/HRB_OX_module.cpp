#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "heartRate.h"
#include "health_helper.h"
#include "time_controlling.hh"

#include "HRB_OX_module.h"

#define DEBUG 1
#define HRB_DEBUG 1
#define MAX_BRIGHTNESS 255
#define DURING_LIFE_TIME_IN_SECONDS 3000

HRBandO2Module::HRBandO2Module() {

}

void HRBandO2Module::setUpModule()
{
    #if DEBUG
    Serial.begin(9600); // initialize serial communication at 115200 bits per second:
    #endif

    int8_t counter = 0;

    // Initialize sensor
    while (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
    {
        #if DEBUG
        Serial.println(F("MAX30105 was not found. Please check wiring/power."));
        #endif
       if ((counter++) == 3) 
            return ;
    }

    #if DEBUG
    Serial.println(F("MAX30105 initilized ..."));
    #endif
}


void HRBandO2Module::wakeup() 
{
  particleSensor.wakeUp();
  delay(1000);
}


void HRBandO2Module::shotDown() 
{
  particleSensor.shutDown();
  delay(1000);
}


void HRBandO2Module::dumpSamples() {
  // Dumping samples.
  for (byte i = SAMPLE_DUMPING_SIZE; i < SAMPLE_BUFFER_SIZE; i++) 
  {
    redBuffer[i - SAMPLE_DUMPING_SIZE] = redBuffer[i];
    irBuffer[i - SAMPLE_DUMPING_SIZE] = irBuffer[i];
  }
}


void HRBandO2Module::heartBeatConfigSetup() 
{
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

  beatAvg = 0;

  #if DEBUG
  Serial.println(F("MAX30105 heart beat configuration is set."));
  #endif
}


float HRBandO2Module::heartBeatStepLoop() 
{
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  #if DEBUG
  Serial.print(F("IR="));
  Serial.print(irValue);
  Serial.print(F(", BPM="));
  Serial.print(beatsPerMinute);
  Serial.print(F(", Avg BPM="));
  Serial.print(beatAvg);

  if (irValue < 50000)
    Serial.print(F(" No finger?"));

  Serial.println();
  #endif

  return beatsPerMinute;
}


void HRBandO2Module::spo2ConfigSetUp() 
{
  // Options: 0=Off to 255=50mA
  byte ledBrightness = 60;

  // Options: 1, 2, 4, 8, 16, 32
  byte sampleAverage = 4;

  // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte ledMode = 2;

  // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  byte sampleRate = 100;

  // Options: 69, 118, 215, 411
  int pulseWidth = 411;

  // Options: 2048, 4096, 8192, 16384
  int adcRange = 4096;

  //Configure sensor with these settings
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

  #if DEBUG
  Serial.println(F("MAX30105 spo2 configuration is set."));
  #endif
}


// during_work_time_in_second should be atleast 2 seconds.
bool HRBandO2Module::spo2LoopStepLoop(bool dumping=false) 
{
  // Heart radio calculate in another funtion.
  int32_t heartRate =0;
  int8_t validHeartRate =0;

  unsigned long timer;
  
  byte index = 0;

  if (dumping) {
    index =  SAMPLE_BUFFER_SIZE - SAMPLE_DUMPING_SIZE;
    dumpSamples();
  }
  // Setting index.

  // ======== Gathering samples ========
  for (byte i = index; i < SAMPLE_BUFFER_SIZE ; i++) 
  {
    // Setting timer.
    set_time(&timer);
    
    while (!particleSensor.available()) 
    {
      particleSensor.check(); //Check the sensor for new data
      
      if (is_time_pass(&timer,(unsigned long) DURING_LIFE_TIME_IN_SECONDS)) 
      {
        #if DEBUG
        Serial.println(F("Data is not available"));
        #endif

        return false;
      }
    } 

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
  
    //We're finished with this sample so move to next sample
    particleSensor.nextSample(); 

    #ifdef DEBUG
      Serial.print(F("red="));
      Serial.print(redBuffer[i], DEC);
      Serial.print(F(", ir="));
      Serial.println(irBuffer[i], DEC);
      #endif
  }

  // ======== Calculating on Samples ========
  // calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, SAMPLE_BUFFER_SIZE, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  return true;
}

void HRBandO2Module::spo2Loop() {
  spo2Loop(20000LU);
}

void HRBandO2Module::spo2Loop(unsigned long duration) 
{
  unsigned long inital_timer;
  bool first_step = true;

  set_time(&inital_timer);
  int32_t sum_spo2 = 0;
  int8_t counter = 0;
  while (!is_time_pass(&inital_timer, duration))
  { 
    if (first_step) 
    {
      spo2LoopStepLoop(false);
      first_step = false;
    }
    else {
      spo2LoopStepLoop(true);
    }
    #if DEBUG
      Serial.print(F("=========SPO2="));
      Serial.print(spo2, DEC);

      Serial.print(F(", SPO2Valid="));
      Serial.println(validSPO2, DEC);
    #endif

    if (validSPO2 == 1)
    {
      sum_spo2 += spo2;
      counter++;
    }
  }

  spo2 = (int32_t) (sum_spo2 / counter);
}


int HRBandO2Module::getSpo2(bool * isValidSpo2) 
{
  
  *isValidSpo2 = validSPO2;
  return spo2;
}


int HRBandO2Module::getAveHeartRadio() 
{
  return beatAvg;
}
