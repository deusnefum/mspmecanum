#include <msp430.h>
#include <stdint.h>
#include <fixmath.h>


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
#define SAMPLE_MAX (PWM_MAX_US/SAMPLE_PERIOD_US)

#define FIX162PWM(in) ((unsigned int)fix16_to_float(fix16_add(fix16_mul(in,fix16_from_int(SAMPLE_RANGE/2)),fix16_from_int(SAMPLE_MIN+SAMPLE_RANGE/2))))
#define PWM2FIX16(in) (fix16_div(fix16_sub(fix16_from_int(in),fix16_from_int(((SAMPLE_RANGE/2)+SAMPLE_MIN))), fix16_from_int(SAMPLE_RANGE/2)))

#define MAX(_v,_max) (_v > _max ? _max : _v)
#define MIN(_v,_min) (_v < _min ? _min : _v)
#define MINMAX(_v, _min, _max) (MAX(MIN(_v, _min),_max))

#define XBUF 0
#define YBUF 1
#define ABUF 2

#define M1 0
#define M2 1
#define M3 2
#define M4 3

struct pwm {
	unsigned int buf;   // the current complete width calculated from wpm
	unsigned int prev;  // the previous value (high or low) read from input pin
	unsigned int count; // iterator variable
};

struct pwm_out {
	unsigned int width; // how many out frames to write
	signed int count; // iterator variable
};

void get_pwm_input (struct pwm *input, unsigned int cur);
void set_pwm_output (struct pwm_out *out, unsigned int motor);

volatile int recompute_flag = 0;

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
	BCSCTL1 = CALBC1_16MHZ;
	DCOCTL = CALDCO_16MHZ;

	// Setup timer
	TACTL = MC_2 | ID_0 | TASSEL_2 | TACLR; // continuous | no prescale | SMCLCK | CLEAR

	// setup GPIO
	P1DIR = 0; // start as all input
	P1DIR |= ALL_MOTORS; // 1 = OUTPUT; set motors to output
	P1OUT &= ~ALL_MOTORS; // 0 = start/idle low
	//P1SEL |= COMM; // change function from GPIO to TimerA
	
	// pwm buffers
	struct pwm inputs[3] = {
		 {((SAMPLE_RANGE/2)+SAMPLE_MIN),0,0},
		 {((SAMPLE_RANGE/2)+SAMPLE_MIN),0,0},
		 {((SAMPLE_RANGE/2)+SAMPLE_MIN),0,0}
	};

	struct pwm_out outputs[4] = {
		{((SAMPLE_RANGE/2)+SAMPLE_MIN),0},
		{((SAMPLE_RANGE/2)+SAMPLE_MIN),0},
		{((SAMPLE_RANGE/2)+SAMPLE_MIN),0},
		{((SAMPLE_RANGE/2)+SAMPLE_MIN),0}
	};

	unsigned int p1inbuf; // so we can capture a single 'frame' of input that won't change under us
	// Every 0.1ms (100 microseconds), read our inputs, do some math, update our outputs. Easy, right?
	TACCTL0 = CCIE; 
	for (;;) {
		// set the interrupt time to SAMPLE_PERIOD_US ahead of current time (assuming 1MHz clock)
		// emprically determined timing adjustment
		TACCR0 = TAR + (SAMPLE_PERIOD_US-1) * 16;

		// store P1IN value so it can't change under us / we're looking at a single slice in time
		p1inbuf = P1IN;
		get_pwm_input(&inputs[XBUF], p1inbuf & X_AXIS ? 1 : 0);
		get_pwm_input(&inputs[YBUF], p1inbuf & Y_AXIS ? 1 : 0);
		get_pwm_input(&inputs[ABUF], p1inbuf & A_AXIS ? 1 : 0);

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

		// if input PWM has changed, recompute our outputs
		if (recompute_flag) {
			fix16_t x = PWM2FIX16(inputs[XBUF].buf);
			fix16_t y = PWM2FIX16(inputs[YBUF].buf);
			fix16_t v_theta = PWM2FIX16(inputs[ABUF].buf);
			
			// derive theta (angle of desired movement), and magnitude (v_d)
			fix16_t theta_d = fix16_atan2(y,x);
			fix16_t v_d = fix16_sqrt(fix16_add(fix16_mul(x,x), fix16_mul(y,y)));

			fix16_t trig_arg = fix16_sub(PI_DIV_4, theta_d);
			fix16_t vdcos = fix16_mul(v_d, fix16_cos(trig_arg));
			fix16_t vdsin = fix16_mul(v_d, fix16_sin(trig_arg));
			
			outputs[M1].width = FIX162PWM(fix16_sub(vdsin, v_theta));
			outputs[M2].width = FIX162PWM(fix16_add(vdcos, v_theta));
			outputs[M3].width = FIX162PWM(fix16_sub(vdcos, v_theta));
			outputs[M4].width = FIX162PWM(fix16_add(vdsin, v_theta));

			recompute_flag = 0;
		}
		
		// Update PWM targets

		// Actually set output pins high/low
		set_pwm_output(&outputs[M1], MOTOR1);
		set_pwm_output(&outputs[M2], MOTOR2);
		set_pwm_output(&outputs[M3], MOTOR3);
		set_pwm_output(&outputs[M4], MOTOR4);

		// if we did a recompute, we may have missed the sleep window
		// if we go to sleep *after* the sleep window, we sleep for *way*
		// too long.
		// the OR is there incase we wrapped the integer
		if (TAR < TACCR0 || TAR + (SAMPLE_PERIOD_US-1) * 16 < TACCR0 + (SAMPLE_PERIOD_US-1) * 16)
			// Go to sleep until TAR hits the value in TACCR0
			__low_power_mode_0();
	}

	return 0; // shut up gcc
}

inline void get_pwm_input(struct pwm *input, unsigned int cur) {
	if (cur == input->prev) {
		if (cur) input->count++;
	} else {
		if (cur) {
			input->count = 1;
		} else {
			if (input->buf != input->count) {
				recompute_flag = 1;
				input->buf = input->count;
			}
			if (input->buf < SAMPLE_MIN)
				input->buf = SAMPLE_MIN;
			if (input->buf > SAMPLE_MAX)
				input->buf = SAMPLE_MAX;
		}
	}
}

inline void set_pwm_output (struct pwm_out *out, unsigned int motor) {
	if (--out->count <= 0) {
		if (P1OUT & motor) {
			// output was high, time to switch to low-output
			// TODO: There's a minor bug here. if we change the width we'll 
			// have a single frame that's off and will cause our PWm singles to fall out of
			// sync--probably doesn't matter 
			out->count = PWM_WINDOW - out->width;
		} else {
			out->count = out->width;
		}
		P1OUT ^= motor;
	}
}

__interrupt_vec(TIMER0_A0_VECTOR) void timerisr(){
	__low_power_mode_off_on_exit();
}
