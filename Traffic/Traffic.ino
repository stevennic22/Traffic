#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>

#define RETURN_NEG -1
#define TURN_ON 1
#define TURN_OFF 2
#define CHECK_STATE 3

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
  unsigned long timeOut = 0UL;
} curProcess;

int randRes = random(1,8);
int lightsSwitched = 0;
int statusCheck = 0;
int lastLight = 0;
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
      digitalWrite(data.lPin, LOW);
      data.onTime = millis();
      Console.println("Turning on  " + String(data.descriptor) + ": " + String(data.onTime));
      return(RETURN_NEG);
    case TURN_OFF:
      digitalWrite(data.lPin, HIGH);
      data.onTime = 0;
      Console.println("Turning off " + String(data.descriptor) + ": " + String(data.onTime));
      return(RETURN_NEG);
    case CHECK_STATE:
      //Check light state (LOW/HIGH)
      //Console.println(F("State of " + String(data.descriptor) + ": " + String(digitalRead(data.lPin))));
      return(digitalRead(data.lPin));
    default:
      return(RETURN_NEG);
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
      lState(TURN_OFF, data);
      Console.println("On too long. Turning " + String(data.descriptor) + " light off");
      return;
    }
  }
}

void clientReply(BridgeClient &replyClient, String message) {
  replyClient.println(F("Status: 200"));
  replyClient.println(F("Content-type: application/json; charset=utf-8"));
  replyClient.println();
  replyClient.println("{'message': '" + message + "'}");
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
  

  if (command.indexOf(F("end")) > -1 || command.indexOf(F("stop")) > -1) {
    clientReply(passedClient, F("Stopping all processes now."));
    currentProcess.ID = 0;
    currentProcess.timeOut = 0UL;

  } else if (command.indexOf(F("off")) > -1) {
    clientReply(passedClient, F("Stopping all processes and turning off the lights."));
    lState(TURN_OFF, Red);
    lState(TURN_OFF, Yel);
    lState(TURN_OFF, Gre);
    currentProcess.ID = 0;
    currentProcess.timeOut = 0UL;

  } else if (command.indexOf(F("rlgl")) > -1) {
    clientReply(passedClient, F("Starting red light/green light."));
    lState(TURN_ON, Red);
    lState(TURN_OFF, Yel);
    lState(TURN_OFF, Gre);
    currentProcess.ID = 1;

  } else if (command.indexOf(F("traffic")) > -1 ) {
    clientReply(passedClient, F("Starting traffic pattern."));
    currentProcess.ID = 2;

  } else if (command.indexOf(F("random")) > -1 ) {
    clientReply(passedClient, F("Starting random pattern."));
    currentProcess.ID = 3;

  } else if (command.indexOf(F("flash")) > -1) {
    if (command.length() > 5) {
      char switchVal = command.charAt(5);
      switch(switchVal) {
        case 'r':
          clientReply(passedClient, F("Flashing Red."));
          lState(TURN_OFF, Red);
          lState(TURN_OFF, Yel);
          lState(TURN_OFF, Gre);
          currentProcess.ID = 4;
          break;

        case 'y':
          clientReply(passedClient, F("Flashing Yellow."));
          lState(TURN_OFF, Yel);
          lState(TURN_OFF, Red);
          lState(TURN_OFF, Gre);
          currentProcess.ID = 5;
          break;

        case 'g':
          clientReply(passedClient, "Flashing Green.");
          lState(TURN_OFF, Gre);
          lState(TURN_OFF, Red);
          lState(TURN_OFF, Yel);
          currentProcess.ID = 6;
          break;

        case 'a':
          clientReply(passedClient, F("Flashing all."));
          lState(TURN_ON, Red);
          lState(TURN_ON, Yel);
          lState(TURN_ON, Gre);
          currentProcess.ID = 7;
          break;

        default:
          clientReply(passedClient, F("Try flashr instead."));
          break;

      }
    } else {
      clientReply(passedClient, F("Flashing all."));
      lState(TURN_ON, Red);
      lState(TURN_ON, Yel);
      lState(TURN_ON, Gre);
      currentProcess.ID = 7;
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
    case 1:
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

    case 2:
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

    case 3:
      if (endTime == 0) {
        endTime = millis() + random(4000,16000);

        if (lastLight == 0) {
          lastLight = randRes;
        } else if (lastLight == randRes) {
          while (lastLight == randRes) {
            randRes = random(1,8);
          }
          lastLight = randRes;
        }

        switch(randRes) {
          case 1:
            lState(TURN_ON, Red);
            lState(TURN_OFF, Yel);
            lState(TURN_OFF, Gre);
            break;
          case 2:
            lState(TURN_ON, Yel);
            lState(TURN_OFF, Red);
            lState(TURN_OFF, Gre);
            break;
          case 3:
            lState(TURN_ON, Gre);
            lState(TURN_OFF, Red);
            lState(TURN_OFF, Yel);
            break;
          case 4:
            lState(TURN_ON, Red);
            lState(TURN_ON, Yel);
            lState(TURN_OFF, Gre);
            break;
          case 5:
            lState(TURN_ON, Red);
            lState(TURN_OFF, Yel);
            lState(TURN_ON, Gre);
            break;
          case 6:
            lState(TURN_OFF, Red);
            lState(TURN_ON, Yel);
            lState(TURN_ON, Gre);
            break;
          case 7:
          default:
            lState(TURN_ON, Red);
            lState(TURN_ON, Yel);
            lState(TURN_ON, Gre);
            break;
        }
      }
      break;

    case 4:
      if (endTime == 0) {
        endTime = millis() + 2000;
        if (lState(CHECK_STATE, Red)) {
          lState(TURN_ON, Red);
        } else {
          lState(TURN_OFF, Red);
        }
      }
      break;

    case 5:
      if (endTime == 0) {
        endTime = millis() + 2000;
        
        if (lState(CHECK_STATE, Yel)) {
          lState(TURN_ON, Yel);
        } else {
          lState(TURN_OFF, Yel);
        }
      }
      break;

    case 6:
      if (endTime == 0) {
        endTime = millis() + 2000;
        
        if (lState(CHECK_STATE, Gre)) {
          lState(TURN_ON, Gre);
        } else {
          lState(TURN_OFF, Gre);
        }
      }
      break;

    case 7:
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
