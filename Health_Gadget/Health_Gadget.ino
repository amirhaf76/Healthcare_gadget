/** 
 * MAX30205 as body temperature:
 * Module 	| Arduino Board
 * vcc 		  | 5v or 3.3 v
 * GND 		  | ground
 * SDA		  | 20 or SDA
 * SCL		  | 21 or SCL
 * 
 * Module controller: pin 24
 * 
 * MAX30102 as Oxi meter:
 * Module 	| Arduino Board
 * vcc 	  	| 3.3 v
 * GND   		| ground
 * SDA		  | SDA or 20
 * SCL		  | SCL or 21
 * 
 * Module controller: pin 26
 * 
 * ADXL33 5 as Accelerator Meter:
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
 * LED+     | 5V through a 220 ohm resistor
 * LED-     | Ground
 * 
 * Buttons:
 * Name     | Pin
 * LEFT     | 6
 * SELECT   | 5
 * RIGHT    | 4
 * 
 * 
 * Transistor Controller:
 * ...
 * 
*/

#include <LiquidCrystal.h>
#include <assert.h>

#include "health_helper.h"
#include "Echocardiogram_module.h"
#include "Thermometer_module.h"
#include "Accelerometer_module.h"
#include "SD_module.h"
#include "HRB_OX_module.h"
#include "SimModule.h"

/* Gadget GUID */
#define ID_GADGET "fc309105-e43e-40de-9356-d6ba875c8fd2"


// It is for debugging program.
#define DEBUG 1
#define SERIAL_STATUS 1
#define FLOW_TESTING 0

/* Global Variables */
#define PIN_ECHO_CONTROLLER 22
#define PIN_BODY_TEMP_CONTROLLER 24
#define PIN_HRB_OX_CONTROLLER 26

#define ECHO_CARDIO_TIME 5000
#define BODY_TEMP_TIME 5000
#define HRB_OX_TIME 5000


// Echo_cardio_module
#define LO_PLUS 2
#define LO_NEG  3

// Buttons
#define LEFT_BUTTON 6
#define SELECT_BUTTON 5
#define RIGHT_BUTTON 4

// SD 
#define CS 53
#define MOSI 51
#define MISO 50
#define SLK	52

// LCD 
#define D4 23
#define D5 25
#define D6 27
#define D7 29
#define RS 31
#define LCD_EN 33

#define LCD_CONTROLLER_COUNTS 5

enum LCDController {
  HRB_OX_SCREEN,
  TEMPERATURE_SCREEN, 
  ECHOCARDIOGRAM_SCREEN,
  STEPS,
  EMPTY,
};

LCDController current_state = EMPTY;
Controller c = NONE_MODULE;
LiquidCrystal lcd(RS, LCD_EN, D4, D5, D6, D7);

int XPIN = A1;
int YPIN = A2;
int ZPIN = A3;

uint8_t counter;
double t;
int storageRes[100];
int index = 0;


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
      lcd.clear();
      lcd.print("HRB_OX");
      lcd.setCursor(0, 1);
      lcd.print("is on");
      #endif

      break;

    case TEMPERATURE_MODULE:
      digitalWrite(PIN_BODY_TEMP_CONTROLLER, HIGH);

      #if DEBUG
      Serial.println("TEMPERATURE_MODULE On");
      lcd.clear();
      lcd.print("TEMPERATURE");
      lcd.setCursor(0, 1);
      lcd.print("is on");
      #endif

      break;

    case ECHOCARDIOGRAM_MODULE:
      digitalWrite(PIN_ECHO_CONTROLLER, HIGH);

      #if DEBUG
      Serial.println("ECHOCARDIOGRAM_MODULE On");
      lcd.clear();
      lcd.print("ECHOCARDIOGRAM");
      lcd.setCursor(0, 1);
      lcd.print("is on");
      #endif  

      break;

    case NONE_MODULE:

      #if DEBUG
      Serial.println("None On");
      lcd.clear();
      lcd.print("Nothing ");
      lcd.print("is on");
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
void change() {
  
  for (int i = 3; i > 0; --i) {
  #if DEBUG
  Serial.print(i);
  Serial.println("s to change.");
  #endif
  lcd.clear();
  lcd.print("Swich in ");
  lcd.print(i);
  lcd.print("s");

  delay(1000);
  }
  lcd.clear();
  lcd.print("Preparing ...");

}

