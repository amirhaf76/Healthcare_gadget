#include <Wire.h>
#include "Protocentral_MAX30205.h"
#include "health_helper.h"

/* Gadget ID*/
#define ID_GADGET 0xf2

// It is for debugging program.
#define DEBUG 1 

// Setup Functions 
//void MAX30102_setup(); 
void MAX30205_setup(); // ok
void KY_039_setup(); // ok
void ADXL_325_setup(); // ok
//void SD_setup();
void AD8232_setup(); // ok

// Loop Step Functions
//void MAX30102_loop_step();
void MAX30205_loop_step(); // ok
void KY_039_loop_step(); // ok
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
// KY-039
const int NUM_POINTs = 50;
double data[NUM_POINTs];
int counter;
double mean;
int beat;
int brt;
unsigned long temp_time;
int KY_INPUT = A0;


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

void KY_039_setup() {
  counter = 0;
  beat = 0;
  brt = 0;
  temp_time = millis();
  mean = analogRead(A0);
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

  delay(100); // ?. it should be checked.
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

  delay(1)
}

void KY_039_loop_step() {
  double pulse;
  int sum = 0;
  
  for (int i = 0; i < 20; ++i){
    sum += analogRead(KY_INPUT);
  }
  
  pulse = sum/ 20.00;

  data[counter++] = pulse;
  Serial.print("Pulse:");
  Serial.print(pulse);
  Serial.print("\t");

  if (counter == NUM_POINTs) {
    mean = calculate_mean(NUM_POINTs, data, nullptr);
    beat = find_local_max(NUM_POINTs, data, mean);
    
    int t = (millis() - temp_time)/1000;
    brt = (beat * 60) / t;
    temp_time = millis();
    
    Serial.print("mean:");
    Serial.print(mean);
    Serial.print("\t");
    
    Serial.print("beat:");
    Serial.print(beat);
    Serial.print("\t");
    
    Serial.print("brt:");
    Serial.print(brt);
    Serial.print("\t");
    
    Serial.print("time:");
    Serial.print(t);
    Serial.print("\t");
    Serial.print("****************");
  } 
    
  Serial.println();

  counter %= NUM_POINTs;
  
  delay(100);// wait for a second
}

// change 
void change(enum Controller c) {
  
}
// Setup
void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT); //? ky-039

//  MAX30102_setup();
  MAX30205_setup(); 
  KY_039_setup();
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
    }
  }

  switch (c)
  {
    case MAX30102_: 
      #if DEBUG
        Serial.print("MAX30102_");
      #else
//      MAX30102_loop_step();
      #endif
      break;
    case MAX30205_:
      #if DEBUG
      Serial.print("MAX30205_");
      #else
      MAX30205_loop_step();
      #endif 
      break;
    case KY_039_: 
      #if DEBUG
      Serial.print("KY_039_");
      #else
      KY_039_loop_step();
      #endif
      break;
    case ADXL_325_: 
        #if DEBUG
      Serial.print("ADXL_325_");
      #else
      ADXL_325_loop_step();
      #endif
      break;
    case SD_: 
      #if DEBUG
      Serial.print("SD_");
      #else
//      SD_loop_step();
      #endif
      break;
    case AD8232_: 
      #if DEBUG
      Serial.print("AD8232_");
      #else
      AD8232_loop_step();
      #endif
      break;
    case None: 
      #if DEBUG
      Serial.print("None");
      #else
      #endif
      break;
    default:
      Serial.print("Error. It needs to see codes.");
  }
}
