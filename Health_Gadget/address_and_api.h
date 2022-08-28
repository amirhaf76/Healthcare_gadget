#ifndef ADDRESS_AND_KEYS_GADGET_H
#define ADDRESS_AND_KEYS_GADGET_H

// Mocks.
#define API_KEY "3J686EHZ7YHMCBUG"
#define CHANEL_NUMBERS 1693645
#define THING_SPEAK_HOST "api.thingspeak.com"

// APIs.
#define UPDATE_GET_API "update"
#define UPDATE_POST_CSV_API(chanel_num) "channels/" #chanel_num "/bulk_update.csv"

#define API_KEY_PARAM "api_key=%s"
#define DATE_PARAM "created_at=%s"

#define GET_REQ_LINE "GET /%s?%s  HTTP/1.1 \r\n"
#define POST_REQ_LINE "POST /%s?%s  HTTP/1.1 \r\n"

#define HOST_HEADER "Host: %s\r\n"
#define REQ_END "\r\n"

int get_request_line(char * buffer, const char * api, const char * query_params);
int post_request_line(char * buffer, const char * api, const char * query_params);

int add_host_header(char * buffer, const char * host);

int add_api_key_as_param(char * buffer);
int add_date_time(char * buffer, char * date_time);

int add_params_and_value(char * buffer, char * field_name, int value);
int add_params_and_value(char * buffer, char * field_name, float value);
int add_params_and_value(char * buffer, char * field_name, char * value);

void add_ampersand(char * buffer);
void end_request(char * buffer);

void create_request(char * buffer, int (*method)(char * , const char * , const char *), char * api, char * query_params, char * host);

#endif