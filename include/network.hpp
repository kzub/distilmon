#ifndef LIB_NET_HPP
#define LIB_NET_HPP

#include <ESP8266WiFi.h>          // Replace with WiFi.h for ESP32
#include <ESP8266WebServer.h>     // Replace with WebServer.h for ESP32
#include <AutoConnect.h>

namespace network {

#define AP_NAME "TEMP-MON"
#define AP_PASSWORD ""
//----------------------------------------
class NetResponse {
public:
	void add (String text, bool elementStart = false){
		if(elementStart){
			if(empty){
				empty = false;
			} else {
				resp += ",";
			}
		}
		resp += text;
	}

	String getText () {
		return resp + "]";
	}
private:
	String resp = "[";
	bool empty = true;
};
//----------------------------------------
void rootPage();
void setup();
void loop();
String localIP();
void setResponse(NetResponse& resp);

} //namespace

#endif
