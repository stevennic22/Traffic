void btSetEventHandlersForCharacteristics(BLEByteCharacteristic &characteristic) {
  // characteristic.setEventHandler(BLERead, btCharacteristicRead);
  // characteristic.setEventHandler(BLEWrite, btCharacteristicWritten);
  characteristic.setEventHandler(BLEUpdated, btCharacteristicUpdated);
  characteristic.setEventHandler(BLESubscribed, btCharacteristicSubscribed);
  characteristic.setEventHandler(BLEUnsubscribed, btCharacteristicUnsubscribed);
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
  }

  switch(thisChar.uuid()[10]) {
    case '1':
      if ((int)incoming != curProc.ID && (int)incoming < LAST) {
        btSetProcessToRun(incoming, curProc.ID, curProc.firstRun, curProc.endTime, curProc.timeOut);
      }
      break;
    case '2':
    case '3':
    case '4':
    default:
      Serial.println(F("No Match."));
      break;
  }
}

void btSetProcessToRun(int setVal, processID &ID, bool &firstRun, unsigned long &endTime, unsigned long &timeOut) {
  switch(setVal){
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

    default:
      break;
  }
}

void btUpdateBLEChar(int realValue, BLEByteCharacteristic &BLEChar) {
  if (realValue != BLEChar.value()) {
    BLEChar.writeValue(realValue);
  }
}
