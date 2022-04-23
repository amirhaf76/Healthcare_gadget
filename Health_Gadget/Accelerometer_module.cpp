#include "health_helper.h"
#include "Accelerometer_module.h"
#include "Arduino.h"

#define DEBUG_ACCELEROMETER 1

Point current_point;
int g_xpin = -1;
int g_ypin = -1;
int g_zpin = -1;
int CF = 1;

long unsigned steps;

/* Stepup Declarations */
void Accelerometer_setup(int xpin, int ypin, int zpin)
{
    g_xpin = xpin;
    g_ypin = ypin;
    g_zpin = zpin;
#if DEBUG
    Serial.println("Initializing Accelerometer...");
#endif
    current_point.x = analogRead(xpin);
    current_point.y = analogRead(ypin);
    current_point.z = analogRead(zpin);
}

void Accelerometer_loop_step()
{
    if (g_xpin == -1 || g_ypin == -1 || g_zpin)
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

    if (bandWithFilter(result, 18, 30))
        ++steps;

    delay(100);
}

long int Accelerometer_get_steps() {
    return steps;
}
