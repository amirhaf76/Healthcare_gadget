#include <DFRobot_SIM808.h>
#include <SoftwareSerial.h>
#include <sim808.h>

#define GPS_DEBUG 0

#define PIN_TX    10
#define PIN_RX    11
SoftwareSerial mySerial(PIN_TX,PIN_RX);
DFRobot_SIM808 sim808(&mySerial);//Connect RX,TX,PWR,

void GPS_setup() {
  //******** Initialize sim808 module *************
  while(!sim808.init()) { 
		delay(1000);
		#if GPS_DEBUG
		Serial.print("Sim808 init error\r\n");
		#endif
  }

  //************* Turn on the GPS power************
	#if GPS_DEBUG
  if( sim808.attachGPS())
      Serial.println("Open the GPS power success");
  else 
      Serial.println("Open the GPS power failure");
	#else
	sim808.attachGPS();
	#endif
}

bool get_GPS_data() {
	 //************** Get GPS data *******************
	 bool status = sim808.getGPS();
   if (status) {
		#if GPS_DEBUG
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
    Serial.println(sim808.GPSdata.lat,6);
    
    sim808.latitudeConverToDMS();
    Serial.print("latitude :");
    Serial.print(sim808.latDMS.degrees);
    Serial.print("^");
    Serial.print(sim808.latDMS.minutes);
    Serial.print("\'");
    Serial.print(sim808.latDMS.seconeds,6);
    Serial.println("\"");
    Serial.print("longitude :");
    Serial.println(sim808.GPSdata.lon,6);
    sim808.LongitudeConverToDMS();
    Serial.print("longitude :");
    Serial.print(sim808.longDMS.degrees);
    Serial.print("^");
    Serial.print(sim808.longDMS.minutes);
    Serial.print("\'");
    Serial.print(sim808.longDMS.seconeds,6);
    Serial.println("\"");
    
    Serial.print("speed_kph :");
    Serial.println(sim808.GPSdata.speed_kph);
    Serial.print("heading :");
    Serial.println(sim808.GPSdata.heading);
		#endif

    //************* Turn off the GPS power ************
    sim808.detachGPS();

	}

	return status;
}

int has_signal() {
  int power;
  sim808.getSignalStrength(&power);

  return power;
}
