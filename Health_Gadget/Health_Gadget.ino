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
// #include "Protocentral_MAX30205.h"
#include "health_helper.h"
#include "Arduino.h"
#include "Thermometer_module.h"
#include "Echocardiogram_module.h"
#include "Accelerometer_module.h"
// #include <SPI.h>
// #include <SD.h>
// #include "MAX30105.h"
// #include "heartRate.h"
// #include "spo2_algorithm.h"


/* Gadget ID*/
#define ID_GADGET 0xf2
#define FILE_RECORDS "records"

// It is for debugging program.
#define DEBUG 0
#define SERIAL_LOG 0
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

/* Declaration Part */
void logger(char *);

void logger(char * message) {
  #if SERIAL_LOG
    Serial.println(message);
  #else

  #endif
}

/* Stepup Declarations */



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
      Echocardiogram_module_setup(LO_PLUS, LO_NEG);

      int storageRes[100];
      int index = 0;

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

  Accelerometer_Meter_setup(xpin, ypin, zpin);

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
  Serial.print(Accelerometer_get_steps());
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
