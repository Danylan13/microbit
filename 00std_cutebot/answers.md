# Lab 09a: Cutebot Answers

## 1. What communication protocol does the micro:bit use to send commands to the Cutebot? How does it work?

The micro:bit uses `I2C` to send commands to the Cutebot.

How it works:

- `I2C` uses two signal lines: `SDA` for data and `SCL` for the clock.
- The micro:bit acts as the controller on the bus and starts each transmission.
- The Cutebot controller listens on I2C address `0x10`.
- Commands are sent as small byte buffers. For the standard Cutebot motor control, the buffer format is:
  - motor selector
  - direction
  - speed
  - padding byte
- The controller receives those bytes and drives the left or right motor accordingly.

In this repository, `00std_cutebot.c` configures TWI/I2C on the micro:bit pins and writes those command buffers directly to the robot.

## 2. What sensors does the Cutebot have on board? What would you need to do to avoid obstacles?

The Cutebot has:

- an ultrasonic distance sensor
- two line-tracking sensors

To avoid obstacles, the program should:

1. read the ultrasonic sensor continuously
2. compare the measured distance to a safety threshold
3. stop if an object is too close
4. reverse a little or turn left/right
5. measure again and continue only when the path is clear

A simple rule would be:

- if distance `< 15 cm`, stop and turn
- otherwise, keep moving forward

## Sources

- ELECFREAKS Cutebot package: https://github.com/elecfreaks/pxt-cutebot
- micro:bit I2C specification: https://tech.microbit.org/software/spec-i2c-protocol/
