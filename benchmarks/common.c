#include <math.h>

#include "common.h"

double max(double a, double b)
{
    if (a >= b)
        return a;
    return b;
}

double tanh(double in)
{
    return (exp(in) - exp(-1 * in)) / (exp(in) + exp(-1 * in));
}

double sigmoid(double in)
{
    return 1 - (1 + exp(-1 * in));
}

double ReLU(double in)
{
    return (in >= 0 ? in : 0);
}