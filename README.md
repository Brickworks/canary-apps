# canary-apps
Tools and apps to support development of [mfc-apps](https://github.com/Brickworks/mfc-apps)

## serial-cmdr
This Arduino project iniitalizes an Arduino microcontroller with a simple
sensor and actuator, and demos sending telemetry and recieving commands over
the serial port.

### Hardware
This demo was prepared for an Arduino Uno with the following peripherals:
* HC-SR04 Ultrasonic Rangefinder (Pins 11 & 12)
* SG90 MicroServo (Pin 10)
* LCD1602 LCD Display (Pins 2-7)

### Theory of Operation
The microcontroller initializes the pins for the peripherals, then
"waves hello" with the servo and begins reporting telemetry on the serial port.

Telemetry is transmitted as JSON-like strings (here I'll call them "packets") 
sent over the serial port. A packet is sent every 0.5 seconds while the
servo is idle, and every 0.015 seconds while the servo is moving. The telemetry
is also displayed on the LCD screen.

```json
// exeample telemetry strings
{"status":"idle","command":"","position":0,"sensor":31.66667}
{"status":"moving ccw","command":"{\"servo_position\":135}","position":85,"sensor":33.05842}
```

After initialization, the microcontroller waits for data to be available on
the serial port. All data sent to the serial port is assumed to be a string
that can be parsed as a JSON. When data is available, the text is parsed as
a JSON. If the JSON has a key called `servo_position`, that value is casted to
a float and clamped to a value between 0 and 180, which are the position 
limits of the servo.

The servo is commanded to move to `servo_position` smoothly in 1 degree
increments, with a short delay in between. By changing this delay time, the
speed of the servo can be controlled. It's hard-coded for now. A new telemetry
packet is emitted at every step of the servo.

After reaching the commanded position, the microcontroller resumes the main
execution loop until there is more data available to be read on the serial bus.

## cmd.py
Boilerplate Python script for sending data over the serial port and reading
a response.
