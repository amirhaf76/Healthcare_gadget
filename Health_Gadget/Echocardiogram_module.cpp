#include "Arduino.h"

#define DEBUG_ECHOCARDIOGRAM_MODULE 1

static int lo_plus = -1;
static int lo_neg = -1;
static int out_pin = -1;

void Echocardiogram_module_setup(int plus, int neg, int o_pin)
{
#if DEBUG_ECHOCARDIOGRAM_MODULE
  Serial.begin(9600);
  Serial.println("Initializing Echo_cardio_module ...");
#endif
  lo_neg = neg;
  lo_plus = plus;
  out_pin = o_pin;

  pinMode(lo_plus, INPUT); // Setup for leads off detection LO +
  pinMode(lo_neg, INPUT);  // Setup for leads off detection LO -
}

/**
 * If one of electrodes are disconnected, it will return -1.
 * Otherwise the result is output of module.
 */
int Echocardiogram_module_loop_step()
{
  int res;

  // It Checks whether the electrodes are connected or not
  if ((digitalRead(lo_neg) == 1) || (digitalRead(lo_plus) == 1))
    res = -1;
  else
    res = analogRead(A0);

#if DEBUG_ECHOCARDIOGRAM_MODULE
  if (res == -1)
    Serial.println("Electrode is disconnected!");
  else
    Serial.println(res);
#endif

  delay(1);

  return res;
}