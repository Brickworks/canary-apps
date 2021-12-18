#include <Servo.h>
#include <LiquidCrystal.h>

// global variables
String status;
float cmd_pos;
float servo_pos;

// configuration settings
int SERVO_MIN_ANGLE_DEG = 0;
int SERVO_MAX_ANGLE_DEG = 180;
int SERVO_SWEEP_DELAY_MS = 15; // milliseconds between position steps

// hardware assignments
const int LCD_RS = 7, LCD_EN = 6, LCD_D4 = 5, LCD_D5 = 4, LCD_D6 = 3, LCD_D7 = 2;
const int SERVO_PWM_PIN = 10;
const int SONAR_TRIGGER_PIN = 11;
const int SONAR_RESPONSE_PIN = 12;

// initialize hardware drivers
Servo servo;
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

void setup()
{
    // Initialize Serial Port
    Serial.begin(115200);
    Serial.setTimeout(1);
    //Define inputs and outputs
    pinMode(SONAR_TRIGGER_PIN, OUTPUT);
    pinMode(SONAR_RESPONSE_PIN, INPUT);
    servo.attach(SERVO_PWM_PIN);

    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW

    // wave hello
    makeTelemJson("initializing", -1, -1, readSonarCm(SONAR_TRIGGER_PIN, SONAR_RESPONSE_PIN));
    moveServo(SERVO_MAX_ANGLE_DEG);
    moveServo(SERVO_MIN_ANGLE_DEG);
}

void loop()
{
    status = "idle";
    cmd_pos = -1; // none
    makeTelemJson(status, servo_pos, cmd_pos, readSonarCm(SONAR_TRIGGER_PIN, SONAR_RESPONSE_PIN));
    if (Serial.available() > 0)
    {
        // Read command from serial input
        cmd_pos = Serial.readString().toFloat();
        moveServo(cmd_pos);
    }
    delay(500);
}

int clamp(float input, float minLimit, float maxLimit)
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

void printToLcd(float cmdAngle, float sonarDistance, float angle, String status)
{
    // Print telemetry to LCD screen
    lcd.begin(16,2);
    lcd.print(status);
    lcd.setCursor(0,1);
    lcd.print((int) cmdAngle);
    lcd.setCursor(4,1);
    lcd.print((int) angle);
    lcd.setCursor(8,1);
    lcd.print(sonarDistance);
}

void makeTelemJson(String status, float angle, float cmdAngle, float sonarDistance)
{
    // Print a JSON-like string to the serial bus
    Serial.print("{\"status\": \"");
    Serial.print(status);
    Serial.print('\"');
    Serial.print(", \"commandAngle_deg\": ");
    Serial.print(cmdAngle);
    Serial.print(", \"sonarDistance_cm\": ");
    Serial.print(sonarDistance);
    Serial.print(", \"servoAngle_deg\": ");
    Serial.print(angle);
    Serial.print("}");
    Serial.println();

    // also print telemetry to the LCD
    printToLcd(cmdAngle, sonarDistance, angle, status);
    return;
}

float moveServo(float cmd_pos)
{
    /*
        command the servo to the desired position
    */
    float angle = servo_pos;
    // limit the command to the allowed range
    cmd_pos = clamp(cmd_pos, SERVO_MIN_ANGLE_DEG, SERVO_MAX_ANGLE_DEG);

    // sweep servo position to the desired
    if (cmd_pos > servo_pos)
    {
        for (angle = servo_pos; angle <= cmd_pos; angle++)
        {
            status = "moving cw";
            servo_pos = angle;
            servo.write(servo_pos);
            makeTelemJson(status, servo_pos, cmd_pos, readSonarCm(SONAR_TRIGGER_PIN, SONAR_RESPONSE_PIN));
            delay(SERVO_SWEEP_DELAY_MS);
        }
    }
    else if (cmd_pos < servo_pos)
    {
        for (angle = servo_pos; angle >= cmd_pos; angle--)
        {
            status = "moving ccw";
            digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
            servo_pos = angle;
            servo.write(servo_pos);
            makeTelemJson(status, servo_pos, cmd_pos, readSonarCm(SONAR_TRIGGER_PIN, SONAR_RESPONSE_PIN));
            delay(SERVO_SWEEP_DELAY_MS);
        }
    }
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    return;
}