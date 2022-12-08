
void lInitLight(light &data, int pinID) {
  data.lPin = pinID;

  if (pinID == 14) {
    strcpy(data.descriptor, "Red");
  } else if (pinID == 27) {
    strcpy(data.descriptor, "Yel");
  } else if (pinID == 26) {
    strcpy(data.descriptor, "Gre");
  }

  if (debug) {
    Serial.print(F("Setting up "));
    Serial.print(data.descriptor);
    Serial.println(F(" light."));
  }

  data.onTime = millis();
  pinMode(data.lPin, OUTPUT);
  digitalWrite(data.lPin, LOW);
}

int lState(int ChkOnOff, light &data) {
  //Return -1 for all else aside from case 3 (state)
  switch(ChkOnOff) {
    case TURN_ON:
      if (digitalRead(data.lPin) > 0) {
        digitalWrite(data.lPin, LOW);
        data.onTime = millis();

        if (debug) {
          Serial.print(F("Turning on: "));
          Serial.print(data.descriptor);
          Serial.print(F(". Time: "));
          Serial.println(data.onTime);
        }
      }
      return(RETURN_NEG);
    case TURN_OFF:
      if (digitalRead(data.lPin) < 1) {
        digitalWrite(data.lPin, HIGH);
        data.onTime = 0;
        if (debug) {
          Serial.print(F("Turning off: "));
          Serial.print(data.descriptor);
          Serial.print(F(". Time: "));
          Serial.println(data.onTime);
        }
      }
      return(RETURN_NEG);
    case CHECK_STATE:
      return(digitalRead(data.lPin));
    default:
      return(RETURN_NEG);
  }
}

bool lRandLight() {
  int randCheck = int(random(0,2));
  if (randCheck > 0) {
    return(true);
  } else {
    return(false);
  }
}

void lOnTooLong(light &data) {
  if (lState(CHECK_STATE, data)) {
    if (data.onTime != 0) {
      data.onTime = 0;
    }
  } else {
    if (data.onTime == 0) {
      data.onTime = millis();
      if (debug) {
        Serial.print(F("Setting time on for "));
        Serial.print(data.descriptor);
        Serial.println(F(" (you heathen)"));
      }
    } else if ((millis() - data.onTime) > 30000) {
      if (debug) {
        Serial.print(data.descriptor);
        Serial.println(F(" on too long."));
      }
      lState(TURN_OFF, data);
    }
  }
}

void lCheckEndTime(processID &ID, unsigned long &endTime, unsigned long &timeOut) {
  unsigned long curMillis = millis();

  if (endTime != 0UL && curMillis > endTime && (curMillis - endTime) < 4000000000) {
    if (debug) {
      Serial.println(F("Resetting end time."));
    }
    endTime = 0UL;
  }

  if (timeOut != 0UL && curMillis > timeOut && (curMillis - timeOut) < 4000000000) {
    if (debug) {
      Serial.println(F("Timed out. Resetting processes."));
    }

    ID = OFF;
    timeOut = 0UL;
  }
}

void lightController(PTracker &currentProcess) {
  lCheckEndTime(currentProcess.ID, currentProcess.endTime, currentProcess.timeOut);

  if (debug) {
    Serial.print(F("Current Process ID: "));
    Serial.println(currentProcess.ID);

    Serial.print(F("Current end time: "));
    Serial.println(currentProcess.endTime);

    Serial.print(F("Current time out: "));
    Serial.println(currentProcess.timeOut);
    Serial.println();
  }

  switch(currentProcess.ID) {
    case RLGL:
      if (currentProcess.firstRun) {
        if (debug) {
          Serial.println(F("RLGL first run."));
        }

        currentProcess.firstRun = false;
        lState(TURN_ON, Red);
        lState(TURN_OFF, Yel);
        lState(TURN_OFF, Gre);
      }

      if (currentProcess.endTime == 0) {
        currentProcess.endTime = millis() + random(2000,6000);

        if (lState(CHECK_STATE, Red)) {
          lState(TURN_OFF, Gre);
          lState(TURN_ON, Red);
        } else {
          lState(TURN_OFF, Red);
          lState(TURN_ON, Gre);
        }
        currentProcess.lightsSwitched += 1;
        if(currentProcess.lightsSwitched > 51) {
          currentProcess.lightsSwitched = 0;
          Serial.println(F("Game over. Are you still there?"));
          currentProcess.ID = OFF;
          currentProcess.timeOut = 0UL;
        }
      }
      break;

    case TRAFFIC:
      if (currentProcess.firstRun) {
        currentProcess.firstRun = false;
      }

      if (currentProcess.endTime == 0) {
        currentProcess.endTime = millis() + random(8000,16000);

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
      if (currentProcess.firstRun) {
        currentProcess.firstRun = false;
      }

      if (currentProcess.endTime == 0) {
        currentProcess.endTime = millis() + random(4000,16000);

        lState(lRandLight()?TURN_ON:TURN_OFF, Red);
        lState(lRandLight()?TURN_ON:TURN_OFF, Yel);
        lState(lRandLight()?TURN_ON:TURN_OFF, Gre);
      }
      break;

    case FLASHR:
      if (currentProcess.firstRun) {
        currentProcess.firstRun = false;
        lState(TURN_OFF, Red);
        lState(TURN_OFF, Yel);
        lState(TURN_OFF, Gre);
      }

      if (currentProcess.endTime == 0) {
        currentProcess.endTime = millis() + 2000;
        if (lState(CHECK_STATE, Red)) {
          lState(TURN_ON, Red);
        } else {
          lState(TURN_OFF, Red);
        }
      }
      break;

    case FLASHY:
      if (currentProcess.firstRun) {
        currentProcess.firstRun = false;
        lState(TURN_OFF, Red);
        lState(TURN_OFF, Yel);
        lState(TURN_OFF, Gre);
      }

      if (currentProcess.endTime == 0) {
        currentProcess.endTime = millis() + 2000;

        if (lState(CHECK_STATE, Yel)) {
          lState(TURN_ON, Yel);
        } else {
          lState(TURN_OFF, Yel);
        }
      }
      break;

    case FLASHG:
      if (currentProcess.firstRun) {
        currentProcess.firstRun = false;
        lState(TURN_OFF, Red);
        lState(TURN_OFF, Yel);
        lState(TURN_OFF, Gre);
      }

      if (currentProcess.endTime == 0) {
        currentProcess.endTime = millis() + 2000;

        if (lState(CHECK_STATE, Gre)) {
          lState(TURN_ON, Gre);
        } else {
          lState(TURN_OFF, Gre);
        }
      }
      break;

    case FLASHA:
      if (currentProcess.firstRun) {
        currentProcess.firstRun = false;
        lState(TURN_OFF, Red);
        lState(TURN_OFF, Yel);
        lState(TURN_OFF, Gre);
      }

      if (currentProcess.endTime == 0) {
        currentProcess.endTime = millis() + 2000;

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
      if (currentProcess.firstRun) {
        currentProcess.firstRun = false;
      }

      break;
  }
}
