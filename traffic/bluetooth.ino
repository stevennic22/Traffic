void btSetEventHandlersForCharacteristics(BLEByteCharacteristic &thisChar) {
  if (debug) {
    Serial.print(F("Setting up BLECharacteristic Events for: "));
    Serial.println(thisChar.uuid());
  }

  thisChar.setEventHandler(BLEUpdated, btCharacteristicUpdated);
  thisChar.setEventHandler(BLESubscribed, btCharacteristicSubscribed);
  thisChar.setEventHandler(BLEUnsubscribed, btCharacteristicUnsubscribed);
}

void btConnectHandler(BLEDevice central) {
  Serial.print(F("Connected event, central: "));
  Serial.println(central.address());
  // digitalWrite(LED_BUITLIN, HIGH);
}

void btDisconnectHandler(BLEDevice central) {
  Serial.print(F("Disconnected event, central: "));
  Serial.println(central.address());
}

void btCharacteristicSubscribed(BLEDevice central, BLECharacteristic thisChar) {
  Serial.print(F("Characteristic subscribed. UUID: "));
  Serial.println(thisChar.uuid());
}

void btCharacteristicUnsubscribed(BLEDevice central, BLECharacteristic thisChar) {
  Serial.print(F("Characteristic unsubscribed. UUID: "));
  Serial.println(thisChar.uuid());
}

void btCharacteristicUpdated(BLEDevice central, BLECharacteristic thisChar) {
  byte incoming = 0;
  thisChar.readValue(incoming);

  if (debug) {
    Serial.print(F("Passed Char: "));
    Serial.println(thisChar.uuid()[10]);
    Serial.print(F("Passed Val : "));
    Serial.println(incoming);
  }

  switch(thisChar.uuid()[10]) {
    case '1':
      if ((int)incoming != curProc.ID && (int)incoming < LAST) {
        btSetProcessToRun(incoming, curProc.ID, curProc.firstRun, curProc.endTime, curProc.timeOut);
      }
      break;

      // Passed Incoming ID value for individual lights
      // 1 & 3 => Toggle Light on only
      // 2 & 4 => Toggle Light on and others lights off
      // 3 & 4 => Also stop process with light controls
    case '2':
      if ((int)incoming == 0) {
        lState(TURN_OFF, Red);
      } else if (((int)incoming % 2) != 0) { //Toggle only this light
        lState(TURN_ON, Red);
        if ((int)incoming == 3) { //Turn off process if incoming is 3
          btSetProcessToRun(OFF, curProc.ID, curProc.firstRun, curProc.endTime, curProc.timeOut);
        }
      } else { //Toggle all lights
        lState(TURN_ON, Red);
        lState(TURN_OFF, Yel);
        lState(TURN_OFF, Gre);
        if ((int)incoming == 4) {
          btSetProcessToRun(OFF, curProc.ID, curProc.firstRun, curProc.endTime, curProc.timeOut);
        }
      }
      break;
    case '3':
      if ((int)incoming == 0) {
        lState(TURN_OFF, Yel);
      } else if (((int)incoming % 2) != 0) { //Toggle only this light
        lState(TURN_ON, Yel);
        if ((int)incoming == 3) { //Turn off process if incoming is 3
          btSetProcessToRun(OFF, curProc.ID, curProc.firstRun, curProc.endTime, curProc.timeOut);
        }
      } else { //Toggle all lights
        lState(TURN_OFF, Red);
        lState(TURN_ON, Yel);
        lState(TURN_OFF, Gre);
        if ((int)incoming == 4) {
          btSetProcessToRun(OFF, curProc.ID, curProc.firstRun, curProc.endTime, curProc.timeOut);
        }
      }
      break;
    case '4':
      if ((int)incoming == 0) {
        lState(TURN_OFF, Gre);
      } else if (((int)incoming % 2) != 0) { //Toggle only this light
        lState(TURN_ON, Gre);
        if ((int)incoming == 3) { //Turn off process if incoming is 3
          btSetProcessToRun(OFF, curProc.ID, curProc.firstRun, curProc.endTime, curProc.timeOut);
        }
      } else {
        lState(TURN_OFF, Red);
        lState(TURN_OFF, Yel);
        lState(TURN_ON, Gre);
        if ((int)incoming == 4) {
          btSetProcessToRun(OFF, curProc.ID, curProc.firstRun, curProc.endTime, curProc.timeOut);
        }
      }
      break;
    default:
      Serial.println(F("No Match."));
      break;
  }
}

void btSetProcessToRun(int setVal, processID &ID, bool &firstRun, unsigned long &endTime, unsigned long &timeOut) {
  switch(setVal){
    case 0:
      ID = OFF;
      break;

    case 1:
      ID = RLGL;
      break;

    case 2:
      ID = TRAFFIC;
      break;

    case 3:
      ID = RANDOM;
      break;

    case 4:
      ID = FLASHR;
      firstRun = true;
      break;

    case 5:
      ID = FLASHY;
      firstRun = true;
      break;

    case 6:
      ID = FLASHG;
      firstRun = true;
      break;

    case 7:
      ID = FLASHA;
      firstRun = true;
      break;

    // Shuffle it up
    case 8:
      setVal = (processID)random(0, LAST);
      btSetProcessToRun(setVal, ID, firstRun, endTime, timeOut);
      break;

    default:
      break;
  }
}

void btUpdateBLEChar(int realValue, BLEByteCharacteristic &BLEChar) {
  if (realValue != BLEChar.value()) {
    BLEChar.writeValue(realValue);
  }
}
