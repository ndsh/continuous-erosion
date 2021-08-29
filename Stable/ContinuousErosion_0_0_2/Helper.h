// Include libraries
#include <ESP8266WiFi.h>
#include <TMC2130Stepper.h>
#include <AccelStepper.h>
#include "Structures.h"

// Pin definitions

// Variables

// Read-only
//extern const uint16_t gamma8[];
#define disable 0
#define enable  1
#define EN_PIN    D4  // Nano v3:   16 Mega:    38  //enable (CFG6)
#define DIR_PIN   D2  //            19          55  //direction
#define STEP_PIN  D3  //            18          54  //step
#define CS_PIN    D5  //            17          40  //chip select
#define BUTTON1   D1  // linker stopp
#define BUTTON2   D0  // rechter stopp

#define CALIBRATE 0
#define PROMISC 1
#define RESET 2
#define SCAN 3
#define CENTER 4


// Global
unsigned int channel = 1;
uint32_t steps_per_mm = 80;
bool dir = true;
int stepCounter = 0;
int steps = 0;
int buttonState1 = 0;
int buttonState2 = 0; 
int state = 0;
int averageSignalStrength = 0;
int calcRSSI = 0;
int rssiLimit = -85; // 10 ^ ((-69 - (-70))/(10 * 2))p = 75 == 3.5m 
boolean debug = false;
bool leftHit = false;
bool rightHit = false;
boolean setupDone = false;

// Millis area
unsigned long timestamp;
unsigned long interval;
unsigned long grindTime = 120000;
unsigned long pauseTime = 30000;

unsigned long lastButtonTimestamp = 0;
unsigned long maxInterval = 10000;

// Objects
TMC2130Stepper driver = TMC2130Stepper(EN_PIN, DIR_PIN, STEP_PIN, CS_PIN);
AccelStepper stepper = AccelStepper(stepper.DRIVER, STEP_PIN, DIR_PIN);

// Arrays


// Functions
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void teensyReset() { // for a software restart of teensy
  #ifdef TEENSYDUINO
   SCB_AIRCR = 0x05FA0004;
//   WRITE_RESTART(0x5FA0004);
  #endif
}



// Expose Espressif SDK functionality
extern "C" {
#include "user_interface.h"
  typedef void (*freedom_outside_cb_t)(uint8 status);
  int  wifi_register_send_pkt_freedom_cb(freedom_outside_cb_t cb);
  void wifi_unregister_send_pkt_freedom_cb(void);
  int  wifi_send_pkt_freedom(uint8 *buf, int len, bool sys_seq);
}



#define MAX_APS_TRACKED 50
#define MAX_CLIENTS_TRACKED 50

beaconinfo aps_known[MAX_APS_TRACKED];                    // Array to save MACs of known APs
int aps_known_count = 0;                                  // Number of known APs
int nothing_new = 0;
clientinfo clients_known[MAX_CLIENTS_TRACKED];            // Array to save MACs of known CLIENTs
int clients_known_count = 0;                              // Number of known CLIENTs

int register_beacon(beaconinfo beacon) {
  int known = 0;   // Clear known flag
  for (int u = 0; u < aps_known_count; u++) {
    if (! memcmp(aps_known[u].bssid, beacon.bssid, ETH_MAC_LEN)) {
      known = 1;
      break;
    }   // AP known => Set known flag
  }
  if (! known && beacon.rssi > rssiLimit)  // AP is NEW, copy MAC to array and return it
  {
    memcpy(&aps_known[aps_known_count], &beacon, sizeof(beacon));
    aps_known_count++;

    if ((unsigned int) aps_known_count >=
        sizeof (aps_known) / sizeof (aps_known[0]) ) {
      Serial.printf("exceeded max aps_known\n");
      aps_known_count = 0;
    }
  }
  return known;
} // register_beacon

int register_client(clientinfo ci) {
  int known = 0;   // Clear known flag
  for (int u = 0; u < clients_known_count; u++) {
    if (! memcmp(clients_known[u].station, ci.station, ETH_MAC_LEN)) {
      known = 1;
      break;
    }
  }
  if (! known && ci.rssi > rssiLimit) {
    memcpy(&clients_known[clients_known_count], &ci, sizeof(ci));
    clients_known_count++;

    if ((unsigned int) clients_known_count >=
        sizeof (clients_known) / sizeof (clients_known[0]) ) {
      Serial.printf("exceeded max clients_known\n");
      clients_known_count = 0;
    }
  }
  return known;
} // register_client

void print_beacon(beaconinfo beacon) {
  if (beacon.err != 0) {
    //Serial.printf("BEACON ERR: (%d)  ", beacon.err);
  } else {
    if(beacon.rssi > rssiLimit) {
      if(debug) Serial.printf("BEACON: <=============== [%32s]  ", beacon.ssid);
      for (int i = 0; i < 6; i++) if(debug) Serial.printf("%02x", beacon.bssid[i]);
      if(debug) Serial.printf("   %2d", beacon.channel);
      if(debug) Serial.printf("   %4d\r\n", beacon.rssi);
      averageSignalStrength += beacon.rssi*-1;
    }
  }
}

