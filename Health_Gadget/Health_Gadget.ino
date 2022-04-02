/** 
 * MAX30205 as body temperature:
 * Module 	| Arduino Board
 * vcc 		| 5v or 3.3 v
 * GND 		| ground
 * SDA		| A4 or SDA
 * SCL		| A5 or SCL
 * 
 * MAX30102 as Oxi meter:
 * Module 	| Arduino Board
 * vcc 		| 3.3 v
 * GND 		| ground
 * SDA		| SDA or A4
 * SCL		| SCL or A5
 * 
 * ADXL325 as Accelerator Meter:
 * Module 	| Arduino Board
 * vcc 		| 3.3 v
 * GND 		| ground
 * x		| A1
 * y		| A2
 * z		| A3
 * 
 * KY-039 as Heartbeat Radio Sensor:
 * Module 	| Arduino Board
 * vcc 		| 5 v
 * GND 		| ground
 * output	| A0
 * 
 * AD8232 as Echocardiography:
 * Module 	| Arduino Board
 * vcc 		| 3.3 v
 * GND 		| ground
 * output	| A0
 * LO+		| 2
 * LO-		| 3
 * SDN		| 4
 * 
 * SD Module:
 * Module 	| Arduino Board
 * vcc 		| 3.3 v
 * GND 		| ground
 * CS		  | 13
 * MOSI		| MOSI = 12
 * MISO		| MISO = 11
 * SLK		| 10
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


/* Gadget ID*/
#define ID_GADGET 0xf2
#define FILE_RECORDS "records"

// It is for debugging program.
#define DEBUG 1 
#define SERIAL_LOG 1


/* Declaration Part */
void logger(char *);

// Setup Functions 
void HRB_OX_module_setup(); // ok
void Temperature_module_setup(); // ok
void Accelerator_Meter_setup(); // ok
//void SD_setup();
void Echo_cardio_module_setup(); // ok

// Loop Step Functions
void HRB_OX_module_loop_step();  // ok
void Temperature_module_loop_step(); // ok
void Accelerator_Meter_loop_step(); // ok
//void SD_loop_step();
void Echo_cardio_module_loop_step(); // ok


/* Global Variables */

enum Controller c = None;
#define PIN_ECHO_CONTROLLER 7
#define PIN_BODY_TEMP_CONTROLLER 6
#define PIN_HRB_OX_CONTROLLER 5
#define ECHO_CARDIO_TIME 5000
#define BODY_TEMP_TIME 5000
#define HRB_OX_TIME 5000

// HRB_OX_module
MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

// Temperature_module
MAX30205 tempSensor;
// Accelerator_Meter
Point p;
const int xpin = A1;                  // x-axis of the accelerometer
const int ypin = A2;                  // y-axis
const int zpin = A3;                  // z-axis (only on 3-axis models)
const int CF = 1;
long unsigned steps = 0;
// Echo_cardio_module
#define LO_PLUS 2
#define LO_NEG  3
// SD 
#define CS 13
#define MOSI 12
#define MISO 11
#define SLK	10

void logger(char * message) {
  #if SERIAL_LOG
    Serial.println(message);
  #else

  #endif
}

/* Stepup Declarations */
void Accelerator_Meter_setup() {
  p.x = analogRead(xpin);
  p.y = analogRead(ypin);
  p.z = analogRead(zpin);
}


void Temperature_module_setup() {
  logger("Initializing...");
  short counter = 0;

  //scan for temperature in every 30 sec untill a sensor is found. Scan for both addresses 0x48 and 0x49
  while(!tempSensor.scanAvailableSensors()){
    if ( (counter++) == 3)
      return ;
    logger("Couldn't find the temperature sensor, please connect the sensor." );
    delay(5000);
  }

  tempSensor.begin();   // set continuos mode, active mode
}


void Echo_cardio_module_setup() {
  pinMode(LO_PLUS, INPUT); // Setup for leads off detection LO +
  pinMode(LO_NEG, INPUT); // Setup for leads off detection LO -
}

