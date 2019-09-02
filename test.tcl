#!/usr/bin/env tclsh

# // Sample period in microseconds
set SAMPLE_PERIOD_US 10

# // The total window we'll be sampling for PWM
set PWM_WINDOW_US 20000

# // The number of frames/samples we'll get
set PWM_WINDOW [expr {$::PWM_WINDOW_US/$::SAMPLE_PERIOD_US}]

# // The PWM input ranges from 500us to 2500us so we to scale that range to a signed int
set PWM_MIN_US 500
set PWM_MAX_US 2500
set PWM_RANGE_US [expr {$::PWM_MAX_US-$::PWM_MIN_US}]
set SAMPLE_RANGE [expr {$::PWM_RANGE_US/$::SAMPLE_PERIOD_US}]
set SAMPLE_MIN [expr {$::PWM_MIN_US/$::SAMPLE_PERIOD_US}]
set SAMPLE_MAX [expr {$::PWM_MAX_US/$::SAMPLE_PERIOD_US}]


proc PWM2FIX16 {in} {
	set in [expr $in * 1.0]
	expr {( ($in-($::SAMPLE_RANGE/2))-$::SAMPLE_MIN ) / ($::SAMPLE_RANGE/2)}
}

proc FIX162PWM {in} {
#define FIX162PWM(in) ((unsigned int)fix16_to_float(fix16_add(fix16_mul(in,fix16_from_int(SAMPLE_RANGE)),fix16_from_int(SAMPLE_MIN))))
	expr {int(($in * ($::SAMPLE_RANGE/2))) + $::SAMPLE_MIN + $::SAMPLE_RANGE/2}
}

set x [PWM2FIX16 150]
set y [PWM2FIX16 150]
set a [PWM2FIX16 150]

# // derive theta, magnitude, and rotation
set theta_d [expr {atan2($x,$y)}]
#set theta_d [atan2 $x $y]
set v_d [expr {sqrt($x**2 + $y**2)}]
set v_theta $a

set trig_arg [expr {(3.14159267 * 3 / 4) - $theta_d}]

#outputs[M1].width = FIX162PWM(fix16_sub(fix16_mul(v_d, fix16_sin(trig_arg)), v_theta));
#puts [expr {$v_d * sin($trig_arg) - $v_theta}]
puts "M1: [FIX162PWM [expr {$v_d * sin($trig_arg) - $v_theta}]]"
# outputs[M2].width = FIX162PWM(fix16_add(fix16_mul(v_d, fix16_cos(trig_arg)), v_theta));
puts "M2: [FIX162PWM [expr {$v_d * cos($trig_arg) + $v_theta}]]"
# outputs[M3].width = FIX162PWM(fix16_sub(fix16_mul(v_d, fix16_cos(trig_arg)), v_theta));
puts "M3: [FIX162PWM [expr {$v_d * cos($trig_arg) - $v_theta}]]"
# outputs[M4].width = FIX162PWM(fix16_add(fix16_mul(v_d, fix16_sin(trig_arg)), v_theta));
puts "M4: [FIX162PWM [expr {$v_d * sin($trig_arg) + $v_theta}]]"

proc atan {x} {
	set M_PI 3.1415927
	set M_PI_2 1.570796335
	# set out [expr {0.9724 * $x - 0.1919 * $x * $x * $x}]
	if {$x > 1 || $x < -1} {
		set out [expr {$M_PI_2 - (1/(abs($x) + 0.2733))}]
		if {$x < 0} {
			return [expr {-$out}]
		}
		return $out
	}
	set out [expr {$x / (1 + 0.28125*$x**2)}]
	if {$out > $M_PI_2} {
		return $M_PI_2
	}
	if {$out < -$M_PI_2} {
		return -$M_PI_2
	}

	return $out
}

# proc atan {x} {
	# expr {$x - $x**3/3 + $x**5/5 - $x**7/7 + $x**9/9}
# }

proc atan2 {y x} {
	set M_PI 3.1415927
	set M_PI_2 1.570796335
	if {$x > 0} {
		return [atan [expr {$y/$x}]]
	}
	if {$x < 0 && $y >= 0} {
		set atan [atan [expr {$y/$x}]]
		return [expr {$atan + $M_PI}]
	}
	if {$x < 0 && $y < 0} {
		set atan [atan [expr {$y/$x}]]
		return [expr {$atan - $M_PI}]
	}
	if {$x == 0 && $y > 0} {
		return $M_PI_2
	}
	if {$x == 0 && $y < 0} {
		return -$M_PI_2
	}
}
#return
for {set x -1} { $x < 1 } {set x [expr {$x + 0.01 }]} {
	for {set y -1} { $y < 1 } {set y [expr {$y + 0.01 }]} {
		#puts "x: $x  y: $y"
		set real [expr round(1000*atan2($x,$y))]
		set mine [expr {round(1000*[atan2 $x $y])}]
		if {abs($real - $mine) > 3} {
			puts "x: $x  y: $y"
			puts "real: $real [expr (10*atan2($x,$y))]"
			puts "mine: $mine [expr {(10*[atan2 $x $y])}]"
		}
	}
}