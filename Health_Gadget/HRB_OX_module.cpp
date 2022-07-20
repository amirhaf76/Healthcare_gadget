#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "heartRate.h"
#include "health_helper.h"

#define DEBUG 1
#define DETAILS_DEBUG 1

MAX30105 particleSensor;
static unsigned long timer;

#define MAX_BRIGHTNESS 255
#define SAMPLE_BUFFER_SIZE 100
#define SAMPLE_DUMPING_SIZE 25
#define DURING_LIFE_TIME_IN_SECONDS 2


#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
//Arduino Uno doesn't have enough SRAM to store 100 samples of IR led data and red led data in 32-bit format
//To solve this problem, 16-bit MSB of the sampled data will be truncated. Samples become 16-bit data.
uint16_t irBuffer[100]; //infrared LED sensor data
uint16_t redBuffer[100];  //red LED sensor data
#else
uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data
#endif

int32_t bufferLength; //data length
int32_t spo2; //SPO2 value
int32_t heartRate; //heart rate value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid

byte pulseLED = 11; //Must be on PWM pin
byte readLED = 13; //Blinks with each data read

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
static byte rateSpot = 0;
static long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;

static bool first_sampling;


void HRB_OX_module_set_config();
void HRB_OX_module_set_config2();
void HRB_OX_module_processing_samples(unsigned long);

void HRB_OX_module_setup_at_first()
{
    #if DEBUG
    Serial.begin(9600); // initialize serial communication at 115200 bits per second:
    #endif

    int8_t counter = 0;

    // pinMode(pulseLED, OUTPUT);
    // pinMode(readLED, OUTPUT);

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

    first_sampling = true;

    HRB_OX_module_set_config();
}

void HRB_OX_module_loop_step_new() 
{
  HRB_OX_module_processing_samples((unsigned long) DURING_LIFE_TIME_IN_SECONDS);
}

void HRB_OX_module_loop_step() 
{
  // Setting timer.
  set_time(&timer);

  // buffer length of 100 stores 4 seconds of samples running at 25sps
  bufferLength = 100; 
  
  //read the first 100 samples, and determine the signal range
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (!particleSensor.available()) {
      particleSensor.check(); //Check the sensor for new data
    } 

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    
    //We're finished with this sample so move to next sample
    particleSensor.nextSample(); 

    #ifdef DETAILS_DEBUG
    Serial.print(F("red="));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.println(irBuffer[i], DEC);
    #endif
  }

  //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  uint8_t counter = 10;
  //Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second
  while (1)
  {
    //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    //take 25 sets of samples before calculating the heart rate.
    for (byte i = 75; i < 100; i++)
    {
      while (!particleSensor.available()) {
        particleSensor.check(); //Check the sensor for new data      
      } 

      // digitalWrite(readLED, !digitalRead(readLED)); //Blink onboard LED with every data read

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample(); //We're finished with this sample so move to next sample

      //send samples and calculation result to terminal program through UART
      #ifdef DETAILS_DEBUG
      Serial.print(F("red="));
      Serial.print(redBuffer[i], DEC);
      Serial.print(F(", ir="));
      Serial.print(irBuffer[i], DEC);

      Serial.print(F(", HR="));
      Serial.print(heartRate, DEC);

      Serial.print(F(", HRvalid="));
      Serial.print(validHeartRate, DEC);

      Serial.print(F(", SPO2="));
      Serial.print(spo2, DEC);

      Serial.print(F(", SPO2Valid="));
      Serial.println(validSPO2, DEC);
      #endif
    }

    //After gathering 25 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  }
 
}

void HRB_OX_module_wakeup() 
{
  particleSensor.wakeUp();

  first_sampling = true;

  delay(1000);
}

void HRB_OX_module_shotdown() 
{
  particleSensor.shutDown();
  delay(1000);
}

// during_work_time_in_second should be atleast 2 seconds.
void HRB_OX_module_processing_samples(unsigned long during_work_time_in_second) 
{
  // Setting timer.
  set_time(&timer);

  // Setting index.
  byte index = 0;

  // ======== Checking Frist Sampling ========
  // Checking whether it's first sampling or not. 
  if (!first_sampling)
  {
    index = SAMPLE_BUFFER_SIZE - SAMPLE_DUMPING_SIZE;

    // Dumping samples.
    for (byte i = SAMPLE_DUMPING_SIZE; i < SAMPLE_BUFFER_SIZE; i++) 
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }
  }
  else
  {
    first_sampling = false;
  }

  // ======== Gathering samples ========
  for (byte i = index; i < SAMPLE_BUFFER_SIZE ; i++) 
  {
    while (!particleSensor.available()) 
    {
      particleSensor.check(); //Check the sensor for new data
      
      if (is_time_pass(&timer, during_work_time_in_second)) 
      {
        //return ;
      }
    } 

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
  
    //We're finished with this sample so move to next sample
    particleSensor.nextSample(); 

    #ifdef DETAILS_DEBUG
      Serial.print(F("red="));
      Serial.print(redBuffer[i], DEC);
      Serial.print(F(", ir="));
      Serial.print(irBuffer[i], DEC);

      Serial.print(F(", HR="));
      Serial.print(heartRate, DEC);

      Serial.print(F(", HRvalid="));
      Serial.print(validHeartRate, DEC);

      Serial.print(F(", SPO2="));
      Serial.print(spo2, DEC);

      Serial.print(F(", SPO2Valid="));
      Serial.println(validSPO2, DEC);
    #endif
  }

  // ======== Calculating on Samples ========
  // calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
}

void HRB_OX_module_set_config() 
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
  Serial.println(F("MAX30105 configuration is set."));
  #endif
}

void HRB_OX_module_set_config2() {
  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}

void get_bpm() {
    long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true) {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  #ifdef DETAILS_DEBUG
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);

  if (irValue < 50000)
    Serial.print(" No finger?");

  Serial.println();
  #endif
}

int32_t get_spo2() 
{
  return spo2;
}

int32_t heartget_Rate() 
{
  return heartRate;
}

int8_t get_valid_SPO2() 
{
  return validSPO2;
}

int8_t get_valid_HeartRate() 
{
  return validHeartRate;
}

void get_results(int32_t * out_spo2, int32_t * out_heartRate, int8_t * out_validSPO2, int8_t * out_validHeartRate) 
{
  *out_spo2 = spo2;
  *out_heartRate = heartRate;
  *out_validSPO2 = validSPO2;
  *out_validHeartRate = validHeartRate;  
}