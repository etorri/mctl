# Objective

The purpose of this pile of rubbish is to build a controller between
Raspberry Pi UART (or SPI) and Pololu 708 (Dual VNH2SP30 Motor Driver
Carrier MD03A) with two DC gear motors (FIT0277).

The controller is based on msp430G2553 (20-pin) because I have several of them.
Besides that, it has nice peripherals to implement

- wall clock with timer A0 to measure time intervals (encoders)
- sufficiently fast two channel PWM (10-20KHz) with timers A1
- UART,SPI or I2C clock
- interrupts from the rotary encoders of the motors.
- just enough IO ports

The code is designed for this particular application. It may have
useful components and "Notes to Self" comments that somebody finds
useful.

# Control?

The Pi tells the controller to

- Initialize
- Break
- Prepare to run
- Run with given PWM values
- Error recovery
- Sleep

The controller reports back to Pi

- state of the controller
- position and speed (and current) of the two motors
- errors reported by the driver chip

# Pins wasted

The following pins of the msp430G2553 are misused for the purpose

Left side, top to down

| Pin# | Port | Function | Description     |
|------|------|----------|-----------------|
|    1 | DVCC | +3.3v    | Electrons exit  |
|    2 | P1.0 | IO       | in: L DIAGa     |
|    3 | P1.1 | UART RX  | data from Pi    |
|    4 | P1.2 | UART TX  | data to Pi      |
|    5 | P1.3 | IO       | in: L DIAGb     |
|    6 | P1.4 | IO       | in :L Encoder A |
|    7 | P1.5 | IO       | in :L Encoder B |
|    8 | P2.0 | IO       | out:L INA       |
|    9 | P2.1 | TA1.1    | out:L PWM       |
|   10 | P2.2 | IO       | out:L INB       |


Right Side, top to down

| Pin# | Port | Function | Description     |
|------|------|----------|-----------------|
|   20 | DVSS | +0v      | Electrons enter |
|   19 | P2.6 | IO       | in :R DIAGa     |
|   18 | P2.7 | IO       | in :R DIAGb     |
|   17 | x    | TEST     | Programmer      |
|   16 | x    | RESET    | Programmer      |
|   15 | P1.7 | IO       | in: R Encoder A |
|   14 | P1.6 | TA0.1    | in: R Encoder B |
|   13 | P2.5 | IO       | out:R INA       |
|   12 | P2.4 | IO       | out:R PWM       |
|   11 | P2.3 | IO       | out:R INB       |

The motor driver also provides two Current Sense (CS) analog outputs
that tell how many amps go through a motor. As you see there are no pins
left in this chip to handle that so I'll leave that out now.

# CPU settings

Low power operation (at which the msp430 is also good at) is not
within the scope. It runs full speed 16MHz from the DCO oscillator.

# Motor control signals to the driver

The controller will handle the confusion that left motor will turn
Clockwise (CW) and right CounterClockWise (CCW) when going forward. 
The Power settings received from the Pi are signed integers bounded
by the timer A1 TA1CCR0 count (PWM frequency).
Negative number means going backwards and positive forwards.

Three states of INA and INB motor driver pins are used 

| State         | L INA | L INB | R INA | R INB |
|---------------|-------|-------|-------|-------|
| Power(x) >= 0 |     1 |     0 |     0 |     1 |
| Power(x) < 0  |     0 |     1 |     1 |     0 |
| Braking       |     0 |     0 |     0 |     0 |

Braking state means shorting both motors to ground. With the 51:1 gears
this shouldn't really be necessary.

The PWM has 10KHz frequency with resolution of 4000 steps. 
This might be an overkill and perhaps even the 20KHz freq with
2000 steps would be too. The motor driver documentation says max 20KHz.

The driver operates at 5v and high level signal is 3.27v which is
uncomfortably close to the 3.3v of msp430 so level converters are needed.

The INA, INB pins in MSP need to be in the same port to be able to change
them simultaneously. Otherwise the motor will be in braking mode for
a short period.

Example:

~~~~
#define LINA BIT4 
#define LINB BIT3
P2OUT = (P2OUT&~LINB)|LINA; // Left motor forward in one op
P2OUT = (P2OUT&~LINA)|LINB; // Left motor backward
P2OUT &= ~(LINA|LINB);      // Left Brake
~~~~

# Encoder signals from the motors

*Speculation based on the vague docs, need to be verified with the hardware*

Hall effect quadrature encoder will give 13 interrupts per 
one motor rotation on the A input. The gearbox is 51:1 which makes it
663 interrupts per one rotation of the wheel.

At 16MHz it seems that by initial measurements
this MSP430 chip can handle 160K interrupts per second 
or 80K interrupts/s per wheel with highly unoptimized C code ISRs.
So the ISR is bounded with wheel speeds of 120r/s. The motor+gear documentation
mentions ~3 r/s for top speed which makes me believe this should not be a problem.

The accuracy looks pretty good when measured with the 2MHz wall clock
timer implemented with T0. Connecting the PWM output to encoder A input
gives variation of one step (0.5 microseconds).


