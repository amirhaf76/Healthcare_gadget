#include <Wire.h>
#include "Protocentral_MAX30205.h"
#include "health_helper.h"

#define ID_GADGET 0xf2

// Setup Functions 
//void MAX30102_setup(); 
void MAX30205_setup(); // ok
//void KY_039_setup();
void ADXL_325_setup(); // ok
//void SD_setup();
void AD8232_setup(); // ok

// Loop Step Functions
//void MAX30102_loop_step();
void MAX30205_loop_step(); // ok
//void KY_039_loop_step();
void ADXL_325_loop_step(); // ok
//void SD_loop_step();
void AD8232_loop_step(); // ok


MAX30205 tempSensor;
enum Controller c = None;

// ADXL_325
Point p;
const int xpin = A0;                  // x-axis of the accelerometer
const int ypin = A1;                  // y-axis
const int zpin = A2;                  // z-axis (only on 3-axis models)
const int CF = 1;
// AD8232
const int LO_PLUS = 10;
const int LO_NEG = 11;


// Stepup Declarations
void ADXL_325_setup() {
  p.x = analogRead(xpin);
  p.y = analogRead(ypin);
  p.z = analogRead(zpin);
}


void MAX30205_setup() {
  Wire.begin();

  //scan for temperature in every 30 sec untill a sensor is found. Scan for both addresses 0x48 and 0x49
  while(!tempSensor.scanAvailableSensors()){
    Serial.println("Couldn't find the temperature sensor, please connect the sensor." );
    delay(30000);
  }

  tempSensor.begin();   // set continuos mode, active mode
}


void AD8232_setup() {
  pinMode(LO_PLUS, INPUT); // Setup for leads off detection LO +
  pinMode(LO_NEG, INPUT); // Setup for leads off detection LO -
}


// Loop Step Declarations
void ADXL_325_loop_step() {

  Point temp;
  temp.x = analogRead(xpin);
  temp.y = analogRead(ypin);
  temp.z = analogRead(zpin);

  Point diff;
  diff.x = temp.x - p.x;
  diff.y = temp.y - p.y;
  diff.z = temp.z - p.z;

  double res = magnitude_calculated(diff);
  Serial.print(noiseFilter(res, CF));
  p.x = temp.x;
  p.y = temp.y;
  p.z = temp.z;
}
 
void MAX30205_loop_step() {

  float temp = tempSensor.getTemperature(); // read temperature for every 100ms
  Serial.print(temp ,2);
  Serial.println("'c" );
  delay(100);
}

void AD8232_loop_step() {
 
  if((digitalRead(LO_NEG) == 1)||(digitalRead(LO_PLUS) == 1)){
    Serial.println('!');
  }
  else{
  // send the value of analog input 0:
  Serial.println(analogRead(A0));
  }
}

void change(enum Controller c) {
  switch (c)
  {
    case MAX30102_: 
//      MAX30102_loop_step();
      break;
    case MAX30205_: 
      MAX30205_loop_step();
      break;
    case KY_039_: 
//      KY_039_loop_step();
      break;
    case ADXL_325_: 
      ADXL_325_loop_step();
      break;
    case SD_: 
//      SD_loop_step();
      break;
    case AD8232_: 
      AD8232_loop_step();
      break;
    case None: 
      break;
    default:
      Serial.print("Error. It needs to see codes.");
  }
}

// Setup
void setup() {
  Serial.begin(9600);

//  MAX30102_setup();
  MAX30205_setup(); 
//  KY_039_setup();
  ADXL_325_setup(); 
//  SD_setup();
  AD8232_setup();
}


// Loop
void loop() {

  if (Serial.available() > 0) {
    int temp = Serial.parseInt();
    if (temp >= 0 && temp < CONTROLLERS_NUMBERS) {
      c = (enum Controller)temp;
      change(c);
      for (int i = 0; i<3; i++) {
        delay(1000);
        Serial.print(3-i);
        Serial.print("s");
      }
    }
  }

  switch (c)
  {
    case MAX30102_: 
//      MAX30102_loop_step();
      break;
    case MAX30205_: 
      MAX30205_loop_step();
      break;
    case KY_039_: 
//      KY_039_loop_step();
      break;
    case ADXL_325_: 
      ADXL_325_loop_step();
      break;
    case SD_: 
//      SD_loop_step();
      break;
    case AD8232_: 
      AD8232_loop_step();
      break;
    case None: 
      break;
    default:
      Serial.print("Error. It needs to see codes.");
  }
}
