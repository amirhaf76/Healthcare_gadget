#ifndef SIM_MODULE_H
#define SIM_MODULE_H

#include "DFRobot_SIM808.h"

int8_t setup_simModule();

int has_signal();

bool send_sms(char * phone_number, char * message);

bool gps_setup();
bool get_GPS_data();
bool send_data(DFRobot_SIM808::gspdata& );


#endif