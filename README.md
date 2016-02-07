## Objective

The purpose of this pile of rubbish is to build a controller between
Raspberry Pi UART (or SPI) and Pololu 708 (Dual VNH2SP30 Motor Driver
Carrier MD03A) with two DC gear motors (FIT0277).

The controller is based on msp430G2553 (20-pin) because I have several of them.
Besides that, I can implement sufficiently fast two channel PWM (10-20KHz)
(Timer A0,A1), UART and a clock (Timer B) to measure time between
interrupts from the rotary encoders of the motors. Most of this with the
embedded hardware peripherals of the chip.

The code is designed to control exactly this configuration.
So peripheral pin configuration is configured in the code.
Clock speeds and other


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
|    2 | P1.0 | IO       | in :R Encoder A |
|    3 | P1.1 | UART RX  | data from Pi    |
|    4 | P1.2 | UART TX  | data to Pi      |
|    5 | P1.3 | IO       | in :R Encoder B |
|    6 | P1.4 | IO       | out:R INA       |
|    7 | P1.5 | IO       | out:R INB       |
|    8 | P2.0 | IO       | in: R DIAGa     |
|    9 | P2.1 | TA0.1    | out:R PWM       |
|   10 | P2.2 | IO       | in: R DIAGb     |


Right Side, top to down

| Pin# | Port | Function | Description     |
|------+------+----------+-----------------|
|   20 | DVSS | +0v      | Electrons enter |
|   19 | P2.6 | IO       | in: L Encoder A |
|   18 | P2.7 | IO       | in: L Encoder B |
|   17 | x    | TEST     | Programmer      |
|   16 | x    | RESET    | Programmer      |
|   15 | P1.7 | IO       | in :L DIAGa     |
|   14 | P1.6 | TA0.1    | out:L PWM       |
|   13 | P2.5 | IO       | in :L DIAGb     |
|   12 | P2.4 | IO       | out:L INA       |
|   11 | P2.3 | IO       | out:L INB       |

The motor driver also provides two Current Sense (CS) analog outputs
that tell how many amps go through a motor. As you see there are no pins
left in this chip to handle that so I'll leave that out now.

# CPU settings

Low power operation (at which the msp430 is also good at) is not
within the scope. It runs 16MHz from the DCO oscillator.

# Motor control signals to the driver

The controller will handle the confusion that left motor will turn
Clockwise (CW) and right CounterClockWise (CCW) when going forward. 
The Power settings
received from the Pi are signed integers between -4000 and 4000.
Negative number means going backwards and positive forwards.

Three states of INA and INB motor driver pins are used 

| State         | L INA | L INB | R INA | R INB |
|---------------+-------+-------+-------+-------|
| Power(x) >= 0 |     1 |     0 |     0 |     1 |
| Power(x) < 0  |     0 |     1 |     1 |     0 |
| Braking       |     0 |     0 |     0 |     0 |

Braking state means shorting both motors to ground. With the 51:1 gears
this shouldn't really be necessary.

The PWM has 10KHz frequency with resolution of 4000 steps. 
This might be an overkill and perhaps even the 20KHz freq with
2000 steps would be too.

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

*Speculation based on the vague docs, need to be verified*

Hall effect quadrature encoder will give 13 pulses per 
one motor rotation per channel. 
