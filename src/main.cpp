#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <DS1820.h>

#include "line_graph.hpp"
#include "network.hpp"

//---------------------------------------------------------------------------------
class Unit {
public:
  Unit(String name, uint8_t pin, uint8_t lineSize, uint8_t lineHeight) : name(name), sensor(pin), line(lineSize, lineHeight) {
    clear();
  }

  void setup () {
    sensor.setup();
    if (sensor.isOk()) {
      exists = true;
    }
  }

  void loop(bool updateTempline) {
    sensor.loop();

    if (sensor.isDataReady()) {
      if (updateTempline) {
        line.add(sensor.getInt16MilliCelsius_Temperature());
      }
      strncpy(temp, sensor.getTextTemperature(), sizeof(temp));
    } else {
      if (updateTempline) {
        line.addEmptyValue();
      }
      clear();
    }
  }

  void clear() {
    strncpy(temp, "-----", sizeof(temp));
  }

  String name;
  DS1820::Sensor sensor;
  LineGraph line;
  bool exists = false;
  char temp[10];
};
//---------------------------------------------------------------------------------

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);
#define CHAR_HEIGHT 10
#define CHAR_WIDTH 6
#define CHAR_FONT u8g2_font_6x10_mr
#define LINE_OFFSET (CHAR_WIDTH * 6)
#define LINE_WIDTH (128 - LINE_OFFSET)
#define LINE_MARGIN 2
Unit units[] = {
  {"P1", D5, LINE_WIDTH, CHAR_HEIGHT},
  {"P2", D6, LINE_WIDTH, CHAR_HEIGHT},
  {"P3", D7, LINE_WIDTH, CHAR_HEIGHT},
  {"P4", D3, LINE_WIDTH, CHAR_HEIGHT}
};

#define LOOP_MEASURE_DELAY 2000
#define DISPLAY_SCALER 5 // LINE_WIDTH *  (60 / DISPLAY_SCALER * LOOP_MEASURE_DELAY/


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

  Serial.printf("Display capacity: %d minutes\n", DISPLAY_SCALER * (LINE_WIDTH / (60000 / LOOP_MEASURE_DELAY)));
}

//---------------------------------------------------------------------------------
bool tictoc = false;

uint32_t measureTempLoopMs = 0;
uint32_t timelineScalerCounter = DISPLAY_SCALER;
bool updateTempline = false;

void loop () {
  network::loop();

  // --------------------------------
  auto now = millis();
  if ((now - measureTempLoopMs) < LOOP_MEASURE_DELAY){
    return;
  }
  measureTempLoopMs = now;
  //---------------------------------
  updateTempline = false;
  timelineScalerCounter--;
  if (timelineScalerCounter == 0) {
    timelineScalerCounter = DISPLAY_SCALER;
    updateTempline = true;
  }

  String ipAddr = "IP: " + network::localIP() + (tictoc? " +" : " x");
  tictoc = tictoc ^ true;

  network::NetResponse response;
  for (auto& unit : units) {
    unit.loop(updateTempline); // measure temp is here
    if (unit.exists) {
      response.add("{\"" + String(unit.name) + "\":", true);
      if (unit.sensor.isDataReady()) {
        response.add(unit.temp);
      } else {
        response.add("null");
      }
      response.add("}");
    }
  }
  network::setResponse(response);

  //---------------------------------
  u8g2.firstPage();
  do {
    u8g2.setFont(CHAR_FONT);
    u8g2.drawStr(0, CHAR_HEIGHT, ipAddr.c_str());

    uint8_t lineNum = 2;
    for (auto& unit : units) {
      yield(); // helps with freezing
      u8g2.drawStr(0, ((CHAR_HEIGHT + LINE_MARGIN) * lineNum), unit.temp);
      if (unit.exists) {
        for (uint8_t i = 0; i < unit.line.size ; i++) {
          if(unit.line.valueExists(i)){
            u8g2.drawPixel(LINE_OFFSET + i, ((CHAR_HEIGHT + LINE_MARGIN) * lineNum) - unit.line.getValue(i));
          }
        }
      }
      lineNum++;
    }
  } while ( u8g2.nextPage() );
}
