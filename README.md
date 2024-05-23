# BLE Door Lock
## What is this?
This is a Arduino Nano ESP32 based door lock, which controls a MG90S servo motor over Bluetooth Low Energy (BLE), built as part of the 2024 Project Week Hackathon by the FSR Elektrotechnik of the University of Rostock. This project includes user authentication and secure Bluetooth communication using the NimBLE stack.

## Building
This project uses PlatformIO.

You can just use the VSCode extension, but the project can also be built with the PlatformIO CLI tool.

To install it, use pip:
`pip install -U platformio`

Then follow these instructions:
1. Clone the repository and cd into it:
```
git clone https://github.com/Bastindo/HackathonBLE
cd HackathonBLE
```
2. Build a firmware image with this command:
```
pio run
```
## Installing
If you never installed this project on your device, you need to prepare the board for filesystem creation. You only need to do this once.

To do so, follow this tutorial: [https://docs.arduino.cc/tutorials/nano-esp32/spiff/]

After that (or if you've done the filesystem part already) run `pio run -t upload` to flash the firmware image to your device.

## Usage
### Hardware setup
Connect your servo PWM pin (yellow/orange wire) to the D2 pin on the Arduino. The servo also needs connections to 5V VCC and GND. You can use the VBUS and GND pins on the Arduino for this.
### BLE
You can use the "nRF Connect" app on your phone to talk to the door lock. To pair the device, you will need a PIN. The default PIN is 123456.

The firmware provides two BLE services, one for opening/closing the lock and one for adding new users.

The first service has three characteristics: Username, Password and Lockstate.
Just send your username and password to the first two characteristics and your lockstate to the third one to interact with the door lock as a user.

The lockstate can be:
- 0 for opening the door
- 1 for closing the door
- 2 for opening the door for a short period of time (default: 10 seconds)

The second service can be used to add new user accounts and provides five characteristics: Admin Username, Admin Password, Username, Password and Admin Action.

### Serial
The firmware provides a command shell and a log on the USB serial interface.
This can be used to add new admin or user accounts and debug various stuff.

### Configuration
Configuration can be done through various defines in the header files in `src/`.
