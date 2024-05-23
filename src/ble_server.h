#define UUID_SERVICE BLEUUID((uint16_t)0x183d)  // 0x183d: Authorization Control Service
#define UUID_CHARACTERISTIC "48cdff87-6063-4144-8e90-203741e90344"
#define UUID_USER_CHARACTERISTIC "5d3932fa-2901-4b6b-9f41-7720976a85d4"
#define UUID_PASS_CHARACTERISTIC "dd16cad0-a66a-402f-9183-201c20753647"
#define UUID_LOCKSTATE_CHARACTERISTIC "05c5653a-7279-406c-9f9e-df72aa99ca2d"
#define SHORT_UNLOCK_TIME 10000 // ms
#define BLE_PIN 123456 // "Pre-Shared Key"-like in a Multi-User Environment

void setupBLE();