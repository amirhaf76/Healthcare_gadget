/** 
 * MAX30205 as body temperature:
 * Module 	| Arduino Board
 * vcc 		  | 5v or 3.3 v
 * GND 		  | ground
 * SDA		  | A4 or SDA
 * SCL		  | A5 or SCL
 * 
 * MAX30102 as Oxi meter:
 * Module 	| Arduino Board
 * vcc 	  	| 3.3 v
 * GND   		| ground
 * SDA		  | SDA or A4
 * SCL		  | SCL or A5
 * 
 * ADXL325 as Accelerator Meter:
 * Module 	| Arduino Board
 * vcc 		  | 3.3 v
 * GND 		  | ground
 * x	  	  | A1
 * y		    | A2
 * z	    	| A3
 * 
 * KY-039 as Heartbeat Radio Sensor: <It was Canceled.>
 * Module 	| Arduino Board
 * vcc 		  | 5 v
 * GND 		  | ground
 * output	  | A0
 * 
 * AD8232 as Echocardiography:
 * Module 	| Arduino Board
 * vcc 		  | 3.3 v
 * GND 		  | ground
 * output	  | A0
 * LO+		  | 2
 * LO-		  | 3
 * SDN		  | from controller pin 7
 * 
 * SD Module:
 * Module 	| Arduino Board
 * vcc 		  | 3.3 v
 * GND 	  	| ground
 * CS		    | 10
 * MOSI	  	| MOSI = 11
 * MISO	  	| MISO = 12
 * SLK	  	| 13
 * 
 * Transistor Controller:
 * ...
 * 
*/


#include <Wire.h>
#include "Protocentral_MAX30205.h"
#include "health_helper.h"
#include <SPI.h>
#include <SD.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"


/* Gadget ID*/
#define ID_GADGET 0xf2
#define FILE_RECORDS "records"

// It is for debugging program.
#define DEBUG 0
#define SERIAL_LOG 1
#define SERIAL_STATUS 1

/* Global Variables */
#define PIN_ECHO_CONTROLLER 7
#define PIN_BODY_TEMP_CONTROLLER 6
#define PIN_HRB_OX_CONTROLLER 5
#define ECHO_CARDIO_TIME 5000
#define BODY_TEMP_TIME 5000
#define HRB_OX_TIME 5000


// Echo_cardio_module
#define LO_PLUS 2
#define LO_NEG  3

// SD 
#define CS 10
#define MOSI 11
#define MISO 12
#define SLK	13

enum Controller c = None;

// HRB_OX_module
MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

// HRB OX
int32_t bufferLength; //data length
int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid
uint16_t irBuffer[50]; //infrared LED sensor data
uint16_t redBuffer[50];  //red LED sensor data
// end hrb ox

// Temperature_module
MAX30205 tempSensor;

// ACCELEROMETER_MODULE
Point current_point;
const int xpin = A1;                  // x-axis of the accelerometer
const int ypin = A2;                  // y-axis
const int zpin = A3;                  // z-axis (only on 3-axis models)
const int CF = 1;



/* Declaration Part */
void logger(char *);

// Setup Functions 
void HRB_OX_module_down();
void HRB_OX_module_setup(); // ok
void Thermometer_module_setup(); // ok
void Accelerometer_Meter_setup(); // ok
void Echocardiogram_module_setup(); // ok

// Loop Step Functions
void HRB_OX_module_loop_step();  // ok
void Thermometer_module_loop_step(); // ok
void Accelerometer_loop_step(); // ok
int Echocardiogram_module_loop_step(); // ok




void logger(char * message) {
  #if SERIAL_LOG
    Serial.println(message);
  #else

  #endif
}

/* Stepup Declarations */
void Accelerometer_Meter_setup() {
  #if DEBUG
  Serial.println("Initializing Accelerometer...");
  #endif
  current_point.x = analogRead(xpin);
  current_point.y = analogRead(ypin);
  current_point.z = analogRead(zpin);
}


void Thermometer_module_setup() {
  #if DEBUG
  Serial.println("Initializing Thermometer ...");
  #endif
  short counter = 0;

  //scan for temperature in every 30 sec untill a sensor is found. Scan for both addresses 0x48 and 0x49
  while(!tempSensor.scanAvailableSensors()){
    if ( (++counter) == 3)
      return ;

    #if DEBUG
    Serial.println("Couldn't find the temperature sensor, please connect the sensor." );
    #endif 

    delay(5000);
  }

  tempSensor.begin();   // set continuos mode, active mode
}


void Echocardiogram_module_setup() {
  #if DEBUG
  Serial.println("Initializing Echo_cardio_module ...");
  #endif
  
  pinMode(LO_PLUS, INPUT); // Setup for leads off detection LO +
  pinMode(LO_NEG, INPUT); // Setup for leads off detection LO -
}

