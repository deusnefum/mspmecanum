#include <stdint.h>
#include "trig.h"

float trig_sinf (float x) {
	while (x > 2 * M_PI)
		x -= 2 * M_PI;

	// less than 45 degrees
	if (x < M_PI/4)
		return x - (x*x*x / 6) + (x * x * x * x * x / 120);

	// less than 90 degrees
	if (x < M_PI/2)
		return trig_cosf(M_PI/2 - x);
		
	// less than 180
	if (x < M_PI)
		return trig_sinf(M_PI - x);

	return -trig_sinf(2*M_PI - x);
}

float trig_cosf (float x) {
	while (x > 2 * M_PI)
		x -= 2 * M_PI;

	// less than 45 degrees
	if (x < M_PI/4)
		return 1 - (x*x/2) + (x*x*x*x/24) - (x*x*x*x*x*x/720);

	// less than 90
	if (x < M_PI/2)
		return trig_sinf(M_PI/2 - x);

	// less than 180
	if (x < M_PI)
		return -trig_cosf(M_PI - x);

	// less than 270
	if (x < M_PI * 3 / 4)
		return -trig_cosf(x - M_PI);

	// 4th quadrant is positive
	return trig_cosf(2*M_PI - x);
}