void print_client(clientinfo ci) {
  int u = 0;
  int known = 0;   // Clear known flag
  if (ci.err != 0) {
    // nothing
  } else {
    if(debug) Serial.printf("DEVICE: ");
    for (int i = 0; i < 6; i++) if(debug) Serial.printf("%02x", ci.station[i]);
    if(debug) Serial.printf(" ==> ");

    for (u = 0; u < aps_known_count; u++) {
      if (! memcmp(aps_known[u].bssid, ci.bssid, ETH_MAC_LEN)) {
        if(debug) Serial.printf("[%32s]", aps_known[u].ssid);
        known = 1;     // AP known => Set known flag
        break;
      }
    }

    if(!known)  {
      if(debug) Serial.printf("   Unknown/Malformed packet \r\n");
      //  for (int i = 0; i < 6; i++) Serial.printf("%02x", ci.bssid[i]);
    } else {
      if(ci.rssi > rssiLimit) {
        if(debug) Serial.printf("%2s", " ");
        for (int i = 0; i < 6; i++) if(debug) Serial.printf("%02x", ci.ap[i]);
        if(debug) Serial.printf("  %3d", aps_known[u].channel);
        if(debug) Serial.printf("   %4d\r\n", ci.rssi);
        averageSignalStrength += ci.rssi*-1;
      }
    }
  }
}

void promisc_cb(uint8_t *buf, uint16_t len) {
  if(!setupDone) return;
  int i = 0;
  uint16_t seq_n_new = 0;
  if (len == 12) {
    struct RxControl *sniffer = (struct RxControl*) buf;
  } else if (len == 128) {
    struct sniffer_buf2 *sniffer = (struct sniffer_buf2*) buf;
    struct beaconinfo beacon = parse_beacon(sniffer->buf, 112, sniffer->rx_ctrl.rssi);
    if (register_beacon(beacon) == 0) {
      print_beacon(beacon);
      nothing_new = 0;
    }
  } else {
    struct sniffer_buf *sniffer = (struct sniffer_buf*) buf;
    //Is data or QOS?
    if ((sniffer->buf[0] == 0x08) || (sniffer->buf[0] == 0x88)) {
      struct clientinfo ci = parse_data(sniffer->buf, 36, sniffer->rx_ctrl.rssi, sniffer->rx_ctrl.channel);
      if (memcmp(ci.bssid, ci.station, ETH_MAC_LEN)) {
        if (register_client(ci) == 0) {
          print_client(ci);
          nothing_new = 0;
        }
      }
    }
  }
}

void changeDirection(bool b) {
  if(b) stepper.setPinsInverted(false, false, true);
  else stepper.setPinsInverted(true, false, true);
}

float rssiToMeter(int rssi) {
  return pow(10, ((-69-(-80))/(10*2)));  
}

void setupEnv() {
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  
  SPI.begin();
  Serial.begin(115200);
  while(!Serial);
  //Serial.println("boing");
  //Serial.println(rssiToMeter(2));
  //Serial.println("doing");
  
  
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  driver.begin();             // Initiate pins and registeries
  driver.rms_current(600);    // Set stepper current to 600mA. The command is the same as command TMC2130.setCurrent(600, 0.11, 0.5);
  driver.stealthChop(1);      // Enable extremely quiet stepping
  driver.stealth_autoscale(1);
  driver.microsteps(16);

  stepper.setMaxSpeed(500*steps_per_mm); // 100mm/s @ 80 steps/mm
  stepper.setAcceleration(10000*steps_per_mm); // 2000mm/s^2
  stepper.setEnablePin(EN_PIN);
  stepper.setPinsInverted(false, false, true);
  stepper.enableOutputs();

  timestamp = 0;
  state = PROMISC;
  interval = 3000;

  steps = 100*steps_per_mm;

  // Show welcome
  //changeDirection(false);

  Serial.println("\n· · · · · · · · · · · · · · · · · · · · ·\n");
  Serial.println("→ Setup");
  Serial.println("Continuous Erosion for ESP8266 is starting");
  Serial.println();
  Serial.println("Software: https://github.com/ndsh/continuous-erosion");

  // init sub-routines

  Serial.printf("\n\nSDK version:%s\n\r", system_get_sdk_version());
  Serial.println(F("ESP8266 mini-sniff by Ray Burnette http://www.hackster.io/rayburne/projects"));
  Serial.println(F("Type:   /-------MAC------/-----WiFi Access Point SSID-----/  /----MAC---/  Chnl  RSSI"));

  wifi_set_opmode(STATION_MODE);            // Promiscuous works only with station mode
  wifi_set_channel(channel);
  wifi_promiscuous_enable(disable);
  wifi_set_promiscuous_rx_cb(promisc_cb);   // Set up promiscuous callback
  wifi_promiscuous_enable(enable);
 
  
  Serial.println("\n· · · · · · · · · · · · · · · · · · · · ·");
  Serial.println();
}
