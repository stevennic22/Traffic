#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>

#define RETURN_NEG -1
#define TURN_ON 1
#define TURN_OFF 2
#define CHECK_STATE 3

enum processID {
  OFF,
  RLGL,
  TRAFFIC,
  RANDOM,
  FLASHR,
  FLASHY,
  FLASHG,
  FLASHA
};

struct Light {
  int lPin;
  char descriptor[4];
  unsigned long onTime;
};

Light Red;
Light Yel;
Light Gre;

struct PTracker {
  int ID = 0;
  bool firstRun = false;
  unsigned long timeOut = 0UL;
} curProcess;

int lightsSwitched = 0;
int statusCheck = 0;
unsigned long curMillis = 0UL;
unsigned long endTime = 0UL;

BridgeServer BServer;

void setup() {
  Red.lPin = 9;
  strcpy(Red.descriptor, "Red");
  Red.onTime = millis();
  pinMode(Red.lPin, OUTPUT);
  digitalWrite(Red.lPin, LOW);
  Console.println("Setting up " + (String)Red.descriptor + " light state.");
  delay(1000);

  Yel.lPin = 8;
  strcpy(Yel.descriptor, "Yel");
  Yel.onTime = millis();
  pinMode(Yel.lPin, OUTPUT);
  digitalWrite(Yel.lPin, LOW);
  Console.println("Setting up " + (String)Yel.descriptor + " light state.");
  delay(1000);

  Gre.lPin = 7;
  strcpy(Gre.descriptor, "Gre");
  Gre.onTime = millis();
  pinMode(Gre.lPin, OUTPUT);
  digitalWrite(Gre.lPin, LOW);
  Console.println("Setting up " + (String)Gre.descriptor + " light state.");

  Serial.begin(9600);
  delay(100);
  Bridge.begin();
  delay(100);
  BServer.listenOnLocalhost();
  BServer.begin();

  delay(1000);

  Console.println(F("Server started. Awaiting requests"));
}

int lState(int ChkOnOff, Light &data) {
  //Return -1 for all else aside from case 3 (state)
  switch(ChkOnOff) {
    case TURN_ON:
      if (digitalRead(data.lPin) > 0) {
        digitalWrite(data.lPin, LOW);
        data.onTime = millis();
        Console.println("Turning on  " + String(data.descriptor) + ": " + String(data.onTime));
      }
      return(RETURN_NEG);
    case TURN_OFF:
      if (digitalRead(data.lPin) < 1) {
        digitalWrite(data.lPin, HIGH);
        data.onTime = 0;
        Console.println("Turning off " + String(data.descriptor) + ": " + String(data.onTime));
      }
      return(RETURN_NEG);
    case CHECK_STATE:
      //Check light state (LOW/HIGH)
      //Console.println("State of " + String(data.descriptor) + ": " + String(digitalRead(data.lPin)));
      return(digitalRead(data.lPin));
    default:
      return(RETURN_NEG);
  }
}

bool randLight() {
  int randCheck = int(random(0,2));
  if (randCheck > 0) {
    return(true);
  } else {
    return(false);
  }
}

void onTooLong(Light &data) {
  if (lState(CHECK_STATE, data)) {
    if (data.onTime != 0) {
      data.onTime = 0;
    }
    return;
  } else {
    curMillis = millis();

    if (data.onTime == 0) {
      data.onTime = curMillis;
      Console.println("Setting time on for " + String(data.descriptor) + " (you heathen)");
      return;
    } else if ((curMillis - data.onTime) > 30000) {
      Console.println(String(data.descriptor) + " on too long.");
      lState(TURN_OFF, data);
      return;
    }
  }
}

void clientReply(BridgeClient &replyClient, String message) {
  replyClient.println(F("Status: 200"));
  replyClient.println(F("Content-type: application/json; charset=utf-8"));
  replyClient.println();
  replyClient.println("{\"message\": \"" + message + "\"}");
  replyClient.stop();
  replyClient.flush();

  Console.println("{'message': '" + message + "'}");
}

void clientRead(BridgeClient &passedClient, PTracker &currentProcess) {
  String command = passedClient.readStringUntil('/');
  command.trim();
  command.toLowerCase();
  unsigned long timeToEnd = (unsigned long)passedClient.parseInt();
  if (!timeToEnd) {
    currentProcess.timeOut = 0UL;
  } else {
    currentProcess.timeOut = millis() + (timeToEnd * 60000UL); //minutes to milliseconds
  }

  if (command.indexOf(F("end")) > -1 || command.indexOf(F("stop")) > -1 || command.indexOf(F("off")) > -1) {
    clientReply(passedClient, F("Stopping running process now."));
    currentProcess.ID = OFF;
    currentProcess.timeOut = 0UL;

    if (command.indexOf(F("off")) > -1) {
      lState(TURN_OFF, Red);
      lState(TURN_OFF, Yel);
      lState(TURN_OFF, Gre);
    }

  } else if (command.indexOf(F("rlgl")) > -1) {
    clientReply(passedClient, F("Playing red light/green light."));
    currentProcess.ID = RLGL;

  } else if (command.indexOf(F("traffic")) > -1 ) {
    clientReply(passedClient, F("Displaying traffic pattern."));
    currentProcess.ID = TRAFFIC;

  } else if (command.indexOf(F("random")) > -1 ) {
    clientReply(passedClient, F("Displaying random pattern."));
    currentProcess.ID = RANDOM;

  } else if (command.indexOf(F("flash")) > -1) {
    if (command.length() > 5) {
      char switchVal = command.charAt(5);
      switch(switchVal) {
        case 'r':
          clientReply(passedClient, F("Flashing Red."));
          currentProcess.ID = FLASHR;
          currentProcess.firstRun = true;
          break;

        case 'y':
          clientReply(passedClient, F("Flashing Yellow."));
          currentProcess.ID = FLASHY;
          currentProcess.firstRun = true;
          break;

        case 'g':
          clientReply(passedClient, "Flashing Green.");
          currentProcess.ID = FLASHG;
          currentProcess.firstRun = true;
          break;

        case 'a':
          clientReply(passedClient, F("Flashing all."));
          currentProcess.ID = FLASHA;
          currentProcess.firstRun = true;
          break;

        default:
          clientReply(passedClient, F("Try flashr instead."));
        break;

      }
    } else {
      clientReply(passedClient, F("Flashing all."));
      currentProcess.ID = FLASHA;
      currentProcess.firstRun = true;
    }
  } else {
    clientReply(passedClient, F("Unknown command. Please try again or try 'STOP' to stop any process."));
  }
}

