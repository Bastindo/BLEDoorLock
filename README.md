# BLE Door Lock

## What is this?

This is a Arduino Nano ESP32 based door lock, which controls a MG90S servo motor over Bluetooth Low Energy (BLE), built as part of the 2024 Project Week Hackathon of the FSR Elektrotechnik of the University of Rostock. This project includes user authentication and secure Bluetooth communication using the NimBLE stack.

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

To do so, follow this tutorial: https://docs.arduino.cc/tutorials/nano-esp32/spiff/

Or

```
pio run --target buildfs
pio run --target uploadfs
```

After that (or if you've done the filesystem part already) run `pio run -t upload` to flash the firmware image to your device.

## Usage

### Hardware setup servo

Connect your servo PWM pin (yellow/orange wire) to the D2 pin on the Arduino. This pin can be configured in `src/config.h`. The servo also needs connections to 5V VCC and GND. You can use the VBUS and GND pins on the Arduino for this.

### Hardware setup relay

Connect your relay s pin to the D2 pin on the Arduino. This pin can be configured in `src/config.h`. The relay also needs connections to 3.3V or 5V VCC (see your relay specification) and GND. You can use the 3.3V or VBUS and GND pins on the Arduino for this.

### BLE

You can use the "nRF Connect" app on your phone to talk to the door lock. To pair the device, you will need a PIN. The default PIN is 123456, but it can be changed in `src/config.h`.

The firmware provides two BLE services, one for opening/closing the lock and one for adding new users.

The first service has three characteristics: Username, Password and Lockstate.
Just send your username and password to the first two characteristics and your lockstate to the third one to interact with the door lock as a user.

The lockstate can be:

- 0 for closing the door
- 1 for opening the door
- 2 for opening the door for a short period of time (default: 10 seconds)

The second service can be used to add or remove user accounts and provides five characteristics: Admin Username, Admin Password, Username, Password and Admin Action.

The admin action can be:

- 0 for removing a user
- 1 for adding a new user (make sure to set a password)

### BLE Examples

To get the actual UUIDs of the characteristics shown in these examples see `src/config.h`.

To authorize and open the door for 10 seconds send the following values to the following characteristics (in order):

```
Username -> "your_username"
Password -> "your_password1234"
Lockstate -> "2"
```

To authorize as an admin and add a new user:

```
AdminUsername -> "your_admin_name"
AdminPassword -> "your_admin_pw1234"
Username -> "user_to_be_created9876"
Password -> "new_user_pw9876"
AdminAction -> "1"
```

To authorize as an admin and remove a user (note that you don't need to specify a password):

```
AdminUsername -> "your_admin_name"
AdminPassword -> "your_admin_pw1234"
Username -> "user_to_be_removed9876"
AdminAction -> "0"
```

Admin accounts can be added via serial. See below for an example.

### Serial

The firmware provides a command shell and a log on the USB serial interface.
This can be used to add or remove admin or user accounts and debug various stuff.
Type `help` in the serial console to see a list of commands.

### [Serialscripts](serialscripts/README.md)

Scripts to manage users, and generate QR-Codes for the [APP](https://github.com/UwUTastisch/BLEDoorLock-App).

### Serial examples

Add a new admin account:

```
addadmin new_admin_name123 password1234
```

Add a new user:

```
adduser new_user123 password1234
```

Remove an admin account:

```
removeadmin admin_name123
```

List all users:

```
cat users.csv
```

List all admins:

```
cat admins.csv
```

### Configuration

Configuration can be done by editing `src/config.h`.
