#include "secrets.h"

// Pulled from secrets.h
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

void wCheckAndBringUpWiFi() {
  // attempt to connect to WiFi network:
  while ( WiFi.status() != WL_CONNECTED) {
    // print the network name (SSID);
    Serial.print(F("Attempting to connect to network named: "));
    Serial.println(ssid);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    // const char hostname = "traffic32";
    // WiFi.setHostname(hostname);
    WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }

  server.begin();                           // start the web server on port 80
}

void wPrintWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  Serial.print(F("IP Address: "));
  Serial.println(WiFi.localIP());

  if (debug) {
    Serial.print(F("signal strength (RSSI):"));
    Serial.print(WiFi.RSSI());
    Serial.println(F(" dBm"));
    // print where to go in a browser:
    Serial.print(F("To see this page in action, open a browser to http://"));
    Serial.println(WiFi.localIP());
  }
}

void wFindIncomingCommand(String request, char *cmd, unsigned long &timeOut) {
  int start = request.indexOf(F("GET /"));
  int end = request.indexOf(F(" "), start+5);
  int slashIndex = request.indexOf(F("/"), start+5) + 1;
  if(start > -1 && end > -1) {
    if (slashIndex < end) {
      char interim[4];
      request.substring(start+5, slashIndex-1).toCharArray(cmd, 8);
      request.substring(slashIndex, end).toCharArray(interim, 4);

      bool isNum = true;
      for(int i = 0; i < strlen(interim); i++) {
        if (!isDigit(interim[i])) {
          isNum = false;
        }
      }

      if (isNum) {
        timeOut = millis() + (atoi(interim) * 60000UL);
      }
    } else {
      request.substring(start+5, end).toCharArray(cmd, 8);
    }
  }
}

void charToLower(char *cmd) {
  while(*cmd) {
    *cmd = tolower(*cmd);
    ++cmd;
  }
}

void wParseCommand(char *cmd, String &msg, processID &ID, bool &firstRun, unsigned long &endTime, unsigned long &timeOut) {
    charToLower(cmd);
    if (debug) {
    Serial.print(F("Current cmd: "));
    Serial.println(cmd);
  }

  if (strcmp(cmd, "status") == 0) {
    msg = F("Light/Process information.");

  } else if (strcmp(cmd, "debug") == 0) {
    if (debug) {
      msg = F("Disabling debug mode.");
      Serial.print(F("Disabling"));
    } else {
      msg = F("Enabling debug mode");
      Serial.print(F("Enabling"));
    }
    Serial.println(F(" debug mode"));

    debug = !debug;

  } else if (strcmp(cmd, "shuffle") == 0) {
    ID = (processID)random(0, LAST);
    firstRun = true;
    msg = F("Starting random pattern.");

  } else if (strcmp(cmd, "traffic") == 0) {
    ID = TRAFFIC;
    msg = F("Displaying traffic pattern.");

  } else if (strcmp(cmd, "random") == 0) {
    ID = RANDOM;
    msg = F("Displaying random pattern.");

  } else if (strcmp(cmd, "rlgl") == 0) {
    ID = RLGL;
    msg = F("Playing red light/green light.");

  } else if (strcmp(cmd, "end") == 0 || strcmp(cmd, "stop") == 0 || strcmp(cmd, "off") == 0) {
    ID = OFF;
    timeOut = 0UL;
    msg = F("Stopping running process now.");

    lState(TURN_OFF, Red);
    lState(TURN_OFF, Yel);
    lState(TURN_OFF, Gre);

  } else if (cmd[0] == 'f' && cmd[1] == 'l' && cmd[2] == 'a' && cmd[3] == 's' && cmd[4] == 'h') {
    if (isAlpha(cmd[5])) {
      switch(cmd[5]) {
        case 'r':
          ID = FLASHR;
          msg = F("Flashing Red.");
          firstRun = true;
          break;

        case 'y':
          ID = FLASHY;
          msg = F("Flashing Yellow.");
          firstRun = true;
          break;

        case 'g':
          ID = FLASHG;
          msg = F("Flashing Green.");
          firstRun = true;
          break;

        case 'a':
          ID = FLASHA;
          msg = F("Flashing all.");
          firstRun = true;
          break;

        default:
          msg = F("Unknown command. Please try again or try 'STOP' to stop any process.");
        break;
      }
    } else {
      ID = FLASHA;
      msg = F("Flashing all.");
      firstRun = true;
    }
  } else {
    msg = F("Unknown command. Please try again or try 'STOP' to stop any process.");
  }
}

void wWebResponse(WiFiClient &client, bool &redirect, processID pID, String msg) {
  if (redirect) { //Set redirect at HTTP header level
    client.println(F("HTTP/1.1 302 Found"));
    client.println(F("Location: /"));
  } else { //Pass HTTP headers (access-control & content-type)
    client.println(F("HTTP/1.1 200 OK"));
    client.println(F("Access-Control-Allow-Origin: *"));
    client.println(F("Cache-Control: no-cache"));
    client.println(F("Content-type:application/json"));
    client.print(F("Content-length:"));
    client.println(69 + (msg.length()));
    client.println();

    //JSON response
    client.print(F("{\"msg\": \""));
    client.print(msg);
    client.print(F("\", \"process\": "));
    client.print(pID);
    client.print(F(", \"lights\": {\"Red\": "));
    client.print(lState(CHECK_STATE, Red));
    client.print(F(", \"Yel\": "));
    client.print(lState(CHECK_STATE, Yel));
    client.print(F(", \"Gre\": "));
    client.print(lState(CHECK_STATE, Gre));
    client.println(F("}}"));
    client.println();
  }
}
