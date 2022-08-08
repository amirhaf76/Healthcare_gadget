#include <LiquidCrystal.h>
#include <assert.h>

#include "health_helper.h"
#include "Echocardiogram_module.h"
#include "Thermometer_module.h"
#include "Accelerometer_module.hh"
#include "SD_module.h"
#include "HRB_OX_module.h"
#include "sim_module.h"
#include "time_controlling.hh"
#include "address_and_api.h"

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
#define O2_TIME 15000
#define HRB_TIME 15000

// Planning
#define PLANNING_COUNT 2
#define NORMAL_SAMPELING 2 * 3600 * 1000LU
#define ECHO_SAMPELING 6 * 3600 * 1000LU
// end Planning

// Echo_cardio_module
#define LO_PLUS 2
#define LO_NEG 3

// Buttons
#define LEFT_BUTTON 6
#define SELECT_BUTTON 5
#define RIGHT_BUTTON 4

// SD
#define CS 53
#define MOSI 51
#define MISO 50
#define SLK 52

// LCD
#define D4 23
#define D5 25
#define D6 27
#define D7 29
#define RS 31
#define LCD_EN 33

#define LCD_CONTROLLER_COUNTS 7

enum LCDController
{
  HRB_SCREEN,
  O2_SCREEN,
  HRB_OX_SCREEN,
  TEMPERATURE_SCREEN,
  ECHOCARDIOGRAM_SCREEN,
  STEPS_SCREEN,
  HOME_SCREEN,
  SAMPELLING_TIME_SCREEN,
  SMS_SCREEN,
  // Notice: LCD_CONTROLLER_COUNTS must be checked.
};

static LCDController current_state = HOME_SCREEN;
static Controller c = NONE_MODULE;
static LiquidCrystal lcd(RS, LCD_EN, D4, D5, D6, D7);

unsigned long int duration_times[] = {NORMAL_SAMPELING, ECHO_SAMPELING};
unsigned long int set_up_times[PLANNING_COUNT];
int8_t planning_indexes[PLANNING_COUNT];

HRBandO2Module hrb;

static uint8_t XPIN = A1;
static uint8_t YPIN = A2;
static uint8_t ZPIN = A3;

// ----- Temperature -----
static double temperature;
static double ave_temperature;

// ----- Echocardiography -----
static int echo_results[100];
static uint8_t echo_result_index = 0;

// ----- Files -----
static uint8_t file_index = 0;
static char file_name_temp[21];

// ----- HRB and O2 -----
static bool is_o2_valid = false;
static int o2_measure;
int avg_bpm = 0;

void switch_module(enum Controller c);
void change();
void run_module(enum Controller c);
void run_processes();
void lcd_setup();
void lcd_show_and_change_controller(LCDController lcd_controller);
void controller();
void send_data_to_server();

// Setup
void setup()
{

#if DEBUG
  Serial.begin(9600);
#endif

  assert(c == NONE_MODULE);
  assert(current_state == HOME_SCREEN);

  lcd_setup();

  // Set up controller modules.
  pinMode(PIN_ECHO_CONTROLLER, OUTPUT);
  pinMode(PIN_BODY_TEMP_CONTROLLER, OUTPUT);
  pinMode(PIN_HRB_OX_CONTROLLER, OUTPUT);

  // Making all modules off.
  digitalWrite(PIN_ECHO_CONTROLLER, LOW);
  digitalWrite(PIN_BODY_TEMP_CONTROLLER, LOW);
  digitalWrite(PIN_HRB_OX_CONTROLLER, LOW);

  // Set up button.
  pinMode(SELECT_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);

  lcd_show_and_change_controller(current_state);

#if !FLOW_TESTING
  accelerometer_setup(XPIN, YPIN, ZPIN);
  hrb.setUpModule();
#endif

  set_times(PLANNING_COUNT, set_up_times);
}

// Loop
void loop()
{
  check_times(PLANNING_COUNT, set_up_times, duration_times, planning_indexes);

  controller();

#if !FLOW_TESTING
  accelerometer_loop_step();
#endif
}

