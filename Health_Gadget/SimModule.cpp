#include <DFRobot_SIM808.h>
#include <SoftwareSerial.h>
#include <sim808.h>

#define SIM_DEBUG 1

#define PIN_TX    10
#define PIN_RX    11
#define SIM808_BUFFER_SIZE 512

SoftwareSerial mySerial(PIN_TX,PIN_RX);
DFRobot_SIM808 sim808(&mySerial);//Connect RX,TX,PWR,

char http_cmd[] = "GET /media/uploads/mbed_official/hello.txt HTTP/1.0\r\n\r\n";
char buffer[SIM808_BUFFER_SIZE];

int8_t setup_simModule() {
  mySerial.begin(9600);

  #if SIM_DEBUG
  Serial.begin(9600);
  #endif

  uint8_t counter = 0;

  while(!sim808.init()) { 
		delay(1000);

		#if SIM_DEBUG
		Serial.print("Sim808 init error\r\n");
		#endif

    if ((counter++) == 5)
      return -1;
  }

  return 0;
}

void send_sms(char * phone_number, char * message) 
{
  bool res = sim808.sendSMS(phone_number,message); 

  #if SIM_DEBUG
		Serial.print("phone_number: ");
		Serial.print(phone_number);
		Serial.print(", message: ");
		Serial.println(message);
    Serial.println((res) ? "Message sent." : "Message failed.");
	#endif
}

bool gps_setup() {

  //************* Turn on the GPS power************
  bool res = sim808.attachGPS();

	#if SIM_DEBUG
  if(res)
      Serial.println("Open the GPS power success");
  else 
      Serial.println("Open the GPS power failure");
	#endif
  
  return res;
}

bool get_GPS_data() {
	 //************** Get GPS data *******************
	 bool status = sim808.getGPS();
   if (status) {
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

  #if SIM_DEBUG
  Serial.print("Signal Strength: ");
  Serial.println(power);
  #endif

  return power;
}

bool send_data(char * http_str, int siz_http_str) {
  
  int8_t counter = 5;
  //*********** Attempt DHCP *******************
  while(!sim808.join(F("cmnet"))) {

    #if SIM_DEBUG
    Serial.println("Sim808 join network error");
    #endif
    
    if (counter++ == 5) return false;

    delay(2000);
  }

  //************ Successful DHCP ****************
  #if SIM_DEBUG
  Serial.print("IP Address is ");
  Serial.println(sim808.getIPAddress());
  #endif

  //*********** Establish a TCP connection ************
  if(!sim808.connect(TCP,"mbed.org", 80)) {
    #if SIM_DEBUG
    Serial.println("Connect error");
    #endif
  }else{
    #if SIM_DEBUG
    Serial.println("Connect mbed.org success");
    #endif
  }

  //*********** Send a GET request *****************
  #if SIM_DEBUG
  Serial.println("waiting to fetch...");
  #endif

  sim808.send(http_str, sizeof(siz_http_str)-1);

  while (true) {
    int ret = sim808.recv(buffer, SIM808_BUFFER_SIZE-1);
    
    if (ret <= 0){

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
}