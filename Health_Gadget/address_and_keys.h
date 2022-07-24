#ifndef ADDRESS_AND_KEYS_GADGET_H
#define ADDRESS_AND_KEYS_GADGET_H

#define API_KEY "3J686EHZ7YHMCBUG"
#define CHANEL_NUMBERS 1693645
#define THING_SPEAK_HOST "api.thingspeak.com"

// APIs.
#define UPDATE_GET_API "update"
#define UPDATE_POST_CSV_API(chanel_num) "channels/" #chanel_num "/bulk_update.csv"

// Qurey params.
#define QUERY_PARAMS(api_key, field_name,  field_value, created_at_value) \
		"api_key=" api_key "&" \
		#field_name "=" #field_value "&" \
		"created_at=" created_at_value

// Get methods.
#define GET_METHOD(host, api, query_params) \
		"GET /" api "?" query_params " HTTP/1.1 \r\n" \
		"Host: " host "\r\n\r\n"


#endif