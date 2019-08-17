#include <msp430.h>
#include <stdint.h>
#include <fastmath.h>
#include "trig.h"


#define X_AXIS BIT0
#define Y_AXIS BIT1
#define A_AXIS BIT2

#define MOTOR1 BIT3
#define MOTOR2 BIT4
#define MOTOR3 BIT5
#define MOTOR4 BIT6

#define ALL_AXES (X_AXIS + Y_AXIS + A_AXIS)
#define ALL_MOTORS (MOTOR1 + MOTOR2 + MOTOR3 + MOTOR4)

// Sample period in microseconds
#define SAMPLE_PERIOD_US (10)

// The total window we'll be sampling for PWM
#define PWM_WINDOW_US (20000)

// The number of frames/samples we'll get
#define PWM_WINDOW (PWM_WINDOW_US/SAMPLE_PERIOD_US)

// The PWM input ranges from 500us to 2500us so we to scale that range to a signed int
#define PWM_MIN_US  500
#define PWM_MAX_US 2500
#define PWM_RANGE_US (PWM_MAX_US-PWM_MIN_US)
#define SAMPLE_RANGE (PWM_RANGE_US/SAMPLE_PERIOD_US)
#define SAMPLE_MIN (PWM_MIN_US/SAMPLE_PERIOD_US)

#define FLOAT2PWM(in) (((unsigned int) in)*(SAMPLE_RANGE/2)+(SAMPLE_RANGE/2)+SAMPLE_MIN)
#define PWM2FLOAT(in) (((((float)in)-((SAMPLE_RANGE/2)+SAMPLE_MIN)))/(SAMPLE_RANGE/2))


enum pwm_buffers {
	XBUF,
	YBUF,
	ABUF
};

enum pwm_outputs {
	M1,
	M2,
	M3,
	M4
};

struct pwm {
	unsigned int buf;   // the current complete width calculated from wpm
	unsigned int prev;  // the previous value (high or low) read from input pin
	unsigned int count; // iterator variable
};

struct pwm_out {
	unsigned int width; // how many out frames to write
	signed int count; // iterator variable
};


void calc_pwm(struct pwm *input, unsigned int cur);
void set_pwm_output (struct pwm_out *out, unsigned int motor);

int main()
{
	/* Bit bangin' magic. Since we need 3 PWM inputs and 4 PWM outputs, it is easier 
	 to just handle this all in software (with the hardware timer / interrupt for 
	 prcise timing.*/
		
	/* stop watchdog */
	WDTCTL = WDTPW | WDTHOLD;
 
	/* load calibration settings
	 * Timing will break if this changes! 
	 */
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;

	// Setup timer
	TACTL = MC_2 | ID_0 | TASSEL_2 | TACLR; // continuous | no prescale | SMCLCK | CLEAR

	// setup GPIO
	P1DIR = 0; // start as all input
	P1DIR |= ALL_MOTORS; // 1 = OUTPUT; set motors to output
	P1OUT &= ~ALL_MOTORS; // 0 = start/idle low
	//P1SEL |= COMM; // change function from GPIO to TimerA
	
	// pwm buffers
	struct pwm inputs[3];

	struct pwm_out outputs[4];

	unsigned int p1inbuf; // so we can capture a single 'frame' of input that won't change under us

	// Every 0.1ms (100 microseconds), read our inputs, do some math, update our outputs. Easy, right?
	for (;;) {
		// set the interrupt time to SAMPLE_PERIOD_US ahead of current time (assuming 1MHz clock)
		TACCR0 = TAR + SAMPLE_PERIOD_US;

		// store P1IN value so it can't change under us / we're looking at a single slice in time
		p1inbuf = P1IN;

		calc_pwm(&inputs[XBUF], p1inbuf & X_AXIS ? 1 : 0);
		calc_pwm(&inputs[YBUF], p1inbuf & Y_AXIS ? 1 : 0);
		calc_pwm(&inputs[ABUF], p1inbuf & A_AXIS ? 1 : 0);

		/*
		 * The value in inputs[].buf is the length of the pwm pulse. 150 (1.5ms) is neutral
		 * 50 (0.5ms) is minimum value and 250 (2.5ms) is maximum value
		 */

		/*
		Dumb diagram to help track what motor is where.

		    M1 --- M2
		      /   \
		     |     |
		     |     |
		      \   /
		    M3 --- M4

		 */

		// If this trig garbage takes to long to complete we can opt to do it only every
		// other loop or 1/10th a loop (or up the DCO clock)

		// do trig here to reconcile X, Y, and A axes
		// LMAO, I just realized I know how to reconsile X and Y but A is 🤷
		// Okay, internet to the rescue... found some equations to crib

		float x = PWM2FLOAT(inputs[XBUF].buf);
		float y = PWM2FLOAT(inputs[YBUF].buf);
		float a = -PWM2FLOAT(inputs[ABUF].buf);

		// derive theta, magnitude, and rotation
		float theta_d = trig_tanf(x/y);
		float v_d = 0; sqrt(x*x + y*y);
		float v_theta = (a + 1) * M_PI;

		outputs[M1].width = FLOAT2PWM(v_d * trig_sinf(-theta_d + M_PI/4) - v_theta);
		outputs[M2].width = FLOAT2PWM(v_d * trig_cosf(-theta_d + M_PI/4) + v_theta);
		outputs[M3].width = FLOAT2PWM(v_d * trig_cosf(-theta_d + M_PI/4) - v_theta);
		outputs[M4].width = FLOAT2PWM(v_d * trig_sinf(-theta_d + M_PI/4) + v_theta);
		
		// Update PWM targets

		// Actually set output pins high/low
		set_pwm_output(&outputs[M1], MOTOR1);
		set_pwm_output(&outputs[M2], MOTOR2);
		set_pwm_output(&outputs[M3], MOTOR3);
		set_pwm_output(&outputs[M4], MOTOR4);

		TACCTL0 = CCIE; // I think I can set this once and then don't have to keep setting it
		// Go to sleep until TAR hits the value in TACCR0
		__low_power_mode_0();
	}

	return 0; // shut up gcc
}

inline void calc_pwm(struct pwm *input, unsigned int cur) {
	if (cur == input->prev) {
		if (cur) {
			input->count++;
			// input->count += 0xFFFF/SAMPLE_RANGE - 1;
		}
	} else {
		if (cur) {
			input->count = 1;
		} else {
			input->buf = input->count;
		}
	}
}

inline void set_pwm_output (struct pwm_out *out, unsigned int motor) {
	if (--out->count <= 0) {
		if (P1OUT & MOTOR1) {
			// output was high, time to switch to low-output
			out->count = PWM_WINDOW - out->width;
		} else {
			out->count = out->width;
		}
		P1OUT ^= motor;
	}
}

__interrupt_vec(TIMER0_A0_VECTOR) void timerisr()
{
	__low_power_mode_off_on_exit();
}

