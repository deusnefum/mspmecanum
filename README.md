# MSPMECANUM

Firmware to make an MSP430 take the PWM signals output (3 axis/channel) from an RC receiver
and do the right trigonometric-magic to drive 4 mecanum (omni-direction) wheels.

I'm making this for a omni-directional skateboard-thing but no reason it won't work for a 
regular robot.

## Disclaimer

This is probably garbage code riddled with bugs and poorly implemented or misunderstood everything.

The code now compiles and seemingly fits on an MSP430G2553. I wrote some trig funcs that hopefully
strike the right balance between fast and accurate.