void HRB_OX_module_setup() {
  logger("Initializing...");
  short counter = 0;

  // Initialize sensor
  while (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    if ((counter++) == 3) 
      return ;

    logger("HRB_OX_module was not found. Please check wiring/power. ");

    delay(3000);
  }

  logger("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}


/* Loop Step Declarations */
void Accelerator_Meter_loop_step() {

  Point temp;
  temp.x = analogRead(xpin);
  temp.y = analogRead(ypin);
  temp.z = analogRead(zpin);

  Point diff;
  diff.x = temp.x - p.x;
  diff.y = temp.y - p.y;
  diff.z = temp.z - p.z;

  double res = magnitude_calculated(diff);

  // Serial.print(noiseFilter(res, CF));
  int result = noiseFilter(res, CF);

  p = temp;
 
  if (12 <=result && result <= 15){
    ++steps;
    Serial.print(res);
  }
  else {
    Serial.print(0);
  }

  // delay before next reading:
  delay(100);
}
 
void Temperature_module_loop_step() {

  float temp = tempSensor.getTemperature(); // read temperature for every 100ms
  Serial.print(temp ,2);
  Serial.println("'c" );

  delay(100);
}

void Echo_cardio_module_loop_step() {
 
  if((digitalRead(LO_NEG) == 1)||(digitalRead(LO_PLUS) == 1)){
    logger('!');
  }
  else{
    // send the value of analog input 0:
    logger(analogRead(A0));
  }
  delay(1);
}

void HRB_OX_module_loop_step() {
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

  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);

  if (irValue < 50000)
    Serial.print(" No finger?");

  Serial.println();      
  
}

void switch_module(enum Controller c) {

  digitalWrite(PIN_HRB_OX_CONTROLLER, LOW);
  digitalWrite(PIN_BODY_TEMP_CONTROLLER, LOW);
  digitalWrite(PIN_ECHO_CONTROLLER, LOW);

  delay(2000);

  switch(c) {
    case HRB_OX_MODULE:
      digitalWrite(PIN_HRB_OX_CONTROLLER, HIGH);
      break;
    case TEMPERATURE_MODULE:
      digitalWrite(PIN_BODY_TEMP_CONTROLLER, HIGH);
      break;
    case ECHO_CARDIO_MODULE:
      digitalWrite(PIN_ECHO_CONTROLLER, HIGH);
      break;
    default:
      Serial.print("Error. It needs to see codes. Line: ");
      Serial.print(__LINE__);
      Serial.print(", File: ");
      Serial.println(__FILE__);
      break;
  }
  
  delay(3000);
}

// change 
void change(enum Controller c) {
  Serial.println("3 s to change.");
  delay(1000);
  Serial.println("2 s to change.");
  delay(1000);
  Serial.println("1 s to change.");
  delay(1000);
}

// Setup
void setup() {

	Serial.begin(9600);
  Wire.begin();

  Accelerator_Meter_setup()

  c =  Controller.None; 

}

void do_loop(enum Controller c) {
  unsigned long myTime = millis();
  
  switch(c) {
    case HRB_OX_MODULE:
      while (millis - myTime < HRB_OX_TIME)
      {
        HRB_OX_module_loop_step();
      }
      break;
    case TEMPERATURE_MODULE:
      while (millis - myTime < BODY_TEMP_TIME)
      {
        Temperature_module_loop_step();
      }
      break;
    case ECHO_CARDIO_MODULE:
      while (millis - myTime < ECHO_CARDIO_TIME)
      {
        Echo_cardio_module_loop_step();
      }
      break;
    default:
      break;
  }
}


// Loop
void loop() {

  if (Serial.available() > 0) {
    int temp = Serial.parseInt();
    if (temp >= 0 && temp < CONTROLLERS_NUMBERS) {
      c = (enum Controller)temp;
      change(c);
      switch_module(c);
    }
  }

  Accelerator_Meter_loop_step();

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

bool write_in_file(int len, char buff[]) {
  // making file ready
  if (!make_file_ready()) 
    return false; // file is not ready and return

  File myFile = SD.open(FILE_RECORDS, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    #if DEBUG
    Serial.print("Writing to test.txt...");
    #endif
    
    myFile.write(buff, len);

    // close the file:
    myFile.close();

    #if DEBUG
    Serial.println("done.");
    #endif
  } else {
    // if the file didn't open, print an error:
    #if DEBUG
    Serial.print("error opening ");
    Serial.print(FILE_RECORDS);
    Serial.println(" file");
    #endif
  }
}

void read_from_file(int len, char buff[]) {
  // making file ready
  if (!make_file_ready()) 
    return false; // file is not ready and return

  // re-open the file for reading:
  File myFile = SD.open(FILE_RECORDS);

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
  } else {
    // if the file didn't open, print an error:
    #if DEBUG
    Serial.println("error opening test.txt");
    #endif
  }
}
