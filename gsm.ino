void gsmReceiveMode(void) {
  delay(500);
  gsm_ss.println("AT");
  gsmUpdateSerial();
  gsm_ss.println("AT+CMGF=1");
  gsmUpdateSerial();
  gsm_ss.println("AT+CNMI=2,2,0,0,0");
  gsmUpdateSerial();
}

void gsmSendSMS(void) {
  char *s = buf;
  memset(buf, 0, sizeof(buf));
  if (gps.date.isValid())
    sprintf(s, "%04d.%02d.%02d. ", gps.date.year(), gps.date.month(), gps.date.day()); else
    sprintf(s, "0000.00.00. ");
  s += 12;
  if (gps.time.isValid())
    sprintf(s, "%02d:%02d:%02d ", (gps.time.hour()+UTC_OFFSET_HOUR) % 24, gps.time.minute(), gps.time.second()); else
    sprintf(s, "00:00:00 ");
  s += 9;
  if (gps.location.isValid())
    sprintf(s, "%.6f, %.6f", gps.location.lat(), gps.location.lng()); else   // "%f" == double
    sprintf(s, "?, ?");

  Serial.print(F("Sending SMS:")); Serial.println(buf);
  Serial.print(F("Phone number:")); Serial.println(gsmPhoneNumber);

  tftPrintMessage("Sending SMS...", gsmPhoneNumber);
  gsm_ss.println("AT");
  gsmUpdateSerial();
  gsm_ss.println("AT+CMGF=1"); // switch to text mode
  gsmUpdateSerial();
  gsm_ss.println("AT+CMGS=\""+gsmPhoneNumber+"\"");
  gsmUpdateSerial();
  gsm_ss.println(buf);
  gsmUpdateSerial();
  gsm_ss.write(26); //send a Ctrl+Z (end of the message)

  gsmReceiveMode();
  tft.fillScreen(TFT_BLACK);  // clear screen
}

void gsmUpdateSerial(void) {
  delay(500);
  //while(Serial.available()) gsm_ss.write(Serial.read());
  while(gsm_ss.available()) Serial.write(gsm_ss.read());
}
