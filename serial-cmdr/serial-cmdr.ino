#include <ArduinoJson.h>
#include <Servo.h>
#include <LiquidCrystal.h>

// configuration settings
int SERVO_MIN_ANGLE_DEG = 0;
int SERVO_MAX_ANGLE_DEG = 180;
int SERVO_SWEEP_DELAY_MS = 1; // milliseconds between position steps, -1 for asap

// hardware assignments
const int LCD_RS = 7, LCD_EN = 6, LCD_D4 = 5, LCD_D5 = 4, LCD_D6 = 3, LCD_D7 = 2;
const int SERVO_PWM_PIN = 10;
const int SONAR_TRIGGER_PIN = 11;
const int SONAR_RESPONSE_PIN = 12;

// initialize hardware drivers
Servo servo;
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// set up command data structure
struct command
{
    String raw = "";
    float servo_position_deg = -1;
};

// set up telemetry data structure (global)
struct telemetry
{
    String status;
    command cmd;
    float pos = -1;
    float sense = -1;
    void printToSerial()
    {
        StaticJsonDocument<200> outDoc;
        outDoc["status"] = status;
        outDoc["command"] = cmd.raw;
        outDoc["position"] = pos;
        outDoc["sensor"] = sense;
        serializeJson(outDoc, Serial);
        Serial.println();
        return;
    };
    void printToLcd()
    {
        // Print telemetry to LCD screen
        lcd.begin(16, 2);
        lcd.print(status);
        lcd.setCursor(0, 1);
        lcd.print((int)cmd.servo_position_deg);
        lcd.setCursor(4, 1);
        lcd.print((int)pos);
        lcd.setCursor(8, 1);
        lcd.print(sense);
        return;
    };
} telem;

void setup()
{
    // Initialize Serial Port
    Serial.begin(115200);
    Serial.setTimeout(1);
    //Define inputs and outputs
    pinMode(SONAR_TRIGGER_PIN, OUTPUT);
    pinMode(SONAR_RESPONSE_PIN, INPUT);
    servo.attach(SERVO_PWM_PIN);
    // digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW

    // wave hello
    telem.status = "initializing";
    telem.sense = readSonarCm(SONAR_TRIGGER_PIN, SONAR_RESPONSE_PIN);
    moveServo(SERVO_MAX_ANGLE_DEG);
    moveServo(SERVO_MIN_ANGLE_DEG);
}

void loop()
{
    command cmd;
    telem.cmd = cmd;
    telem.status = "idle";
    telem.sense = readSonarCm(SONAR_TRIGGER_PIN, SONAR_RESPONSE_PIN);
    if (Serial.available() > 0)
    {
        // Read command from serial input
        String cmdString = Serial.readString();
        cmd = parseCommand(cmdString);
        telem.cmd = cmd;
    }
    telem.printToSerial();
    telem.printToLcd();

    if ((cmd.servo_position_deg != telem.pos) && (cmd.servo_position_deg > 0))
    {
        // move the servo
        moveServo(cmd.servo_position_deg);
    }
    delay(500);
}

command parseCommand(String cmdString)
{
    DynamicJsonDocument inDoc(200);
    deserializeJson(inDoc, cmdString);
    // construct command data structure
    command cmd;
    cmd.servo_position_deg = (float)inDoc["servo_position"];

    // add the interpreted command for debugging
    String parsedCmdString;
    serializeJson(inDoc, parsedCmdString);
    cmd.raw = parsedCmdString;
    return cmd;
}

float readSonarCm(int SONAR_TRIGGER_PIN, int SONAR_RESPONSE_PIN)
{
    /*
        Ultrasonic sensor Pins:
            VCC: +5VDC
            Trig : Trigger (INPUT) 
            Echo: Echo (OUTPUT) 
            GND: GND
    */
    float duration, cm;
    // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
    // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
    digitalWrite(SONAR_TRIGGER_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(SONAR_TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(SONAR_TRIGGER_PIN, LOW);

    // Read the signal from the sensor: a HIGH pulse whose
    // duration is the time (in microseconds) from the sending
    // of the ping to the reception of its echo off of an object.
    pinMode(SONAR_RESPONSE_PIN, INPUT);
    duration = pulseIn(SONAR_RESPONSE_PIN, HIGH);

    // Convert the time into a distance
    cm = (duration / 2) / 29.1; // Divide by 29.1 or multiply by 0.0343
    // inches = (duration / 2) / 74; // Divide by 74 or multiply by 0.0135
    return cm;
}

float clamp(float input, float minLimit, float maxLimit)
{
    // clamp the input angle
    if (input < minLimit)
    {
        input = minLimit;
    }
    else if (input > maxLimit)
    {
        input = maxLimit;
    }
    return input;
}

void moveServo(float cmd_pos)
{
    /*
        command the servo to the desired position
    */
    // limit the command to the allowed range
    float angle = telem.pos;
    cmd_pos = clamp(cmd_pos, SERVO_MIN_ANGLE_DEG, SERVO_MAX_ANGLE_DEG);
    telem.cmd.servo_position_deg = cmd_pos;

    if (SERVO_SWEEP_DELAY_MS > 0)
    {
        // sweep servo position to the desired
        if (cmd_pos > angle)
        {
            for (angle; angle <= cmd_pos; angle++)
            {
                telem.status = "moving ccw";
                telem.pos = angle;
                servo.write(angle);
                telem.sense = readSonarCm(SONAR_TRIGGER_PIN, SONAR_RESPONSE_PIN);
                telem.printToSerial();
                telem.printToLcd();
                delay(SERVO_SWEEP_DELAY_MS);
            }
        }
        else if (cmd_pos < angle)
        {
            for (angle; angle >= cmd_pos; angle--)
            {
                telem.status = "moving cw";
                telem.pos = angle;
                telem.sense = readSonarCm(SONAR_TRIGGER_PIN, SONAR_RESPONSE_PIN);
                servo.write(angle);
                telem.printToSerial();
                telem.printToLcd();
                delay(SERVO_SWEEP_DELAY_MS);
            }
        }
    }
    else
    {
        telem.pos = cmd_pos;
        servo.write(cmd_pos);
        telem.sense = readSonarCm(SONAR_TRIGGER_PIN, SONAR_RESPONSE_PIN);
        telem.printToSerial();
        telem.printToLcd();
    }
    return;
}
