#include "health_helper.h"
#include "math.h"
#include "Arduino.h"


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
double noiseFilter(double x,float cf) {
  if (fabs(x) < cf *10) 
    return 0.0;

  return x;
}

int bandWithFilter(double x, int min_val, int max_val) {
  if (min_val <=x && x <= max_val)
    return 1;

  return 0;
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

void set_time(unsigned long * time_set) {
  *time_set = millis();
}

bool is_time_pass(unsigned long * time_set, unsigned long during) 
{
  return (millis() - *time_set) >= during || (millis() - *time_set) < 0;
}
