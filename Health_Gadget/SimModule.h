#ifndef SIM_MODULE_H
#define SIM_MODULE_H


bool get_GPS_data();
int has_signal();
void send_sms(char *, size_t );
void send_data_to_server();

#endif