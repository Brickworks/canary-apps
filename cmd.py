import serial
import time

arduino = serial.Serial(port='COM3', baudrate=115200, timeout=.1)
def write_read(x):
    arduino.write(bytes(x, 'utf-8'))
    time.sleep(1)
    data = arduino.readline()
    return data

while True:
    input_text = input("Enter text to send over Serial: ") # Taking input from user
    value = write_read(input_text)
    print(value) # printing the value