void HRB_OX_module_setup() {
  #if DEBUG
  Serial.println("Initializing...");
  #endif

  short counter = 0;

  // Initialize sensor
  while (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    if ((++counter) == 3) 
      return ;

    #if DEBUG
    Serial.println("HRB_OX_module was not found. Please check wiring/power. ");
    #endif

    delay(3000);
  }

  #if DEBUG
  Serial.println("Place your index finger on the sensor with steady pressure.");
  #endif

  // particleSensor.setup(); //Configure sensor with default settings
  // particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  // particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

  byte ledBrightness = 60; //Options: 0=Off to 255=50mA
  byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
}

void HRB_OX_module_down() {
  particleSensor.setPulseAmplitudeRed(0);
  particleSensor.setPulseAmplitudeGreen(0);
}


/* Loop Step Declarations */
void Accelerometer_loop_step() {

  Point temporary_point;
  temporary_point.x = analogRead(xpin);
  temporary_point.y = analogRead(ypin);
  temporary_point.z = analogRead(zpin);

  Point diff;
  diff.x = temporary_point.x - current_point.x;
  diff.y = temporary_point.y - current_point.y;
  diff.z = temporary_point.z - current_point.z;

  int result = noiseFilter(magnitude_calculated(diff), CF);

  current_point = temporary_point;

  #if DEBUG
  Serial.print("s:");
  if (bandWithFilter(result, 18, 30)) {
    ++steps;
    
    Serial.print(result);
  } else {
    Serial.print(0);
  }
    
  Serial.print(", r:");
  Serial.print(result);
  Serial.println();
  #else
  if (bandWithFilter(result, 18, 30))
    ++steps;
  #endif


  

  delay(100);
}
 
void Thermometer_module_loop_step() {

  // read temperature for every 100ms
  temperature = tempSensor.getTemperature(); 

  #if DEBUG
  Serial.print(temperature ,2);
  Serial.println("'c" );
  #endif

  delay(100);
}

/**
 * If one of electrodes are disconnected, it will return -1. 
 * Otherwise the result is output of module.
 */
int Echocardiogram_module_loop_step() {
  int res;

  // It Checks whether the electrodes are connected or not
  if((digitalRead(LO_NEG) == 1)||(digitalRead(LO_PLUS) == 1))
    res = -1;
  else 
    int res = analogRead(A0);

  #if DEBUG
  if (res == -1) 
    Serial.println("Electrode is disconnected!");
  else 
    Serial.println(res);
  #endif

  delay(1);

  return res;
}

void HRB_OX_module_loop_step() {
//  long irValue = particleSensor.getIR();
//
//  if (checkForBeat(irValue) == true) {
//    //We sensed a beat!
//    long delta = millis() - lastBeat;
//    lastBeat = millis();
//
//    beatsPerMinute = 60 / (delta / 1000.0);
//
//    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
//      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
//      rateSpot %= RATE_SIZE; //Wrap variable
//
//      //Take average of readings
//      beatAvg = 0;
//      for (byte x = 0 ; x < RATE_SIZE ; x++)
//        beatAvg += rates[x];
//      beatAvg /= RATE_SIZE;
//    }
//  }
//
//  Serial.print("IR=");
//  Serial.print(irValue);
//  Serial.print(", BPM=");
//  Serial.print(beatsPerMinute);
//  Serial.print(", Avg BPM=");
//  Serial.print(beatAvg);
//
//  if (irValue < 50000)
//    Serial.print(" No finger?");
//
//  Serial.println();      
  
  //

  bufferLength = 100; //buffer length of 100 stores 4 seconds of samples running at 25sps

  //read the first 100 samples, and determine the signal range
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (particleSensor.available() == false) //do we have new data?
      particleSensor.check(); //Check the sensor for new data

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample(); //We're finished with this sample so move to next sample

    Serial.print(F("red="));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.println(irBuffer[i], DEC);
  }

  //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

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
      while (particleSensor.available() == false) //do we have new data?
        particleSensor.check(); //Check the sensor for new data

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample(); //We're finished with this sample so move to next sample

      //send samples and calculation result to terminal program through UART
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
    }

    //After gathering 25 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  }
}

void switch_module(enum Controller c) {

  // turn off all modules
  digitalWrite(PIN_HRB_OX_CONTROLLER, LOW);
  digitalWrite(PIN_BODY_TEMP_CONTROLLER, LOW);
  digitalWrite(PIN_ECHO_CONTROLLER, LOW);

  #if DEBUG
  Serial.println("OFF all module in 2s");
  #endif
  // "OFF all module in 2s"
  delay(2000);

  // Turn on one of modules at the moment
  switch(c) {
    case HRB_OX_MODULE:
      digitalWrite(PIN_HRB_OX_CONTROLLER, HIGH);
      #if DEBUG
      Serial.println("HRB_OX_MODULE On");
      #endif

      break;
    case TEMPERATURE_MODULE:
      digitalWrite(PIN_BODY_TEMP_CONTROLLER, HIGH);
      #if DEBUG
      Serial.println("TEMPERATURE_MODULE On");
      #endif

      break;
    case ECHOCARDIOGRAM_MODULE:
      digitalWrite(PIN_ECHO_CONTROLLER, HIGH);
      #if DEBUG
      Serial.println("ECHOCARDIOGRAM_MODULE On");
      #endif  

      break;
    case None:
      #if DEBUG
      Serial.println("None On");
      #endif

      break;

    default:
      #if DEBUG
      Serial.println("Switch None");
      #endif

      break;
  }
  

  delay(2000);
}

