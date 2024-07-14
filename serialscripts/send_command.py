import serial
import time
from dotenv import load_dotenv
import os

load_dotenv()

# Configuration
SERIAL_PORT = os.getenv('SERIAL_PORT', '/dev/ttyACM0')  # Replace with your actual serial port
BAUD_RATE = int(os.getenv('BAUD_RATE', '115200'))  # Replace with your actual baud rate
DEBUG = os.getenv('DEBUG', 'false').lower() == 'true'
PING_CMD = "ping"
EXPECTED_PING_RESPONSE = "pong"

def check_connection():
    print("Checking connection...")
    response = send_command(PING_CMD, 0.3)
    if DEBUG:
        print(f"Response: {response}")
    if response and EXPECTED_PING_RESPONSE in response:
        print("Connection is working properly.")
    else:
        print("Connection failed. No 'pong' response.")
        exit(1)

def send_command(command, timeout_duration=5):
    if(command != PING_CMD):
        check_connection()
    try:
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=timeout_duration) as ser:
            print(f"Serial port configured: {SERIAL_PORT} at {BAUD_RATE} baud rate.")
            
            # Flush the serial port
            ser.reset_input_buffer()
            ser.reset_output_buffer()
            print("Flushed the serial port.")

            # Send the command
            ser.write(f"{command}\r".encode())
            if DEBUG:
                print(f"Command '{command}' sent to serial port.")
            
            # Read the response
            response = b''
            start_time = time.time()
            while (time.time() - start_time) < timeout_duration:
                if ser.in_waiting > 0:
                    response += ser.read(ser.in_waiting)
                time.sleep(0.1)

            response = response.decode().strip()
            if DEBUG:
                print(f"Response from serial port: {response}")

            return response
    except serial.SerialException as e:
        print(f"Error communicating with serial port: {e}")
        return None

