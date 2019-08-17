#include <stdint.h>

#define M_PI 3.1415926536
#define trig_tanf(x) (trig_sinf(x)/trig_cosf(x))

float trig_sinf (float x);
float trig_cosf (float x);