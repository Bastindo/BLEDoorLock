#ifndef CONFIG_H
#define CONFIG_H

/* ######## Debug Mode ######## */
//
// Debug mode waits for serial connection on boot and provides additional serial
// commands (open, close, shortopen)
#define DEBUG_MODE 1
//

/* ######## Logging ######## */
//
// Log level (possible options: LOG_LEVEL_{SILENT, FATAL, ERROR, WARNING, INFO,
// TRACE, VERBOSE}, use ERROR if you only want to log user authentications)
#define LOG_LEVEL LOG_LEVEL_VERBOSE
//

/* ######## Door Opener ######## */
//
// Connect servo PWM line to the GPIO pin configured here (2 usually corresponds
// to D2) new pin is D13
#define SERVO_PIN 2
#define SERVO_OPEN_ANGLE 80   // angle to open the door
#define SERVO_CLOSE_ANGLE 90  // angle to close the door (Default state)
//
// Connect relay control line to the GPIO pin configured here (13 usually
// corresponds to D13)
#define RELAY_PIN 13
//

/* ######## Wifi ######## */
// set to 1 to activate WiFi
#define WIFI 0

/* ######## WiFi Client ######## */
// network credentials
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

/* ######## HTTP logging ######## */
#define HTTP_TARGET1 192  // IP of the target for logging
#define HTTP_TARGET2 168
#define HTTP_TARGET3 178
#define HTTP_TARGET4 236

#define HTTP_PORT 64000  // Port of the target for logging

/* ######## BLE server ######## */
// BLE name
#define BLE_NAME "NimBLE Servo Lock 4"
//
// time interval for the Short Unlock function
#define SHORT_UNLOCK_TIME 3000  // ms
//
// Logout time in ms
#define LOGOUT_TIME 12000
//
// UUIDs (usually shouldn't be changed)
#define UUID_USER_SERVICE "2ff7c135-5010-497b-a054-cea3984c7cc9"
#define UUID_ADMIN_SERVICE "be527357-c722-4367-aac3-bddef6a6f6e2"
#define UUID_CRYPTO_SERVICE "1c970e06-8094-4b83-a54b-a465396ebaa8"
//
#define UUID_USER_CHARACTERISTIC "5d3932fa-2901-4b6b-9f41-7720976a85d4"
#define UUID_PASS_CHARACTERISTIC "dd16cad0-a66a-402f-9183-201c20753647"
#define UUID_LOCKSTATE_CHARACTERISTIC "05c5653a-7279-406c-9f9e-df72aa99ca2d"
//
#define UUID_ADMIN_CHARACTERISTIC "68f2b041-dc1e-42af-af96-773a2386b08b"
#define UUID_ADMINPASS_CHARACTERISTIC "394e8790-109b-47c0-aa67-1aa61c02188b"
#define UUID_ADDUSER_CHARACTERISTIC "92acb83b-ff02-43ec-9adb-16755eb8ce9b"
#define UUID_ADDPASS_CHARACTERISTIC "8de8c0c0-0568-40a0-a52b-520a6e772503"
#define UUID_ADMINACTION_CHARACTERISTIC "b1d86fdf-7d5d-49b7-8da7-b02bd53bdb0a"
//
#define UUID_KEY_CHARACTERISTIC "df5ba2aa-c90c-4c90-8c5f-059f62ff51a1"

/* ######## CMD ######## */
//
// usually shouldn't be changed
#define CMD_BUFFER_SIZE 256
//

#define VALID_USERNAME_CHARACTER \
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"

/* ######## LockState ######## */

// Enum for the lock state
// do not change the values of the enum!
enum LockState { LOCKED = 0, UNLOCKED = 1, SHORT_UNLOCK = 2 };

#endif
