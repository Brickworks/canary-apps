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
// an exeample telemetry string
{"status": "idle", "commandAngle_deg": -1.00, "sonarDistance_cm": 9.54, "servoAngle_deg": 45.00}
```

|key|type|description|
|---|---|---|
|`status`|string|What the microcontroller is working on at the moment.|
|`commandAngle_deg`|float|The currently commanded servo position (in degrees). `-1` indicates no command right now.|
|`sonarDistance_cm`|float|The last distance measurement from the HC-SR04 (in centimeters). Large numbers usually mean loss of signal, since the measurement is based on the time-of-flight of an active "ping".|
|`servoAngle_deg`|float|The position (in degrees) that the microcontroller thinks the servo is at.|

After initialization, the microcontroller waits for data to be available on
the serial port. All data sent to the serial port is assumed to be a string
that can be parsed as a float. When data is available, the text is parsed as
a float and clamped to a value between 10 and 180, which are the position 
limits of the servo.

The servo is commanded to move to a new angular position smoothly in 1 degree
increments, with a short delay in between. By changing this delay time, the
speed of the servo can be controlled. It's hard-coded for now. A new telemetry
packet is emitted at every step of the servo.

After reaching the commanded position, the microcontroller resumes the main
execution loop until there is more data available to be read on the serial bus.

## cmd.py
Boilerplate Python script for sending data over the serial port and reading
a response.
