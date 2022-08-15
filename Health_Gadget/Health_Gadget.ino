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
#define TESTING_SCREEN 1
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
#define NORMAL_SAMPELING 2 * 1 * 1000LU
#define ECHO_SAMPELING 3 * 1 * 1000LU
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

#if TESTING_SCREEN
#define LCD_CONTROLLER_COUNTS 12
#else
#define LCD_CONTROLLER_COUNTS 8
#endif

enum LCDController
{
  /** Notice: !!!!!
   *  LCD_CONTROLLER_COUNTS must be checked.
   */
  HRB_SCREEN,
  O2_SCREEN,
  TEMPERATURE_SCREEN,
  ECHOCARDIOGRAM_SCREEN,
  STEPS_SCREEN,
  HOME_SCREEN,
  SAMPELLING_TIME_SCREEN,
  SMS_SCREEN,
#if TESTING_SCREEN
  SMS_SCREEN_TESTING,
  GPS_SCREEN_TESTING,
  CALLING_API_SCREEN_TESTING,
  SD_SCREEN_TESTING,
#endif
};

#if TESTING_SCREEN
#define CONTROLLERS_NUMBERS 12
#else
#define CONTROLLERS_NUMBERS 8
#endif

enum Controller
{
  /**
   * Number of controllers should be
   * matched with Macro CONTROLLERS_NUMBERS.
   */
  HRB_MODULE,
  O2_MODULE,
  TEMPERATURE_MODULE,
  ACCELEROMETER_MODULE,
  ECHOCARDIOGRAM_MODULE,
  NONE_MODULE,
  SAMPELLING_TIME,
  SMS_SENDING,
#if TESTING_SCREEN
  SMS_TESTING,
  GPS_TESTING,
  CALLING_API_TESTING,
  SD_TESTING,
#endif
};

static LCDController current_state = HOME_SCREEN;
static Controller current_controller = NONE_MODULE;
static LiquidCrystal lcd(RS, LCD_EN, D4, D5, D6, D7);

enum Plan
{
  NORMAL_SAMPELING_PLAN,
  ECHO_SAMPELING_PLAN
};

Plan planIds[PLANNING_COUNT] = {NORMAL_SAMPELING_PLAN, ECHO_SAMPELING_PLAN};
unsigned long int duration_times[PLANNING_COUNT] = {NORMAL_SAMPELING, ECHO_SAMPELING};
unsigned long int set_up_times[PLANNING_COUNT];
int8_t planning_indexes[PLANNING_COUNT] = {-1, -1};

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
void run_pip(enum Controller c);
void lcd_setup();
void lcd_show_and_change_controller(LCDController lcd_controller);
void controller();
void send_data_to_server();
void run_hrb_module();
void run_o2_module();
void run_temperature_module();
void run_echocardiogram_module();
void test_sms();
void test_gps();
void test_api();
void test_sd();

// Setup
void setup()
{

#if DEBUG
  Serial.begin(9600);
#endif

  assert(current_controller == NONE_MODULE);
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

#endif

  set_times(PLANNING_COUNT, set_up_times);
}

// Loop
void loop()
{
  check_times(PLANNING_COUNT, set_up_times, duration_times, planning_indexes);

  for (int i = 0; i < PLANNING_COUNT; i++)
  {
    if (planning_indexes[i] == 0)
    {
      switch (planIds[i])
      {
      case NORMAL_SAMPELING_PLAN:
        // run_pip(HRB_MODULE);

        // run_pip(O2_MODULE);

        // run_pip(TEMPERATURE_MODULE);

        // send_data_to_server("hrb", avg_bpm);

        break;
      case ECHO_SAMPELING_PLAN:
        // run_pip(ECHOCARDIOGRAM_MODULE);

        break;
      }
#if 1
      Serial.print("[");
      Serial.print(i);
      Serial.print("] planning_indexes: ");
      Serial.print(planning_indexes[i]);
      Serial.print(", planIds: ");
      Serial.println(planIds[i]);
#endif
    }
  }

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
    lcd.clear();
    lcd.print("Switch None");
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
    run_hrb_module();
    break;

  case O2_MODULE:
    run_o2_module();
    break;

  case TEMPERATURE_MODULE:
    run_temperature_module();
    break;

  case ECHOCARDIOGRAM_MODULE:
    run_echocardiogram_module();
    break;

#if TESTING_SCREEN
  case SMS_TESTING:
    test_sms();
    break;

  case GPS_TESTING:
    test_gps();
    break;

  case CALLING_API_TESTING:
    test_api();
    break;

  case SD_TESTING:
    test_sd();
    break;

#endif

  case NONE_MODULE:
    lcd.clear();
    lcd.print("NONE_MODULE.");
  default:
    break;
  }
}