void loop() {
  BridgeClient client = BServer.accept();

  if (client) {
    clientRead(client, curProcess);
  }

  curMillis = millis();

  if (endTime != 0 && curMillis > endTime && (curMillis - endTime) < 4000000000) {
    endTime = 0;
  }

  if (curProcess.timeOut != 0 && curMillis > curProcess.timeOut && (curMillis - curProcess.timeOut) < 4000000000) {
    curProcess.ID = 0;
    curProcess.timeOut = 0;
  }

  if (statusCheck >= 1000) {
    onTooLong(Red);
    onTooLong(Yel);
    onTooLong(Gre);
    statusCheck = 0;
  } else {
    statusCheck += 1;
  }

  switch(curProcess.ID) {
    case RLGL:
      if (curProcess.firstRun) {
        curProcess.firstRun = false;
        lState(TURN_ON, Red);
        lState(TURN_OFF, Yel);
        lState(TURN_OFF, Gre);
      }

      if (endTime == 0) {
        endTime = millis() + random(2000,6000);

        if (lState(CHECK_STATE, Red)) {
          lState(TURN_OFF, Gre);
          lState(TURN_ON, Red);
        } else {
          lState(TURN_OFF, Red);
          lState(TURN_ON, Gre);
        }
        lightsSwitched += 1;
        if(lightsSwitched > 51) {
          lightsSwitched = 0;
          Console.println(F("Game over. Are you still there?"));
          curProcess.ID = 0;
        }
      }
      break;

    case TRAFFIC:
      if (endTime == 0) {
        endTime = millis() + random(8000,16000);

        if (!lState(CHECK_STATE, Red)) {
          lState(TURN_OFF, Red);
          lState(TURN_OFF, Yel);
          lState(TURN_ON, Gre);
        } else if (!lState(CHECK_STATE, Gre)) {
          lState(TURN_OFF, Red);
          lState(TURN_OFF, Gre);
          lState(TURN_ON, Yel);
        } else if (!lState(CHECK_STATE, Yel)) {
          lState(TURN_OFF, Yel);
          lState(TURN_OFF, Gre);
          lState(TURN_ON, Red);
        } else {
          lState(TURN_ON, Red);
        }
      }
      break;

    case RANDOM:
      if (endTime == 0) {
        endTime = millis() + random(4000,16000);

        lState(randLight()?TURN_ON:TURN_OFF, Red);
        lState(randLight()?TURN_ON:TURN_OFF, Yel);
        lState(randLight()?TURN_ON:TURN_OFF, Gre);
      }
      break;

    case FLASHR:
      if (curProcess.firstRun) {
        curProcess.firstRun = false;
        lState(TURN_OFF, Red);
        lState(TURN_OFF, Yel);
        lState(TURN_OFF, Gre);
      }

      if (endTime == 0) {
        endTime = millis() + 2000;
        if (lState(CHECK_STATE, Red)) {
          lState(TURN_ON, Red);
        } else {
          lState(TURN_OFF, Red);
        }
      }
      break;

    case FLASHY:
      if (curProcess.firstRun) {
        curProcess.firstRun = false;
        lState(TURN_OFF, Red);
        lState(TURN_OFF, Yel);
        lState(TURN_OFF, Gre);
      }

      if (endTime == 0) {
        endTime = millis() + 2000;

        if (lState(CHECK_STATE, Yel)) {
          lState(TURN_ON, Yel);
        } else {
          lState(TURN_OFF, Yel);
        }
      }
      break;

    case FLASHG:
      if (curProcess.firstRun) {
        curProcess.firstRun = false;
        lState(TURN_OFF, Red);
        lState(TURN_OFF, Yel);
        lState(TURN_OFF, Gre);
      }

      if (endTime == 0) {
        endTime = millis() + 2000;

        if (lState(CHECK_STATE, Gre)) {
          lState(TURN_ON, Gre);
        } else {
          lState(TURN_OFF, Gre);
        }
      }
      break;

    case FLASHA:
      if (curProcess.firstRun) {
        curProcess.firstRun = false;
        lState(TURN_OFF, Red);
        lState(TURN_OFF, Yel);
        lState(TURN_OFF, Gre);
      }

      if (endTime == 0) {
        endTime = millis() + 2000;

        if (lState(CHECK_STATE, Red)) {
          lState(TURN_ON, Red);
        } else {
          lState(TURN_OFF, Red);
        }

        if (lState(CHECK_STATE, Yel)) {
          lState(TURN_ON, Yel);
        } else {
          lState(TURN_OFF, Yel);
        }

        if (lState(CHECK_STATE, Gre)) {
          lState(TURN_ON, Gre);
        } else {
          lState(TURN_OFF, Gre);
        }
      }

      break;

    case 0:
    default:
    break;
  }
}