// Just counting time for 3 seconds. 
void change(enum Controller c) {
  Serial.println("3 s to change.");
  delay(1000);
  Serial.println("2 s to change.");
  delay(1000);
  Serial.println("1 s to change.");
  delay(1000);
}

void run_module(enum Controller c) {
  unsigned long myTime = millis();
  
  switch(c) {
    case HRB_OX_MODULE:
      // Set up module
      HRB_OX_module_setup(); 

      // Loop for module
      while (millis() - myTime < HRB_OX_TIME)
      {
        HRB_OX_module_loop_step();
      }
      break;
    case TEMPERATURE_MODULE:
      // Set up module
      Thermometer_module_setup();

      // Loop for module
      while (millis() - myTime < BODY_TEMP_TIME)
      {
        Thermometer_module_loop_step();
      }
      break;
    case ECHOCARDIOGRAM_MODULE:
      // Set up module
      Echocardiogram_module_setup();

      int storageRes[100];
      size_t index = 0;

      // Loop for module
      while (millis() - myTime < ECHO_CARDIO_TIME)
      {
        storageRes[index++] = Echocardiogram_module_loop_step();

        if ( index == 100) {
          create_csv_file(storageRes, index);
          index = 0;
        }
      }

      create_csv_file(storageRes, index);
      break;

    default:
      break;
  }

}

// Setup
void setup() {

	Serial.begin(9600);
  Wire.begin();

  Accelerometer_Meter_setup();

  c =  None; 

}

// Loop
void loop() {

  if (Serial.available() > 0) {
    int input_key = Serial.parseInt(SKIP_ALL);
    if (input_key >= 0 && input_key < CONTROLLERS_NUMBERS) {
      c = (enum Controller)input_key;

      #if DEBUG
      Serial.print("change to ");
      Serial.println(input_key);
      #endif

      change(c);
      switch_module(c);
      run_module(c);
      switch_module(None);
    }
  }

  Accelerometer_loop_step();

  #if SERIAL_STATUS
  Serial.print("steps: ");
  Serial.print(steps);
  Serial.println();
  #endif
  
}

bool make_file_ready() {
  #if DEBUG
  // Open serial communications and wait for port to open:
  Serial.print("Initializing SD card...");
  #endif

  if (!SD.begin(4)) {
    #if DEBUG
    Serial.println("initialization failed!");
    #endif
    return false;
  }
  #if DEBUG
  Serial.println("initialization done.");
  #endif

  return true;
}

bool print_in_file(uint8_t val) {
  // making file ready
  if (!make_file_ready()) 
    return false; // file is not ready and return

  File myFile = SD.open(FILE_RECORDS, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    #if DEBUG
    Serial.print("Writing to test.txt...");
    #endif
    
    myFile.print(val);

    // close the file:
    myFile.close();

    #if DEBUG
    Serial.println("done.");
    #endif

    return true;
  } else {
    // if the file didn't open, print an error:
    #if DEBUG
    Serial.print("error opening ");
    Serial.print(FILE_RECORDS);
    Serial.println(" file");
    #endif

    return false;
  }
}

bool create_csv_file(const int buff[], size_t siz) {
  // making file ready
  if (!make_file_ready()) 
    return false; // file is not ready and return

  File myFile = SD.open("myFile.csv", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    #if DEBUG
    Serial.print("Writing to test.txt...");
    #endif
    
    for (size_t i = 0; i < siz; ++i)
      myFile.println(val);

    // close the file:
    myFile.close();

    #if DEBUG
    Serial.println("done.");
    #endif

    return true;
  } else {
    // if the file didn't open, print an error:
    #if DEBUG
    Serial.print("error opening ");
    Serial.print(FILE_RECORDS);
    Serial.println(" file");
    #endif

    return false;
  }
}

bool read_from_file() {
  // making file ready
  if (!make_file_ready()) 
    return false; // file is not ready and return

  // re-open the file for reading:

  File myFile = SD.open(FILE_RECORDS, FILE_READ);

  if (myFile) {
    #if DEBUG
    Serial.println(FILE_RECORDS);
    #endif

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }

    // close the file:
    myFile.close();

    return true;
  } else {
    // if the file didn't open, print an error:
    #if DEBUG
    Serial.println("error opening test.txt");
    #endif

    return false;
  }

}
