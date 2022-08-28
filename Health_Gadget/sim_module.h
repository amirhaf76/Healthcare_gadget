#ifndef SIM_MODULE_H
#define SIM_MODULE_H

#include "DFRobot_SIM808.h"

const int8_t DATE_CHARS_SIZ = 17;

bool setup_sim_module();

bool gps_setup();
bool get_gps_data(char * date, float * lat, float * lon);

bool send_sms(char * phone_number, char * message);
bool send_data();
bool send_data(char * http_cmd);

int has_signal();

#endif