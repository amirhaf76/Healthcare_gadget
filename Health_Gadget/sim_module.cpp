#include <DFRobot_SIM808.h>
#include <SoftwareSerial.h>
#include "time_controlling.hh"
#include "address_and_api.h"

#define SIM_DEBUG 1

#define PIN_TX 10
#define PIN_RX 11
#define SIM808_BUFFER_SIZE 1024

static SoftwareSerial mySerial(PIN_TX, PIN_RX);
static DFRobot_SIM808 sim808(&mySerial); // Connect RX,TX,PWR,

static char buffer[SIM808_BUFFER_SIZE];

bool setup_sim_module()
{
  mySerial.begin(9600);
  Serial.begin(9600);

#if SIM_DEBUG
  Serial.println("start sim808");
#endif

  uint8_t counter = 0;

  while (!sim808.init())
  {
#if SIM_DEBUG
    Serial.print("Sim808 init error\r\n");
#endif

    if ((counter++) == 5)
      return false;

    delay(1000);
  }

#if SIM_DEBUG
  Serial.println("sim808 initiated successfuly");
#endif

  return true;
}

bool send_sms(char *phone_number, char *message)
{
  bool res = sim808.sendSMS(phone_number, message);

#if SIM_DEBUG
  Serial.print("phone_number: ");
  Serial.print(phone_number);
  Serial.print(", message: ");
  Serial.println(message);
  Serial.println((res) ? "Message sent." : "Message failed.");
#endif

  return res;
}

bool gps_setup()
{
  //************* Turn on the GPS power************
  bool res = sim808.attachGPS();

#if SIM_DEBUG
  if (res)
    Serial.println("Open the GPS power success");
  else
    Serial.println("Open the GPS power failure");
#endif

  return res;
}

bool get_gps_data(char *date, float *lat, float *lon)
{
  //************** Get GPS data *******************
  unsigned long int time;

  set_time(&time);
  while (!is_time_pass(&time, 10000))
  {
    if (sim808.getGPS())
    {
#if SIM_DEBUG
      Serial.print(sim808.GPSdata.year);
      Serial.print("/");
      Serial.print(sim808.GPSdata.month);
      Serial.print("/");
      Serial.print(sim808.GPSdata.day);
      Serial.print(" ");
      Serial.print(sim808.GPSdata.hour);
      Serial.print(":");
      Serial.print(sim808.GPSdata.minute);
      Serial.print(":");
      Serial.print(sim808.GPSdata.second);
      Serial.print(":");
      Serial.println(sim808.GPSdata.centisecond);

      Serial.print("latitude :");
      Serial.println(sim808.GPSdata.lat, 6);

      sim808.latitudeConverToDMS();
      Serial.print("latitude :");
      Serial.print(sim808.latDMS.degrees);
      Serial.print("^");
      Serial.print(sim808.latDMS.minutes);
      Serial.print("\'");
      Serial.print(sim808.latDMS.seconeds, 6);
      Serial.println("\"");
      Serial.print("longitude :");
      Serial.println(sim808.GPSdata.lon, 6);
      sim808.LongitudeConverToDMS();
      Serial.print("longitude :");
      Serial.print(sim808.longDMS.degrees);
      Serial.print("^");
      Serial.print(sim808.longDMS.minutes);
      Serial.print("\'");
      Serial.print(sim808.longDMS.seconeds, 6);
      Serial.println("\"");

      Serial.print("speed_kph :");
      Serial.println(sim808.GPSdata.speed_kph);
      Serial.print("heading :");
      Serial.println(sim808.GPSdata.heading);
#endif

      sprintf(date, "%4d-%d-%dT%d:%d", // 12 + 4= 16
              sim808.GPSdata.year,
              sim808.GPSdata.month,
              sim808.GPSdata.day,
              sim808.GPSdata.hour,
              sim808.GPSdata.minute,
              sim808.GPSdata.second);

      *lat = sim808.GPSdata.lat;
      *lon = sim808.GPSdata.lon;

      //************* Turn off the GPS power ************
      sim808.detachGPS();

      return true;
    }
  }

  //************* Turn off the GPS power ************
  sim808.detachGPS();

  sprintf(date, "");

  *lat = -1;
  *lon = -1;

  return false;
}

int has_signal()
{
  int power;
  sim808.getSignalStrength(&power);

#if SIM_DEBUG
  Serial.print("Signal Strength: ");
  Serial.println(power);
#endif

  return power;
}

bool send_data()
{
}

bool send_data(char *http_cmd)
{
  delay(4000);

#if SIM_DEBUG
  Serial.println("sending data start");
#endif

  int8_t counter = 0;
  bool status;

  //*********** Attempt DHCP *******************
  while (!sim808.join(F("cmnet")))
  {
#if SIM_DEBUG
    Serial.println("Sim808 join network error");
#endif

    if (counter++ == 5)
      return false;

    delay(2000);
  }

#if SIM_DEBUG
  Serial.println("Sim808 join successfully");
#endif

  delay(2000);

//************ Successful DHCP ****************
#if SIM_DEBUG
  Serial.print("IP Address is ");
  Serial.println(sim808.getIPAddress());
  Serial.println("connecting");
#endif
  delay(2000);

  //*********** Establish a TCP connection ************
  status = sim808.connect(TCP, THING_SPEAK_HOST, 80);
  if (status)
  {
#if SIM_DEBUG
    Serial.println("Connect success");
#endif
  }
  else
  {
#if SIM_DEBUG
    Serial.println("Connect error");
#endif
    sim808.disconnect();
    return false;
  }

//*********** Send a GET request *****************
#if SIM_DEBUG
  Serial.println("waiting to fetch...");
#endif

#if SIM_DEBUG
  Serial.println(http_cmd);
#endif

  sim808.send(http_cmd, sizeof(http_cmd) - 1);

  counter = 0;
  while (counter++ < 5)
  {
    int ret = sim808.recv(buffer, SIM808_BUFFER_SIZE - 1);

    if (ret <= 0)
    {

#if SIM_DEBUG
      Serial.println("fetch over...");
#endif

      break;
    }
    buffer[ret] = '\0';

#if SIM_DEBUG
    Serial.print("Recv: ");
    Serial.print(ret);
    Serial.print(" bytes: ");
    Serial.println(buffer);
#endif

    break;
  }

  //************* Close TCP or UDP connections **********
  sim808.close();

  //*** Disconnect wireless connection, Close Moving Scene *******
  sim808.disconnect();

#if SIM_DEBUG
  Serial.println("Sim808 disconnect");
#endif
}
