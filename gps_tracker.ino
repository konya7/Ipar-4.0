#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

// Telepítés: https://www.youtube.com/watch?v=b8254--ibmM
// TFT: https://www.youtube.com/watch?v=WFVjsxFMbSM
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <FS.h>
#include <SPIFFS.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "src/Button2/Button2.h"
#include "src/EEPROM/EEPROM.h"

#define EEPROM_SIZE 128
#define UTC_OFFSET_HOUR 1

const uint8_t pinGSM_TX = 22;   // zöld
const uint8_t pinGSM_RX = 21;   // kék
const uint8_t pinGPS_TX = 38;   // zöld
const uint8_t pinGPS_RX = 37;   // kék
const uint8_t pinBTN_UP = 35;
const uint8_t pinBTN_DOWN = 0;

String ssid = "TTGO";
String password = "12345678";  // minimum 8 characters required
String gsmPhoneNumber = "+36304005037";

Button2 buttonUp = Button2(pinBTN_UP);
Button2 buttonDown = Button2(pinBTN_DOWN);

TFT_eSPI tft = TFT_eSPI(); // ttgo t-display: 240x135

const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial gps_ss(pinGPS_RX, pinGPS_TX);

const uint32_t GSMBaud = 9600;
SoftwareSerial gsm_ss(pinGSM_RX, pinGSM_TX);
String gsmReceivedSMS = "";

WiFiServer server(80);

char buf[200] = {0};
uint32_t msTFTUpdateDisplay = 0;
char c;
String str;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  WiFi.softAPdisconnect(true);

  tft.init();
  tft.setRotation(1);
  tft.setTextFont(4);
  tft.setTextWrap(false);
  tft.fillScreen(TFT_BLACK);

  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println(F("EEPROM initialization error"));
    tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.setCursor(0,0,2);
    tft.println(F("EEPROM error"));
    delay(5000);
    tft.fillScreen(TFT_BLACK);
    //ESP.restart();
  }

  if(!SPIFFS.begin(true)){
    Serial.println(F("SPIFFS initialization error"));
    tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.setCursor(0,0,2);
    tft.println(F("SPIFFS error"));
    delay(5000);
    tft.fillScreen(TFT_BLACK);
    delay(5000);
  }

  eepromLoad();

  gps_ss.begin(GPSBaud);
  gsm_ss.begin(GSMBaud);
  gsmReceiveMode();

  buttonUp.setClickHandler(handlerButtonUp);
  buttonUp.setLongClickHandler(handlerButtonUp);
  buttonUp.setDoubleClickHandler(handlerButtonUp);
  buttonDown.setClickHandler(handlerButtonDown);
  buttonDown.setLongClickHandler(handlerButtonDown);
  buttonDown.setDoubleClickHandler(handlerButtonDown);

  serialHelp();
}

void loop() {
  buttonUp.loop();
  buttonDown.loop();
  serialInput();
  while (gps_ss.available() > 0) gps.encode(gps_ss.read());
  while (gsm_ss.available() > 0) {
    c = (char)gsm_ss.read();
    Serial.print(c);
    gsmReceivedSMS += c;
  }
  if (gsmReceivedSMS.indexOf("\n") >= 0) {
    if (gsmReceivedSMS.indexOf("Location") >= 0) gsmSendSMS();
    gsmReceivedSMS = "";
  }
  tftUpdateDisplay(1000);
/*
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println(F("GPS not detected"));
    while(true);
  }
*/
}

void tftUpdateDisplay(unsigned int interval) {
  if (millis() - msTFTUpdateDisplay < interval) return;
  msTFTUpdateDisplay = millis();

  //tft.fillScreen(TFT_BLACK);
  tft.setCursor(0,2);
  tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
  if (gps.date.isValid()) {
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%04d.%02d.%02d. ", gps.date.year(), gps.date.month(), gps.date.day());
    tft.print(buf);
  } else
    tft.print(F("0000.00.00 "));

  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  if (gps.time.isValid()) {
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%02d:%02d:%02d", (gps.time.hour()+UTC_OFFSET_HOUR) % 24, gps.time.minute(), gps.time.second());
    tft.println(buf);
  } else
    tft.println(F("00:00:00"));

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  if (gps.location.isValid()) {
    tft.setCursor(0,34);
    tft.println(gps.location.lat(), 6);
    tft.setCursor(0,66);
    tft.println(gps.location.lng(), 6);
  } else {
    tft.setCursor(0,34);
    tft.println(F("latitude?"));
    tft.setCursor(0,66);
    tft.println(F("longitude?"));
  }

  if (gps.speed.isValid())
    sprintf(buf, "%.0f", gps.speed.kmph()); else
    sprintf(buf, "-");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawCentreString(buf, 190, 38, 7);  //240x135
  tft.drawCentreString(F("km/h"), 192, 106, 4);
}

void tftPrintMessage(char *msg1=NULL, String msg2="") {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0,0);
  tft.println();
  if (msg1 != NULL) tft.println(msg1);
  tft.setCursor(0,56);
  if (msg2 != "") tft.println(msg2);
}

void handlerButtonUp(Button2& btn) {
  unsigned int time = btn.wasPressedFor();
  switch (btn.getClickType()) {
      case SINGLE_CLICK:
          gpsSerialPrintInfo();
          gpsSaveToFile();
          break;
      case DOUBLE_CLICK:
          gsmSendSMS();
          break;
      case LONG_CLICK:
          if (time > 3000) gpsDeleteFile();
          break;
  }
}

void handlerButtonDown(Button2& btn) {
  unsigned int time = btn.wasPressedFor();
  switch (btn.getClickType()) {
      case SINGLE_CLICK:
          wifiConnect();
          break;
      case DOUBLE_CLICK:
          Serial.println("SIM800L gsm number is +36702200434");
          tftPrintMessage("GSM number is", String("+36702200434"));
          delay(5000);
          tft.fillScreen(TFT_BLACK);
          break;
      case LONG_CLICK:
          if (time > 3000) {
            Serial.println("'"+ssid+"'");
            Serial.println("'"+password+"'");
            Serial.println("'"+gsmPhoneNumber+"'");
            tftPrintMessage((char *)gsmPhoneNumber.c_str(), ssid);
            tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
            tft.println(password);
            delay(5000);
            tft.fillScreen(TFT_BLACK);
          }
          break;
  }
}
