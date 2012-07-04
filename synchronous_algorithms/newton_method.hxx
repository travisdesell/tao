#ifndef TAO_NEWTON_METHOD_H
#define TAO_NEWTON_METHOD_H

void newton_step__alloc(int number_parameters, double **hessian, double *gradient, double **step);
void newton_step(int number_parameters, double **hessian, double *gradient, double *step);

#endif