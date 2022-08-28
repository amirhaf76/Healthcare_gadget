#include <string.h>
#include "address_and_api.h"
#include <stdio.h>

int get_request_line(char * buffer, const char * api, const char * query_params) 
{
    return sprintf(buffer, GET_REQ_LINE, api, query_params);
}

int post_request_line(char * buffer, const char * api, const char * query_params) 
{
    return sprintf(buffer, POST_REQ_LINE, api, query_params);
}

int add_host_header(char * buffer, const char * host)
{
    char temp_buffer[50];
    
    int len = sprintf(temp_buffer, HOST_HEADER, host);

    strcat(buffer, temp_buffer);

    return len;
}

int add_api_key_as_param(char * buffer)
{
    char temp_buffer[50];

    int len = sprintf(temp_buffer, API_KEY_PARAM, API_KEY);

    strcat(buffer, temp_buffer);

    return len;
}

int add_date_time(char * buffer, char * date_time)
{
    char temp_buffer[20];
    
    int len = sprintf(temp_buffer, DATE_PARAM, date_time);

    strcat(buffer, temp_buffer);

    return len;
}

int add_params_and_value(char * buffer, char * field_name, int value) 
{
    char temp_buffer[50];
    
    int len = sprintf(temp_buffer, "%s=%d", field_name, value);

    strcat(buffer, temp_buffer);

    return len;
}
int add_params_and_value(char * buffer, char * field_name, float value) 
{
    char temp_buffer[50];
    
    int len = sprintf(temp_buffer, "%s=%f", field_name, value);

    strcat(buffer, temp_buffer);

    return len;
}
int add_params_and_value(char * buffer, char * field_name, char * value) 
{
    char temp_buffer[50];
    
    int len = sprintf(temp_buffer, "%s=%s", field_name, value);

    strcat(buffer, temp_buffer);

    return len;
}

void add_ampersand(char * buffer) 
{
    strcat(buffer, "&");
}

void end_request(char * buffer) 
{
    strcat(buffer, REQ_END);
}

void create_request(char * buffer, int (*method)(char * , const char * , const char *), char * api, char * query_params, char * host) 
{
    method(buffer, api, query_params);
    add_host_header(buffer, host);
    end_request(buffer);
}