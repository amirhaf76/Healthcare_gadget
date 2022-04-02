#ifndef HEALTH_HELPER
#define HEALTH_HELPER
#include <math.h>
#include <stdlib.h>


typedef struct {
    float x;
    float y;
    float z;
} Point;

/* cf: coefficient of filter
 * x: input
*/
int noiseFilter(double x,float cf);

double magnitude_calculated(Point p);
double calculate_mean(const int siz, const double arr[], int* err);
int find_local_max(const int siz, const double arr[], double min_value);
double find_max(const int siz, const double arr[]);

double magnitude_calculated(Point p) {
    return sqrt(
            pow(p.x, 2.0) +
            pow(p.y, 2.0) +
            pow(p.z, 2.0)
    );
}


/* cf: coefficient of filter
 * x: input
*/
int noiseFilter(double x,float cf) {
  if (abs(x) < cf *10) return 0;

  return x;
}


double calculate_mean(const int siz, const double arr[], int* err) {
    if (err != nullptr) {
        if (siz < 1) {
            *err = -1;
            return 0;
        }
        *err = 0;
    }

    double sum = 0;

    for (int i=0; i < siz; ++i)
        sum += arr[i];

    return sum/siz;
}


int find_local_max(const int siz, const double arr[], double min_value) {

    // For founding local maximum, it needs at
    // least 3 or more than 3 inputs.
    if (siz < 3)
        return -1;

    // Initial number of local maximum.
    int num_max = 0;

    // First input.
    if (arr[0] > arr[1] && arr[0] >= min_value)
        ++num_max;

    // Counting local maximum.
    for (int i = 1; i < siz - 1; ++i) {
        if (arr[i-1] < arr[i] && arr[i] > arr[i+1] && arr[i] >= min_value)
            ++num_max;
    }

    // Last input.
    if (arr[siz - 1] > arr[siz - 2] && arr[siz - 1] >= min_value)
        ++num_max;

    return num_max;
}


double find_max(const int siz, const double arr[]) {
  if (siz < 1)
    return -1;

  if (siz == 1)
    return arr[0];

  double max_num = arr[0];
  for (int i = 1; i < siz; ++i) {
    if (max_num < arr[i])
      max_num = arr[i];
  }

  return max_num;
}


#define CONTROLLERS_NUMBERS 7
enum Controller {
    /** Number of controllers should be
     * matched with Macro CONTROLLERS_NUMBERS.
     */
    HRB_OX_MODULE,
    TEMPERATURE_MODULE, 
    KY_039_, 
    Accelerator_Meter, 
    SD_MODULE, 
    ECHO_CARDIO_MODULE,
    None
};

#endif
