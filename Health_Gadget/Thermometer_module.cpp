#include <Wire.h>
#include "Protocentral_MAX30205.h"
#include "Arduino.h"

#define DEBUG_THERMOMETER_MODULE 1


static MAX30205 THERMOMETER_SENSOR;
static double g_temperature = 0;


void Thermometer_module_setup()
{
#if DEBUG_THERMOMETER_MODULE
    Serial.begin(9600);
    Serial.println("Initializing Thermometer ...");
#endif

    Wire.begin();

    short counter = 0;

    // scan for temperature in every 30 sec untill a sensor is found. Scan for both addresses 0x48 and 0x49
    while (!THERMOMETER_SENSOR.scanAvailableSensors())
    {
        if ((++counter) == 3)
            return;

#if DEBUG_THERMOMETER_MODULE
        Serial.println("Couldn't find the temperature sensor, please connect the sensor.");
#endif

        delay(5000);
    }

    THERMOMETER_SENSOR.begin(); // set continuos mode, active mode
}

// read temperature for every 100ms
void Thermometer_module_loop_step()
{
    g_temperature = THERMOMETER_SENSOR.getTemperature();

#if DEBUG_THERMOMETER_MODULE
    Serial.print(g_temperature, 2);
    Serial.println("'c");
#endif

    delay(100);
}


double Thermometer_get_temperature() {
    return g_temperature;
}
