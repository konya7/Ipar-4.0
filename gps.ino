void gpsSerialPrintInfo(void) {
  if (gps.date.isValid()) {
    sprintf(buf, "%04d.%02d.%02d.", gps.date.year(), gps.date.month(), gps.date.day());
    Serial.print(buf);
  } else
    Serial.print(F("0000.00.00."));
  
  Serial.print(F(" "));

  if (gps.time.isValid()) {
    sprintf(buf, "%02d:%02d:%02d.%02d", (gps.time.hour()+UTC_OFFSET_HOUR) % 24, gps.time.minute(), gps.time.second(), gps.time.centisecond());
    Serial.print(buf);  
  } else
    Serial.print(F("00:00:00.00"));
  
  Serial.print(" ");

  if (gps.location.isValid()) {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  } else
    Serial.print(F("0,0"));

  Serial.println();
}

void gpsSaveToFile(void) {
  static const char *msg = "Saving GPS data";
  char *s = buf;
  memset(buf, 0, sizeof(buf));
  if (gps.date.isValid())
    sprintf(s, "\"%04d.%02d.%02d.\",", gps.date.year(), gps.date.month(), gps.date.day()); else
    sprintf(s, "\"0000.00.00.\",");
  s += 14;
  if (gps.time.isValid())
    sprintf(s, "\"%02d:%02d:%02d\",", (gps.time.hour()+UTC_OFFSET_HOUR) % 24, gps.time.minute(), gps.time.second()); else
    sprintf(s, "\"00:00:00\",");
  s += 11;
  if (gps.location.isValid())
    sprintf(s, "\"%.6f\",\"%.6f\"\n", gps.location.lat(), gps.location.lng()); else   // "%f" == double
    sprintf(s, "\"?\",\"?\"\n");
  Serial.print(F("Writing position to file: "));
  Serial.print(buf);
  
  File fileLog = SPIFFS.open(F("/location.csv"), "a");
  if (fileLog){
    int len = strlen(buf);
    int bw = fileLog.print(buf);
    fileLog.close();
    if (bw == len) {
      Serial.println(F("OK"));
      tftPrintMessage((char *)msg,"OK");
    } else {
      Serial.println(F("ERROR"));
      tftPrintMessage((char *)msg,"ERROR");
    }
  } else {
    Serial.println(F("Error writing location.csv"));
    tftPrintMessage((char *)msg,"ERROR");
  }
  delay(2000);
  tftPrintMessage();
}

void gpsDeleteFile(void) {
  static const char *msg = "Deleting GPS data";
  Serial.print(F("Deleting location.csv..."));
  if ( SPIFFS.remove(F("/location.csv")) ) {
    Serial.println(F("OK"));
    tftPrintMessage((char *)msg,"OK");
  } else {
    Serial.println(F("ERROR"));
    tftPrintMessage((char *)msg,"ERROR");
  }
  delay(2000);
  tftPrintMessage();

}

void gpsShowFile(void) {
  File fileLog = SPIFFS.open(F("/location.csv"), "r");
  if (fileLog) {
    //Serial.println(F("Content of location.csv:"));
    Serial.println(F("\"date\",\"time\",\"latitude\",\"longitude\""));
    while (fileLog.available()) Serial.write(fileLog.read());
    fileLog.close();
    Serial.println();
  } else
    Serial.println(F("Error reading location.csv"));
}
