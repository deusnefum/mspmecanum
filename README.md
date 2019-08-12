# MSPMECANUM

Firmware to make an MSP430 take the PWM signals output (3 axis/channel) from an RC receiver
and do the right trigonometric-magic to drive 4 mecanum (omni-direction) wheels.

I'm making this for a omni-directional skateboard-thing but no reason it won't work for a 
regular robot.

## Disclaimer

This is probably garbage code riddled with bugs and poorly implemented or misunderstood everything.

As of right now (2019-08-11) the code is too big to fit on the inteded chip, an MSP430G2553.