#include <Servo.h>

String status;
int cmd_pos;
int angle;
int home = 10;
int maxAngle = 180;

int trigPin = 11; // Trigger
int echoPin = 12; // Echo
long duration, cm, inches;
Servo servo;

void setup()
{
    //Serial Port begin
    Serial.begin(115200);
    Serial.setTimeout(1);
    //Define inputs and outputs
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    //Servo setup
    servo.attach(10);
    servo.write(10);

    // wave hello
    cmd_pos = -1; // none
    makeTelemJson(cmd_pos, readSonarCm(trigPin, echoPin), -1, "hello");
    moveServo(180);
    makeTelemJson(cmd_pos, readSonarCm(trigPin, echoPin), -1, "world");
}

void loop()
{
    status = "idle";
    angle = home;   // home
    cmd_pos = -1; // none
    makeTelemJson(cmd_pos, readSonarCm(trigPin, echoPin), angle, status);
    if (Serial.available()>0)
    {
        // Read command from serial input
        cmd_pos = Serial.readString().toInt();
        // clamp the input angle
        if (cmd_pos < 10)
        {
            cmd_pos = 10;
        }
        else if (cmd_pos > 180)
        {
            cmd_pos = 180;
        }
        moveServo(cmd_pos);
    }
    delay(500);
}

long readSonarCm(int trigPin, int echoPin) {
    /*
    * created by Rui Santos, https://randomnerdtutorials.com
    * 
    * Complete Guide for Ultrasonic Sensor HC-SR04
    *
        Ultrasonic sensor Pins:
            VCC: +5VDC
            Trig : Trigger (INPUT) - Pin11
            Echo: Echo (OUTPUT) - Pin 12
            GND: GND
    */
    // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
    // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
    digitalWrite(trigPin, LOW);
    delayMicroseconds(5);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Read the signal from the sensor: a HIGH pulse whose
    // duration is the time (in microseconds) from the sending
    // of the ping to the reception of its echo off of an object.
    pinMode(echoPin, INPUT);
    duration = pulseIn(echoPin, HIGH);

    // Convert the time into a distance
    cm = (duration / 2) / 29.1;   // Divide by 29.1 or multiply by 0.0343
    // inches = (duration / 2) / 74; // Divide by 74 or multiply by 0.0135
    return cm;
}

void makeTelemJson(int cmdAngle, long sonarDistance, int angle, String status) {
    Serial.print("{\"commandAngle_deg\": ");
    Serial.print(cmdAngle);
    Serial.print(", \"sonarDistance_cm\": ");
    Serial.print(sonarDistance);
    Serial.print(", \"servoAngle_deg\": ");
    Serial.print(angle);
    Serial.print(", \"status\": ");
    Serial.print(status);
    Serial.print('\"');
    Serial.print("}");
    Serial.println();
    return;
}

void moveServo(int cmd_pos) {
    // scan from 0 to 180 degrees
    for (angle = home; angle < cmd_pos; angle++)
    {
        status = "moving to position";
        servo.write(angle);
        makeTelemJson(cmd_pos, readSonarCm(trigPin, echoPin), angle, status);
        delay(15);
    }
    // now scan back from 180 to 0 degrees
    for (angle = cmd_pos; angle > home; angle--)
    {
        status = "moving home";
        servo.write(angle);
        makeTelemJson(cmd_pos, readSonarCm(trigPin, echoPin), angle, status);
        delay(15);
    }
    return;
}