void switch_module(enum Controller c)
{
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
  switch (c)
  {
  case HRB_MODULE:
  case O2_MODULE:
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
void change()
{

  for (int i = 3; i > 0; --i)
  {

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

void run_module(enum Controller c)
{
  unsigned long myTime;
  bool echo_status = false;

  uint8_t counter;

  switch (c)
  {

  case HRB_MODULE:
#if !FLOW_TESTING
    hrb.wakeup();

    hrb.heartBeatConfigSetup();

    lcd.clear();
    lcd.print("HRB_OX_MODUL");
    lcd.setCursor(0, 1);
    lcd.print("is runnig");

    while (!is_time_pass(&myTime, HRB_TIME))
    {
      hrb.heartBeatStepLoop();
    }
    avg_bpm = hrb.getAveHeartRadio();

    lcd.clear();
    lcd.print("AVG BPM");
    lcd.setCursor(0, 1);
    lcd.print(avg_bpm);

    hrb.shotDown();

    delay(2000);
#else
    lcd.clear();
    lcd.print("HRB_OX_MODUL");
    lcd.setCursor(0, 1);
    lcd.print("is runnig");
    delay(2000);
#endif

    break;

  case O2_MODULE:
#if !FLOW_TESTING
    hrb.wakeup();
    hrb.spo2ConfigSetUp();

    lcd.clear();
    lcd.print("HRB_OX_MODUL");
    lcd.setCursor(0, 1);
    lcd.print("is runnig");

    hrb.spo2Loop(O2_TIME);

    o2_measure = hrb.getSpo2(&is_o2_valid);

    lcd.clear();
    lcd.print("O2 measure");
    lcd.setCursor(0, 1);
    lcd.print(o2_measure);
    lcd.print("%");

    hrb.shotDown();

    delay(2000);
#else
    lcd.clear();
    lcd.print("HRB_OX_MODUL");
    lcd.setCursor(0, 1);
    lcd.print("is runnig");
    delay(2000);
#endif

    break;

  case TEMPERATURE_MODULE:
#if !FLOW_TESTING
    Thermometer_module_setup();

    counter = 0;
    temperature = 0;
    set_time(&myTime);
    while (!is_time_pass(&myTime, BODY_TEMP_TIME))
    {
      Thermometer_module_loop_step();
      temperature += Thermometer_get_temperature();
      ++counter;
    }
    ave_temperature = temperature / counter;

    lcd.clear();
    lcd.print("TEMPERATURE:");
    lcd.setCursor(0, 1);
    lcd.print(ave_temperature, 2);
    lcd.print("c, ");
    lcd.print(ave_temperature + 10, 2);
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
    lcd.clear();
    lcd.print("ECHO");
    lcd.setCursor(0, 1);
    lcd.print("is runnig");

    echo_result_index = 0;
    file_index = 0;
    file_name_temp[0] = '\0';

    sprintf(file_name_temp, "file_num_%i.csv", file_index++);
    set_time(&myTime);

    while (!is_time_pass(&myTime, ECHO_CARDIO_TIME))
    {
      echo_results[echo_result_index++] = Echocardiogram_module_loop_step();

      if (echo_result_index == 100)
      {
        echo_status = echo_status || create_csv_file(echo_results, echo_result_index, file_name_temp);
        echo_result_index = 0;
        delay(1000);
      }
    }
    if (echo_result_index != 0)
    {
      echo_status = echo_status || create_csv_file(echo_results, echo_result_index, file_name_temp);
      echo_result_index = 0;
    }

    if (echo_status)
    {
      lcd.clear();
      lcd.print("Echo done");
      lcd.setCursor(0, 1);
      lcd.print("correctly");
    }
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

void run_processes()
{
  change();
  switch_module(c);
  run_module(c);
  switch_module(NONE_MODULE);
}

void lcd_setup()
{
  lcd.begin(16, 2);
}

void lcd_show_and_change_controller(LCDController lcd_controller)
{
  switch (lcd_controller)
  {
  case HRB_SCREEN:
    c = HRB_MODULE;
    lcd.clear();
    lcd.print("HRB_OX_SCREEN");
    //---1---2---3---4
    //   HRB and O2
    //<<   Select   >>
    lcd.print("   HRB and O2   ");
    lcd.setCursor(0, 1);
    lcd.print("<<   Select   >>");
    Serial.println("HRB_OX_SCREEN");
    break;

  case O2_SCREEN:
    c = O2_MODULE;
    lcd.clear();
    lcd.print("HRB_OX_SCREEN");
    //---1---2---3---4
    //   HRB and O2
    //<<   Select   >>
    lcd.print("   HRB and O2   ");
    lcd.setCursor(0, 1);
    lcd.print("<<   Select   >>");
    Serial.println("HRB_OX_SCREEN");
    break;
  case TEMPERATURE_SCREEN:
    c = TEMPERATURE_MODULE;
    lcd.clear();
    //---1---2---3---4
    //   Temperature
    //<<   Select   >>
    lcd.print("  Temperature  ");
    lcd.setCursor(0, 1);
    lcd.print("<<   Select   >>");
    Serial.println("TEMPERATURE_SCREEN");
    break;

  case ECHOCARDIOGRAM_SCREEN:
    c = ECHOCARDIOGRAM_MODULE;
    lcd.clear();
    //---1---2---3---4
    //      Echo
    //<<   Select   >>
    lcd.print("      Echo      ");
    lcd.setCursor(0, 1);
    lcd.print("<<   Select   >>");
    Serial.println("ECHOCARDIOGRAM_SCREEN");
    break;
  case STEPS_SCREEN:
    c = ACCELEROMETER_MODULE;
    lcd.clear();
    //---1---2---3---4
    //  Step Counter
    //<<   Select   >>
    lcd.print("  Step Counter  ");
    lcd.setCursor(0, 1);
    lcd.print("<<   ");
    // unsigned long step = accelerometer_get_steps();
    // int8_t step_num_len = 5 - num_len(step);
    // lcd.print(step);
    // for (int8_t i = 0; i < step_num_len; i++)
    // {
    //   lcd.print(" ");
    // }
    lcd.print("   >>");
    Serial.println("steps");
    break;

  case HOME_SCREEN:
    c = NONE_MODULE;
    lcd.clear();
    //---1---2---3---4
    //      Home
    //<<   Choose   >>
    lcd.print("      Home      ");
    lcd.setCursor(0, 1);
    lcd.print("<<   Choose   >>");
    Serial.println("HOME_SCREEN");
    break;
  case SAMPELLING_TIME_SCREEN:
    c = NONE_MODULE;
    lcd.clear();
    //---1---2---3---4
    // Next Sampling:
    //<<   time     >>
    lcd.print("Next Sampling:  ");
    lcd.setCursor(0, 1);
    lcd.print("<<   time     >>");
    Serial.println("SAMPELLING_TIME_SCREEN");
    break;
  case SMS_SCREEN:
    c = NONE_MODULE;
    lcd.clear();
    //---1---2---3---4
    //      SMS
    //<<    Send    >>
    lcd.print("      SMS       ");
    lcd.setCursor(0, 1);
    lcd.print("<<    Send    >>");
    Serial.println("SMS_SCREEN");
    break;
  default:
    lcd.clear();
    lcd.println("Error in");
    lcd.print("lcd show");
    Serial.println("Error in lcd show");
    break;
  }
}

void controller()
{
  // Buttons
  int select_button = digitalRead(SELECT_BUTTON);
  int left_button = digitalRead(LEFT_BUTTON);
  int right_button = digitalRead(RIGHT_BUTTON);

  // Todo: making new function.
  if (right_button == LOW)
  {
    int state = (current_state + 1) % LCD_CONTROLLER_COUNTS;
    current_state = (LCDController)state;

#if DEBUG
    Serial.print("change to right_button: ");
    Serial.println((int)current_state);
#endif

    lcd_show_and_change_controller(current_state);
    delay(1500);
  }
  else if (left_button == LOW)
  {
    int state = (current_state + LCD_CONTROLLER_COUNTS - 1) % LCD_CONTROLLER_COUNTS;
    current_state = (LCDController)state;

#if DEBUG
    Serial.print("change to left_button: ");
    Serial.println((int)current_state);
#endif

    lcd_show_and_change_controller(current_state);
    delay(1500);
  }
  else if (select_button == LOW)
  {
#if DEBUG
    Serial.println("run");
    Serial.println((int)current_state);
#endif

    lcd_show_and_change_controller(current_state);

    run_processes();

    lcd_show_and_change_controller(current_state);
    delay(1500);
  }

  if (current_state == STEPS_SCREEN)
  {
    lcd_show_and_change_controller(current_state);
  }
}

bool prepare_request(char *buffer, char *query_params)
{
  bool is_sim_setup = setup_sim_module();

  if (is_sim_setup)
  {
    char date[DATE_CHARS_SIZ];
    float lat;
    float lon;

    bool is_gotten_gps_data;

    // Getting gps data
    if (gps_setup())
    {
      is_gotten_gps_data = get_gps_data(date, &lat, &lon);
    }

    add_api_key_as_param(buffer);

    if (is_gotten_gps_data)
    {
      add_ampersand(buffer);
      add_params_and_value(query_params, "lat", lat);
      add_ampersand(buffer);
      add_params_and_value(query_params, "long", lon);
      add_ampersand(buffer);
      add_date_time(buffer, date);
    }
  }

  return is_sim_setup;
}

bool send_data_to_server(char *field_name, int value)
{
  char buffer[400];
  char query_params[200] = "";

  if (prepare_request(buffer, query_params))
  {
    add_ampersand(query_params);
    add_params_and_value(query_params, field_name, value);

    create_request(buffer, get_request_line, UPDATE_GET_API, query_params, THING_SPEAK_HOST);

    send_data(buffer);

    return true;
  }

  return false;
}
/* todo:
  control
  GPS,
  sending data,
  Database,
  saving data for no connection,
  test,
*/