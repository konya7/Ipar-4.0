String inputString = "";

void serialInput() {
  if (Serial.available() > 0) {
    char inChar = (char)Serial.read(); 
    if (inChar == '\r' or inChar == 0x0F or inChar == 0x0A or inChar == '$') {
      parseSerialInput();
      inputString = "";
    } else {
      inputString += inChar;
    }
  }
}

void parseSerialInput() {
  if (inputString == F("help") || inputString == "?") { serialHelp(); return; }
  
  if (inputString == F("load")) {
    eepromLoad();
    Serial.println("'"+ssid+"'");
    Serial.println("'"+password+"'");
    Serial.println("'"+gsmPhoneNumber+"'");
    return;
  }
  if (inputString == F("save")) {
    eepromSave();
    Serial.println(F("saved"));
    return;
  }
  
//  if (inputString == F("time")) { Serial.println(&timeinfo, "%Y.%m.%d. %A, %H:%M:%S"); return; }

  if (inputString == F("ssid")) { Serial.println("'"+ssid+"'"); return; }
  if (inputString == F("pass")) { Serial.println("'"+password+"'"); return; }
  if (inputString == F("phone")) { Serial.println("'"+gsmPhoneNumber+"'"); return; }
  if (inputString.startsWith("ssid=")) { ssid = inputString.substring(5); return; }
  if (inputString.startsWith("pass=")) {
    if (inputString.substring(5).length() < 8)
      Serial.println(F("ERROR: new password shorter then 8 characters")); else
      password = inputString.substring(5);
    return;
  }
  if (inputString.startsWith("phone=")) { gsmPhoneNumber = inputString.substring(6); return; }

  if (inputString == F("gps info")) { gpsSerialPrintInfo(); return; }
  if (inputString == F("gps save")) { gpsSaveToFile(); return; }
  if (inputString == F("gps delete")) { gpsDeleteFile(); return; }
  if (inputString == F("gps show")) { gpsShowFile(); return; }

  if (inputString == F("dir")) { spiffsSerialPrintInfo(); return; }

  if (inputString == F("gsm sms")) {
    Serial.println(F("SMS start"));
    gsmSendSMS();
    Serial.println(F("SMS finished"));
    return;
  }
  if (inputString == F("gsm receive")) { gsmReceiveMode(); return; }
  if (inputString == F("gsm number")) { Serial.println("SIM800L gsm number is +36702200434"); return; }

  if (inputString != "") Serial.print(F("Unknown command: "));
  Serial.println(inputString);
}

void serialHelp() {
  Serial.println();
  Serial.println(F("Usage:"));
  Serial.println(F("help - this page"));
  Serial.println(F("gps info - print gps information"));
  Serial.println(F("gps save - write gps info to location.csv"));
  Serial.println(F("gps delete - remove location.csv"));
  Serial.println(F("gps show - print location.csv to serial"));
  Serial.println(F("dir - list spiffs filesystem"));
  Serial.println(F("gsm sms - send gps information in sms"));
  Serial.println(F("gsm receive - switch to receive mode"));
  Serial.println(F("gsm number - print SIM800L gsm number"));
  Serial.println(F("ssid - print current ssid"));
  Serial.println(F("ssid=<your ssid> - set your ssid"));
  Serial.println(F("pass - print current password"));
  Serial.println(F("pass=<your passs> - set your password"));
  Serial.println(F("phone - print current phone number"));
  Serial.println(F("phone=<your phone number> - set your phone number, format: +36309988777"));
  Serial.println(F("save - save ssid/pass to eeprom"));
  Serial.println(F("load - load ssid/pass from eeprom"));
  Serial.println();
}

void spiffsSerialPrintInfo(void) {
  unsigned int totalBytes = SPIFFS.totalBytes();
  unsigned int usedBytes = SPIFFS.usedBytes();

  Serial.println();
  Serial.println(F("File system info"));
 
  Serial.print(F("Total space: "));
  Serial.print(totalBytes);
  Serial.println(F(" bytes"));
 
  Serial.print(F("Total space used: "));
  Serial.print(usedBytes);
  Serial.println(F(" bytes"));
 
  File dir = SPIFFS.open("/");
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) break;
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
    } else {
      Serial.print("  ");
      Serial.print(entry.size(), DEC);
      Serial.println(F(" bytes"));
    }
    entry.close();
  }
  Serial.println();
}
