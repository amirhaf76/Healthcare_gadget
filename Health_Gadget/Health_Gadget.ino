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
 * CS		| 13
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
#include "MAX30105.h"
#include "health_helper.h"
#include <SPI.h>
#include <SD.h>


/* Gadget ID*/
#define ID_GADGET 0xf2
#define FILE_RECORDS "records"

// It is for debugging program.
#define DEBUG 1 


/* Declaration Part */

// Setup Functions 
void MAX30102_setup(); // ok
void MAX30205_setup(); // ok
void KY_039_setup(); // ok
void ADXL_325_setup(); // ok
//void SD_setup();
void AD8232_setup(); // ok

// Loop Step Functions
void MAX30102_loop_step();  // ok
void MAX30205_loop_step(); // ok
void KY_039_loop_step(); // ok
void ADXL_325_loop_step(); // ok
//void SD_loop_step();
void AD8232_loop_step(); // ok


/* Global Variables */

// MAX30102
MAX30105 particleSensor; // initialize MAX30102 with I2C
// MAX30205
MAX30205 tempSensor;
enum Controller c = None;
// KY-039
const int NUM_POINTs = 50;
double data[NUM_POINTs];
int counter;
double mean;
int beat;
int brt;
unsigned long temp_time;
int KY_INPUT = A0;
// ADXL_325
Point p;
const int xpin = A0;                  // x-axis of the accelerometer
const int ypin = A1;                  // y-axis
const int zpin = A2;                  // z-axis (only on 3-axis models)
const int CF = 1;
// AD8232
const int LO_PLUS = 10;
const int LO_NEG = 11;


/* Stepup Declarations */
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

void MAX30102_setup() {
  // Serial.begin(115200);
  while(!Serial); //We must wait for Teensy to come online
  delay(100);
  Serial.println("");
  Serial.println("MAX30102");
  Serial.println("");
  delay(100);
  // Initialize sensor
  if (particleSensor.begin(Wire, I2C_SPEED_FAST) == false) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }

  byte ledBrightness = 70; //Options: 0=Off to 255=50mA
  byte sampleAverage = 1; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  int sampleRate = 400; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 69; //Options: 69, 118, 215, 411
  int adcRange = 16384; //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
}


/* Loop Step Declarations */
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

  delay(1);
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

void MAX30102_loop_step() {
  particleSensor.check(); //Check the sensor
  while (particleSensor.available()) {
      // read stored IR
      Serial.print(particleSensor.getFIFOIR());
      Serial.print(",");
      // read stored red
      Serial.println(particleSensor.getFIFORed());
      // read next set of samples
      particleSensor.nextSample();      
  }
}


// change 
void change(enum Controller c) {
  
}
// Setup
void setup() {
	Serial.begin(9600);
	// pinMode(A0, INPUT); //? ky-039

	#if !DEBUG
	MAX30102_setup();
	MAX30205_setup(); 
	KY_039_setup();
	ADXL_325_setup(); //
	//  SD_setup();
	AD8232_setup();
	#endif

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
        MAX30102_loop_step();
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

bool make_file_ready() {
  #if DEBUG
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
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
    Serial.println("error opening test.txt");
    #endif
  }
}

void read_from_file(int len, char buff[]) {
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
