
#ifndef HEALTH_HELPER_h
#define HEALTH_HELPER_h
#include <math.h>
#include <stdlib.h>



typedef struct
{
    double temperature;
    int o2_measure;
    int avg_bpm = 0;
    unsigned char echo_result_index;
    unsigned long int steps;
} RecordData;

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