void run_pip(enum Controller c)
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
    current_controller = HRB_MODULE;
    lcd.clear();
    //---1---2---3---4
    //      HRB
    //<<   Select   >>
    lcd.print("      HRB       ");
    lcd.setCursor(0, 1);
    lcd.print("<<   Select   >>");
    Serial.println("HRB_OX_SCREEN");
    break;

  case O2_SCREEN:
    current_controller = O2_MODULE;
    lcd.clear();
    //---1---2---3---4
    //       O2
    //<<   Select   >>
    lcd.print("       O2       ");
    lcd.setCursor(0, 1);
    lcd.print("<<   Select   >>");
    Serial.println("HRB_OX_SCREEN");
    break;
  case TEMPERATURE_SCREEN:
    current_controller = TEMPERATURE_MODULE;
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
    current_controller = ECHOCARDIOGRAM_MODULE;
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
    current_controller = ACCELEROMETER_MODULE;
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
    current_controller = NONE_MODULE;
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
    current_controller = NONE_MODULE;
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
    current_controller = NONE_MODULE;
    lcd.clear();
    //---1---2---3---4
    //      SMS
    //<<    Send    >>
    lcd.print("      SMS       ");
    lcd.setCursor(0, 1);
    lcd.print("<<    Send    >>");
    Serial.println("SMS_SCREEN");
    break;

#if TESTING_SCREEN
  case SMS_SCREEN_TESTING:
    current_controller = SMS_TESTING;
    lcd.clear();
    //---1---2---3---4
    //  SMS Testing
    //<<    Test    >>
    lcd.print("  SMS Testing   ");
    lcd.setCursor(0, 1);
    lcd.print("<<    Test    >>");
    Serial.println("SMS_SCREEN_TESTING");
    break;
  case GPS_SCREEN_TESTING:
    current_controller = GPS_TESTING;
    lcd.clear();
    //---1---2---3---4
    //  GPS Testing
    //<<    Test    >>
    lcd.print("  GPS Testing   ");
    lcd.setCursor(0, 1);
    lcd.print("<<    Test    >>");
    Serial.println("GPS_SCREEN_TESTING");
    break;
  case CALLING_API_SCREEN_TESTING:
    current_controller = CALLING_API_TESTING;
    lcd.clear();
    //---1---2---3---4
    //   API Testing
    //<<    Test    >>
    lcd.print("   API Testing  ");
    lcd.setCursor(0, 1);
    lcd.print("<<    Test    >>");
    Serial.println("CALLING_API_SCREEN_TESTING");
    break;
  case SD_SCREEN_TESTING:
    current_controller = SD_TESTING;
    lcd.clear();
    //---1---2---3---4
    //    SD Tesing
    //<<    Test    >>
    lcd.print("    SD Tesing   ");
    lcd.setCursor(0, 1);
    lcd.print("<<    Test    >>");
    Serial.println("SD_SCREEN_TESTING");
    break;
#endif

  default:
    lcd.clear();
    lcd.print("Error in");
    lcd.setCursor(0, 1);
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

    run_pip(current_controller);

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

bool send_data_to_server(char *field_name, char *value)
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

bool send_data_to_server(char *field_name, float value)
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

void run_hrb_module()
{
  unsigned long int myTime;

#if !FLOW_TESTING
  hrb.setUpModule();

  hrb.wakeup();

  hrb.heartBeatConfigSetup();

  lcd.clear();
  lcd.print("HRB_OX_MODUL");
  lcd.setCursor(0, 1);
  lcd.print("is runnig");
  set_time(&myTime);
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
}

void run_o2_module()
{
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
}

void run_temperature_module()
{
  int8_t counter = 0;
  unsigned long int myTime;

#if !FLOW_TESTING
  Thermometer_module_setup();

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
}

void run_echocardiogram_module()
{
  unsigned long myTime;
  bool echo_status = false;

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
}

void test_sms()
{
  setup_sim_module();

  bool res = send_sms("09121067991", "Testing sms.");
  Serial.print(res);
  lcd.clear();
  //---1---2---3---4
  lcd.print("  09121067991   ");
  lcd.setCursor(0, 1);
  lcd.print(res==true ? "   Successful   " : "  Unsuccessful  ");
  Serial.println("test_sms");
}

void test_gps()
{
  setup_sim_module();

  gps_setup();

  char date[20];
  float lan;
  float lon;

  bool res = get_gps_data(date, &lan, &lon);

  //---1---2---3---4
  lcd.clear();
  lcd.print(date);
  delay(3000);
  lcd.clear();
  lcd.print(lan);
  lcd.setCursor(0, 1);
  lcd.print(lon);
  Serial.println("test_gps");
}

void test_api()
{
  setup_sim_module();

  bool res = send_data_to_server("f", 1);

  lcd.clear();
           //---1---2---3---4
  lcd.print("  API Testing  ");
  lcd.setCursor(0, 1);
  lcd.print(res ? "   Successful   " : "  Unsuccessful  ");
  Serial.println("test_api");
}

void test_sd()
{
  bool res = make_file_ready();
  
  lcd.clear();
           //---1---2---3---4
  lcd.print("   SD Testing  ");
  lcd.setCursor(0, 1);
  lcd.print(res ? "   Successful   " : "  Unsuccessful  ");
  Serial.println("test_api");
}

/* todo:
  control
  GPS,
  sending data,
  Database,
  saving data for no connection,
  test,
*/