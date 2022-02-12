#ifndef LIB_DS1820_HPP
#define LIB_DS1820_HPP

#include <OneWire.h>

namespace DS1820 {

#define DS1820_READ_INTERVAL 750 // is a MAX, may be faster

class Sensor {
 public:
  Sensor(uint8_t pin) : pin(pin), ds(pin) { }
  void setup();
  void loop();

  uint32_t lastConversationMillis = 0;

  int16_t getIntTemperature();
  int16_t getInt16MilliCelsius_Temperature();
  float getFloatTemperature();
  const char* getTextTemperature();
  bool isDataReady();
  bool isOk();

  const uint8_t pin;
 private:
  void startConversion(void);
  void readData(void);
  void getTextValue(char *buf, int len);
  void checkResult();
  bool error = false;
  void SerialPrintWithDeviceAddr(const char *str);

  OneWire ds;
  uint8_t addr[8];
  bool initialized = false;
  int16_t SignBit = 0;
  int16_t Tc_100 = 0;
  int16_t Whole = 0;
  int16_t Fract = 0;
  char temp[10] = { 0,0,0,0,0,0,0,0,0,0 }; // +125.625\0 max
  bool dataReady = false;
};
}  // namespace

#endif
