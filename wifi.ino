uint32_t msWiFiLoop = 0;
String currentLine = "";

void wifiLoop(void) {
  while (true) {
    if (millis() - msWiFiLoop > 120000UL) break;
    WiFiClient client = server.available();
    if (!client) continue;
    Serial.println(F("New Client"));
    currentLine = "";
    while (client.connected()) {
      if (!client.available()) continue;
      char c = client.read();
      Serial.write(c);
      if (c == '\n') {
        if (currentLine.length() == 0) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/plain");
          client.println();
          client.println(F("\"date\",\"time\",\"latitude\",\"longitude\""));
          File fileLog = SPIFFS.open(F("/location.csv"), "r");
          if (fileLog) {
            while (fileLog.available()) client.write(fileLog.read());
            fileLog.close();
          } else
            client.println(F("Error reading location.csv"));
          break;
        } else
          currentLine = "";
      } else if (c != '\r') {
        currentLine += c;
      }
    }
    client.stop();
    Serial.println(F("Client Disconnected"));
    msWiFiLoop = millis();
  }
}

void wifiConnect(void) {
  Serial.println();
  Serial.println(F("Activating WiFi AP"));
  Serial.println(ssid);
  tftPrintMessage("WiFi Access Point", ssid);
  WiFi.setAutoReconnect(false);
  WiFi.setAutoConnect(false);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid.c_str(), password.c_str());
  IPAddress IP = WiFi.softAPIP();

  server.begin();
  Serial.println(IP);
  tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
  tft.println(IP);

  msWiFiLoop = millis();
  wifiLoop();
  wifiDisconnect();
  tft.fillScreen(TFT_BLACK);
}

void wifiDisconnect(void) {
  Serial.print(F("Stopping AP ")); Serial.println(ssid);
  WiFi.softAPdisconnect(true);
}
