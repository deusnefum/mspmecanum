#include <stdint.h>

#define M_PI 3.14159265358979323846
#define trig_tanf(x) (trig_sinf(x)/trig_cosf(x))

float trig_sinf (float x);
float trig_cosf (float x);
float trig_atan (float x);
float trig_atan2 (float y, float x);