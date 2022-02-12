#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <DS1820.h>

#include "line_graph.hpp"
#include "network.hpp"

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);

//---------------------------------------------------------------------------------
class Unit {
public:
  Unit(uint8_t pin, uint8_t lineSize, uint8_t lineHeight) : sensor(pin), line(lineSize, lineHeight) {
    clear();
  }

  void setup () {
    sensor.setup();
    if (sensor.isOk()) {
      exists = true;
    }
  }

  void loop() {
    sensor.loop();

    if (sensor.isDataReady()) {
      line.add(sensor.getInt16MilliCelsius_Temperature());
      strncpy(temp, sensor.getTextTemperature(), sizeof(temp));
    } else {
      line.addEmptyValue();
      clear();
    }
  }

  void clear() {
    strncpy(temp, "----", sizeof(temp));
  }

  DS1820::Sensor sensor;
  LineGraph line;
  bool exists = false;
  char temp[10];
};
//---------------------------------------------------------------------------------

#define CHAR_HEIGHT 10
#define CHAR_WIDTH 6
#define CHAR_FONT u8g2_font_6x10_mr
#define LINE_OFFSET (CHAR_WIDTH * 6)
#define LINE_WIDTH (128 - LINE_OFFSET)

Unit units[] = {
  {D5, LINE_WIDTH, CHAR_HEIGHT - 1},
  {D6, LINE_WIDTH, CHAR_HEIGHT - 1},
  {D7, LINE_WIDTH, CHAR_HEIGHT - 1},
  {D8, LINE_WIDTH, CHAR_HEIGHT - 1}
};

//---------------------------------------------------------------------------------
void setup() {
  u8g2.begin();
  Serial.begin(115200);
  while (!Serial);
  Serial.println("initializing devices...");

  for (auto& unit : units) {
    unit.setup();
  }

  u8g2.firstPage();
  do {
    u8g2.setFont(CHAR_FONT);
    u8g2.drawStr(0, CHAR_HEIGHT,   "WiFi AP MODE ");
    u8g2.drawStr(0, CHAR_HEIGHT*2, AP_NAME);
    u8g2.drawStr(0, CHAR_HEIGHT*3, "waiting for connection");
  } while ( u8g2.nextPage() );

  network::setup();
}

//---------------------------------------------------------------------------------
void loop () {
  network::loop();
  String ipAddr = "IP: " + network::localIP();

  network::NetResponse response;
  for (auto& unit : units) {
    unit.loop(); // measure temp is here
    if (unit.exists) {
      response.add("{\"P" + String(unit.sensor.pin) + "\":", true);
      if (unit.sensor.isDataReady()) {
        response.add(unit.temp);
      } else {
        response.add("null");
      }
      response.add("}");
    }
  }
  network::setResponse(response);

  u8g2.firstPage();
  do {
    u8g2.setFont(CHAR_FONT);
    u8g2.drawStr(0, CHAR_HEIGHT, ipAddr.c_str());

    uint8_t lineNum = 2;
    for (auto& unit : units) {
      u8g2.drawStr(0, CHAR_HEIGHT * lineNum, unit.temp);
      if (unit.exists) {
        for (uint8_t i = 0; i < unit.line.size ; i++) {
          if(unit.line.valueExists(i)){
            u8g2.drawPixel(LINE_OFFSET + i, (CHAR_HEIGHT * lineNum) - unit.line.getValue(i));
          }
        }
      }
      lineNum++;
    }
  } while ( u8g2.nextPage() );

  delay(1000);
}
