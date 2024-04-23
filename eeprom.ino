void eepromSave(void) {
  uint8_t p = 0;
  EEPROM.writeString(p, ssid);

  p += ssid.length()+1;
  EEPROM.writeString(p, password);

  p += password.length()+1;
  EEPROM.writeString(p, gsmPhoneNumber);

  EEPROM.commit();
}

void eepromLoad(void) {
  uint8_t p = 0;
  str = EEPROM.readString(p);
  if (str != "") ssid = str;

  p += ssid.length()+1;
  str = EEPROM.readString(p);
  if (str != "") password = str;

  p += password.length()+1;
  str = EEPROM.readString(p);
  if (str != "") gsmPhoneNumber = str;
}
