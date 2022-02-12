#include "network.hpp"

namespace network {

ESP8266WebServer Server;
AutoConnect Portal(Server);
AutoConnectConfig acConfig;

NetResponse metricsResponse;

//---------------------------------------------------------------------------------
void rootPage() {
  Server.send(200, "applicaion/json", metricsResponse.getText());
}

//---------------------------------------------------------------------------------
void setResponse(NetResponse& resp) {
  metricsResponse = resp;
}

//---------------------------------------------------------------------------------
void handleNotFound() {
  String message = "Server is running!\n\n";
  message += "URI: ";
  message += Server.uri();
  message += "\nMethod: ";
  message += (Server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += Server.args();
  message += "\n\n";
  message += "source code: https://github.com/kzub/distilmon\n\n";
  Server.send(200, "text/plain", message);
}

//---------------------------------------------------------------------------------
String localIP() {
  return WiFi.localIP().toString();
}

//---------------------------------------------------------------------------------
void setup() {
  Serial.println("network starting...");

  acConfig.title = AP_NAME;
  acConfig.apid = AP_NAME;
  acConfig.psk = AP_PASSWORD;
  acConfig.hostName = acConfig.apid;
  acConfig.autoReconnect = true;
  acConfig.reconnectInterval = 10;

  Portal.config(acConfig);
  Portal.onNotFound(handleNotFound);

  Server.on("/", rootPage);
  if (Portal.begin()) { // will freeze here until connected
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
  }
  Serial.println("network ok");
}

//---------------------------------------------------------------------------------
void loop() {
    Portal.handleClient();
}

} //namespace
