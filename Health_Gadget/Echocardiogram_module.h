#ifndef ECHOCARDIOGRAM_MODULE_H
#define ECHOCARDIOGRAM_MODULE_H

/**
 * @brief set up echocardogram
 * @param LO+ leadout +
 * @param LO- leadout -
 */
void Echocardiogram_module_setup(int plus, int neg); // ok
int Echocardiogram_module_loop_step(); // ok

#endif