void run_module(enum Controller c) {
  unsigned long myTime = millis();
  
  switch(c) {
    case HRB_OX_MODULE:
      #if !FLOW_TESTING
      HRB_OX_module_setup(); 

      while (millis() - myTime < HRB_OX_TIME)
      {
        HRB_OX_module_loop_step();
      }

      HRB_OX_module_down();

      lcd.clear();
      lcd.print("HRB_OX_MODULE");
      lcd.setCursor(0, 1);
      lcd.print("is runnig");
      delay(2000);
      #else
      lcd.clear();
      lcd.print("HRB_OX_MODULE");
      lcd.setCursor(0, 1);
      lcd.print("is runnig");
      delay(2000);
      #endif

      break;

    case TEMPERATURE_MODULE:
      #if !FLOW_TESTING
      Thermometer_module_setup();

      counter = 0;
      t = 0;
      
      while (millis() - myTime < BODY_TEMP_TIME)
      {
        Thermometer_module_loop_step();
        t += Thermometer_get_temperature();
        ++counter;
      }
      lcd.clear();
      lcd.print("TEMPERATURE:");
      lcd.setCursor(0, 1);
      lcd.print((t/counter)+5, 2);
      lcd.print("c");
      delay(5000);
      #else
      lcd.clear();
      lcd.print("TEMPERATURE");
      lcd.setCursor(0, 1);
      lcd.print("is runnig");
      delay(2000);
      #endif

      break;

    case ECHOCARDIOGRAM_MODULE:
      #if !FLOW_TESTING
      Echocardiogram_module_setup(LO_PLUS, LO_NEG);

      index = 0;

      while (millis() - myTime < ECHO_CARDIO_TIME)
      {
        storageRes[index++] = Echocardiogram_module_loop_step();

        if ( index == 100) {
          create_csv_file(storageRes, index);
          index = 0;
        }
      }

      create_csv_file(storageRes, index);

      lcd.clear();
      lcd.print("EXHOCARDIOGRAM");
      lcd.setCursor(0, 1);
      lcd.print("is runnig");
      delay(2000);
      #else
      lcd.clear();
      lcd.print("EXHOCARDIOGRAM");
      lcd.setCursor(0, 1);
      lcd.print("is runnig");
      delay(2000);
      #endif

      break;
    case NONE_MODULE:
      lcd.clear();
      lcd.print("NONE_MODULE.");

    default:
    break;
      
  }

}

// Setup
void setup() {

  #if DEBUG
	Serial.begin(9600);
  #endif

  lcd_setup();

  #if !FLOW_TESTING
  accelerometer_setup(XPIN, YPIN, ZPIN);
  #endif

  assert(c == NONE_MODULE);
  assert(current_state == EMPTY);
  
  
  // Set up controller modules.
  pinMode(PIN_ECHO_CONTROLLER, OUTPUT);
  pinMode(PIN_BODY_TEMP_CONTROLLER, OUTPUT);
  pinMode(PIN_HRB_OX_CONTROLLER, OUTPUT);
  
  // All module are off.
  digitalWrite(PIN_ECHO_CONTROLLER, LOW);
  digitalWrite(PIN_BODY_TEMP_CONTROLLER, LOW);
  digitalWrite(PIN_HRB_OX_CONTROLLER, LOW);

  // Set up button.
	pinMode(SELECT_BUTTON, INPUT_PULLUP);
	pinMode(LEFT_BUTTON, INPUT_PULLUP);
	pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  lcd_show(current_state);
}

// Loop
void loop() {
  
  // Buttons
  int select_button = digitalRead(SELECT_BUTTON);
  int left_button = digitalRead(LEFT_BUTTON);
  int right_button = digitalRead(RIGHT_BUTTON);

  if (right_button == LOW) {
    int state = (current_state + 1) % LCD_CONTROLLER_COUNTS;
    current_state = (LCDController)state;
    Serial.println("change right_button");
    Serial.println((int)current_state);
    lcd_show(current_state);
    delay(1500);
  } 
  else if (left_button == LOW) {
    int state = (current_state + LCD_CONTROLLER_COUNTS - 1) % LCD_CONTROLLER_COUNTS;
    current_state = (LCDController)state;
    Serial.println("change left_button");
    Serial.println((int)current_state);
    lcd_show(current_state);
    delay(1500);
  }
  else if (select_button == LOW) {
    Serial.println("run");
    Serial.println((int)current_state);
    lcd_show(current_state);
    change();
    switch_module(c);
    run_module(c);
    switch_module(NONE_MODULE);
    lcd_show(current_state);
    delay(1500);
  }

  if (current_state == STEPS) {
    lcd_show(current_state);
  }


  #if !FLOW_TESTING
  accelerometer_loop_step();
  #endif
  
}

void lcd_setup() {
  lcd.begin(16, 2);
}

void lcd_show(LCDController lcd_controller) {
  switch (lcd_controller)
  {
  case HRB_OX_SCREEN: 
    c = HRB_OX_MODULE;
    lcd.clear();
    lcd.print("HRB_OX_SCREEN");
    Serial.println("HRB_OX_SCREEN");
    break;

  case TEMPERATURE_SCREEN: 
    c = TEMPERATURE_MODULE;
    lcd.clear();
    lcd.print("TEMPERATURE_SCREEN");
    Serial.println("TEMPERATURE_SCREEN");
    break;

  case ECHOCARDIOGRAM_SCREEN: 
    c = ECHOCARDIOGRAM_MODULE;
    lcd.clear();
    lcd.print("ECHOCARDIOGRAM_SCREEN");
    Serial.println("ECHOCARDIOGRAM_SCREEN");
    break;
  case STEPS: 
    c = ACCELEROMETER_MODULE;
    lcd.clear();
    lcd.print("steps");
    lcd.setCursor(0, 1);
    lcd.print(accelerometer_get_steps());
    Serial.println("steps");
    break;

  case EMPTY: 
    c = NONE_MODULE;
    lcd.clear();
    lcd.print("EMPTY");
    Serial.println("EMPTY");
    break;

  default:
    lcd.clear();
    lcd.println("Error in");
    lcd.print("lcd show");
    Serial.println("Error in lcd show");
    break;
  }
}



/* todo: 
	control
	GPS,
	sending data, 
	Database, 
	saving data for no connection, 
	test, 
*/