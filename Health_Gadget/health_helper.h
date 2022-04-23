
#ifndef HEALTH_HELPER_h
#define HEALTH_HELPER_h
#include <math.h>
#include <stdlib.h>

#define CONTROLLERS_NUMBERS 7

enum Controller {
    /** Number of controllers should be
     * matched with Macro CONTROLLERS_NUMBERS.
     */
    HRB_OX_MODULE,
    TEMPERATURE_MODULE, 
    KY_039_, 
    ACCELEROMETER_MODULE, 
    SD_MODULE, 
    ECHOCARDIOGRAM_MODULE,
    None
};

typedef struct {
    float x;
    float y;
    float z;
} Point;


double noiseFilter(double x,float cf);
double magnitude_calculated(Point p);
double calculate_mean(const int siz, const double arr[], int* err);
int bandWithFilter(double x, int min_val, int max_val);
int find_local_max(const int siz, const double arr[], double min_value);
double find_max(const int siz, const double arr[]);


#endif
