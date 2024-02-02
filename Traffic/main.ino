#include <SPI.h>
#include <WiFi.h>
#include <ArduinoBLE.h>

#include "definitions.h" // Import variable type definitions
#include "secrets.h" // Import device/wifi details

// Pulled from secrets.h
char bleName[] = DEVICE_HOST_NAME;

// Toggle debug output
bool debug = false;

unsigned long wifiConnCheck; // For monitoring wifi connectivity
unsigned long btCharStateCheck; // For monitoring BT characteristic updates
unsigned long lightsCheck; // For monitoring whether lights are on too long

WiFiServer server(80);

light Red;
light Yel;
light Gre;

// Init BLE service
BLEService myService(MY_UUID("0000"));

BLEByteCharacteristic PIDChar(MY_UUID("0001"), BLERead | BLEWrite | BLEWriteWithoutResponse | BLENotify);
BLEByteCharacteristic RLChar(MY_UUID("0002"), BLERead | BLEWrite | BLEWriteWithoutResponse | BLENotify);
BLEByteCharacteristic YLChar(MY_UUID("0003"), BLERead | BLEWrite | BLEWriteWithoutResponse | BLENotify);
BLEByteCharacteristic GLChar(MY_UUID("0004"), BLERead | BLEWrite | BLEWriteWithoutResponse | BLENotify);

// Init process tracker
struct PTracker {
  processID ID = OFF;
  bool redirect = false;
  bool firstRun = false;
  int lightsSwitched = 0;
  unsigned long endTime = 0UL;
  unsigned long timeOut = 0UL;
  unsigned long requestTimeOut = 15000UL;
} curProc;

void setup() {
  Serial.begin(115200);

  // Initialize the lights with pins and UUIDs
  lInitLight(Red, 14);
  strcpy(Red.UUID, MY_UUID("0002"));
  delay(1000);

  lInitLight(Yel, 27);
  strcpy(Yel.UUID, MY_UUID("0003"));
  delay(1000);

  lInitLight(Gre, 26);
  strcpy(Gre.UUID, MY_UUID("0004"));

  // Initialize WiFi and log it out
  wCheckAndBringUpWiFi();
  wPrintWiFiStatus();

  // Begin blootueth initialization
  if (!BLE.begin()) {
    Serial.println(F("starting BLE failed"));
    while (true);
  }

  BLE.setLocalName(bleName);
  BLE.setDeviceName(bleName);
  Serial.print(F("Device name: "));
  Serial.println(bleName);

  BLE.setAdvertisedService(myService);

  myService.addCharacteristic(PIDChar);
  myService.addCharacteristic(RLChar);
  myService.addCharacteristic(YLChar);
  myService.addCharacteristic(GLChar);

  BLE.addService(myService);

  btUpdateBLEChar((int)curProc.ID, PIDChar);
  btUpdateBLEChar(lState(CHECK_STATE, Red), RLChar);
  btUpdateBLEChar(lState(CHECK_STATE, Yel), YLChar);
  btUpdateBLEChar(lState(CHECK_STATE, Gre), GLChar);

  BLE.setEventHandler(BLEConnected, btConnectHandler);
  BLE.setEventHandler(BLEDisconnected, btDisconnectHandler);

  btSetEventHandlersForCharacteristics(PIDChar);
  btSetEventHandlersForCharacteristics(RLChar);
  btSetEventHandlersForCharacteristics(YLChar);
  btSetEventHandlersForCharacteristics(GLChar);

  BLE.advertise();
  // Blootueth initialization complete

  // Initialize status checks
  wifiConnCheck = millis();
  btCharStateCheck = millis();
  lightsCheck = millis();

  Serial.println(F("Server started. Awaiting requests\n"));
}

void loop(){
  BLE.poll(); // Does the Blootueth stuff

  // Ensure characteristic states match with reality ever quarter of a second
  if ((millis() - btCharStateCheck) >= 250) {
    btUpdateBLEChar((int)curProc.ID, PIDChar);
    btUpdateBLEChar(lState(CHECK_STATE, Red), RLChar);
    btUpdateBLEChar(lState(CHECK_STATE, Yel), YLChar);
    btUpdateBLEChar(lState(CHECK_STATE, Gre), GLChar);
    btCharStateCheck = millis();
  }

  WiFiClient client = server.available();   // now listen for incoming clients

  if (client) { // wevegotone.ghostbusters
    if (debug) {
      Serial.println(F("New Client."));
    }

    client.setTimeout(curProc.requestTimeOut); // Don't let them stick around too long

    // Start tracking the connection
    connectionTracker curConn;
    curConn.connStart = millis();

    while (client.connected()) {
      // Stop listening if they're talking too long
      if ((millis() - curConn.connStart) > curProc.requestTimeOut) {
        break;
      }

      if (client.available()) { // Listen to what they have to say
        String output = client.readStringUntil(*"\n");

        if (debug){
          Serial.println(output);
        }

        wFindIncomingCommand(output, curConn.cmd, curProc.timeOut); // Get command from client's incoming stream

        if (output == "\r") { // Expected end of request
          if (debug) {
            Serial.println(F("Checking command and sending response."));
          }

          // Parse command and respond accordingly
          wParseCommand(curConn.cmd, curConn.msg, curProc.ID, curProc.firstRun, curProc.endTime, curProc.timeOut);
          wWebResponse(client, curProc.redirect, curProc.ID, curConn.msg);
        }
      }
    }

    client.stop();  // close the connection

    if (debug) {
      Serial.println(F("Client disconnected."));
      Serial.println();
    }

  } else if ((millis() - wifiConnCheck) > 600000) { //Re-check wifi connection after 10 minutes
    if (debug) {
      Serial.println(F("Checking WiFi connection."));
    }

    // Ensure WiFi is up and reset the clock
    wCheckAndBringUpWiFi();
    wifiConnCheck = millis();

  } else if (millis() > 2592000000) { //Restart device after 30 days
    Serial.print(F("On for "));
    Serial.print(millis());
    Serial.println(F(" milliseconds. Restarting device."));
    ESP.restart();

  } else {
    if ((millis() - lightsCheck) >= 1000) { //Check light status every second
      if (debug) {
        Serial.print(F("Checking if lights are on too long. "));
        Serial.println(millis());
      }

      // Check if lights have been on too long
      lOnTooLong(Red);
      lOnTooLong(Yel);
      lOnTooLong(Gre);

      lightController(curProc); // Handle lights based on current process

      lightsCheck = millis();
    }
  }
}
