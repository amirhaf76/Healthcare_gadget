#include "health_helper.h"
#include "Accelerometer_module.h"
#include "Arduino.h"

#define DEBUG_ACCELEROMETER 1

#define MAX_BOUND 30
#define MIN_BOUND 18
Point current_point;
int g_xpin = -1;
int g_ypin = -1;
int g_zpin = -1;
int CF = 1;

long unsigned time = 0;

static unsigned long steps = 0;


void accelerometer_setup(int xpin, int ypin, int zpin)
{
    g_xpin = xpin;
    g_ypin = ypin;
    g_zpin = zpin;

#if DEBUG_ACCELEROMETER
    Serial.begin(9600);
    Serial.println("Initializing Accelerometer...");
#endif

    current_point.x = analogRead(xpin);
    current_point.y = analogRead(ypin);
    current_point.z = analogRead(zpin);
}


void accelerometer_loop_step()
{
    if (g_xpin == -1 || g_ypin == -1 || g_zpin == -1)
        return; // one or more than one pin numbers are not correct

    Point temporary_point;
    temporary_point.x = analogRead(g_xpin);
    temporary_point.y = analogRead(g_ypin);
    temporary_point.z = analogRead(g_zpin);

    // Calculate difference between two points
    Point diff;
    diff.x = temporary_point.x - current_point.x;
    diff.y = temporary_point.y - current_point.y;
    diff.z = temporary_point.z - current_point.z;

    double result = noiseFilter(magnitude_calculated(diff), CF);

    current_point = temporary_point;

#if DEBUG_ACCELEROMETER
    if (1) {
        Serial.println("----- Steps -----");
        Serial.print("current: ");
        Serial.print(steps);
        Serial.print(", current res: ");
        Serial.println(result);
    }
#endif

    if (bandWithFilter(result, MIN_BOUND, MAX_BOUND)) {
        ++steps;
    }

    delay(100);
}


unsigned long accelerometer_get_steps() {
    return steps;
}
