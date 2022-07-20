#ifndef SIM_MODULE_H
#define SIM_MODULE_H


int8_t setup_simModule();

int has_signal();
void send_sms(char * phone_number, char * message);
bool gps_setup();
bool get_GPS_data();
bool send_data(char * http_str, int siz_http_str);


#endif