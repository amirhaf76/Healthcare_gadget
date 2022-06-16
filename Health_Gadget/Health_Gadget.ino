/** 
 * MAX30205 as body temperature:
 * Module 	| Arduino Board
 * vcc 		  | 5v or 3.3 v
 * GND 		  | ground
 * SDA		  | 20 or SDA
 * SCL		  | 21 or SCL
 * 
 * MAX30102 as Oxi meter:
 * Module 	| Arduino Board
 * vcc 	  	| 3.3 v
 * GND   		| ground
 * SDA		  | SDA or 20
 * SCL		  | SCL or 21
 * 
 * ADXL325 as Accelerator Meter:
 * Module 	| Arduino Board
 * vcc 		  | 3.3 v
 * GND 		  | ground
 * x	  	  | A1
 * y		    | A2
 * z	    	| A3
 * 
 * AD8232 as Echocardiography:
 * Module 	| Arduino Board
 * vcc 		  | 3.3 v
 * GND 		  | ground
 * output	  | A0
 * LO+		  | 2
 * LO-		  | 3
 * SDN		  | from controller pin 22
 * 
 * SD Module:
 * Module 	| Arduino Board
 * vcc 		  | 3.3 v
 * GND 	  	| ground
 * CS		    | 53
 * MOSI	  	| MOSI = 51
 * MISO	  	| MISO = 50
 * SCK	  	| 52
 * 
 * Liquid Cristal:
 * Name     | Pin
 * VSS      | Ground
 * VCC      | 5V
 * RS       | 31
 * Enable   | 33
 * R/W      | Ground
 * V0       | Potentiometer
 * D4       | 23
 * D5       | 25
 * D6       | 27
 * D7       | 29
 * LED+     | 5V
 * LED-     | Ground
 * 
 * Transistor Controller:
 * ...
 * 
*/

#include <LiquidCrystal.h>


#include "health_helper.h"
#include "Echocardiogram_module.h"
#include "Thermometer_module.h"
#include "Accelerometer_module.h"
#include "SD_module.h"
#include "HRB_OX_module.h"

/* Gadget ID */
#define ID_GADGET 0xf2


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

// Buttons
#define SELECT_BUTTON 6
#define LEFT_BUTTON 5
#define RIGHT_BUTTON 4

// SD 
#define CS 10
#define MOSI 11
#define MISO 12
#define SLK	13

// LCD 
#define D4 23
#define D5 25
#define D6 27
#define D7 29
#define RS 31
#define LCD_EN 33

int XPIN = A1;
int YPIN = A2;
int ZPIN = A3;


LiquidCrystal lcd(RS, LCD_EN, D4, D5, D6, D7);

enum Controller c = None;



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

    //default:
      
  }

}

// Setup
void setup() {

	Serial.begin(9600);
  lcd_setup();
  Accelerometer_setup(XPIN, YPIN, ZPIN);

	pinMode(SELECT_BUTTON, INPUT);
	pinMode(LEFT_BUTTON, INPUT);
	pinMode(RIGHT_BUTTON, INPUT);

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

void lcd_setup() {
    lcd.begin(16, 2);
}

void lcd_controller(int a) {
}
/* todo: 
	control
	GPS,
	sending data, 
	Database, 
	saving data for no connection, 
	test, 
*/