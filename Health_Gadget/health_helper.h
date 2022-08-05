
#ifndef HEALTH_HELPER_h
#define HEALTH_HELPER_h
#include <math.h>
#include <stdlib.h>

#define CONTROLLERS_NUMBERS 6

enum Controller {
    /** 
     * Number of controllers should be
     * matched with Macro CONTROLLERS_NUMBERS.
     */
    HRB_MODULE,
    O2_MODULE,
    TEMPERATURE_MODULE, 
    ACCELEROMETER_MODULE, 
    SD_MODULE, 
    ECHOCARDIOGRAM_MODULE,
    NONE_MODULE,
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

int num_len(unsigned long num);

// void set_time(unsigned long * curr_time);
// bool is_time_pass(unsigned long * curr_time, unsigned long during);

#endif
