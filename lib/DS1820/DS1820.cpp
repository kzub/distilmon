#include "DS1820.h"

namespace DS1820 {
//--------------------------------------------------------------------------------
// PUBLIC
//--------------------------------------------------------------------------------
void Sensor::setup() {
  if (initialized) {
    return;
  }

  if (!ds.search(addr)) {
    ds.reset_search();
    error = true;
    SerialPrintWithDeviceAddr("search error");
    return;
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
    error = true;
    SerialPrintWithDeviceAddr("CRC is not valid");
    return;
  }

  if (addr[0] != 0x28) {
    error = true;
    SerialPrintWithDeviceAddr("not a DS18B20 family device");
    return;
  }

  SerialPrintWithDeviceAddr("OK");
  error = false;
  initialized = true;
  return;
}

//---------------------------------------------------------------
void Sensor::loop() {
  if (!initialized) {
    return;
  }

  auto now = millis();

  if (now - lastConversationMillis > DS1820_READ_INTERVAL) {
    if(lastConversationMillis != 0) { // skip first read on start without conversion
      readData();
    }
    // start conversation again
    startConversion();
    lastConversationMillis = millis();
  }
}

//---------------------------------------------------------------
bool Sensor::isDataReady() {
  return error == false && dataReady == true;
}
//---------------------------------------------------------------
bool Sensor::isOk() {
  return error == false;
}

//---------------------------------------------------------------
int16_t Sensor::getIntTemperature() {
  if (error) {
    return 0;
  }

  if (SignBit) {
    return -Whole;
  } else {
    return Whole;
  }
}


//---------------------------------------------------------------
int16_t Sensor::getInt16MilliCelsius_Temperature() {
  if (error) {
    return 0;
  }

  if (SignBit) {
    return -Tc_100;
  } else {
    return Tc_100;
  }
}

//---------------------------------------------------------------
float Sensor::getFloatTemperature() {
  if (error) {
    return 0;
  }

  if (SignBit) {
    return ((float)(-Tc_100)) / 100;
  } else {
    return ((float)(Tc_100)) / 100;
  }
}

//---------------------------------------------------------------
const char* Sensor::getTextTemperature() {
  if (error) {
    return "*";
  }
  // +125.0625  - 10 chars
  char *buf = temp;
  if (SignBit) {
    buf[0] = '-';
    buf = buf + 1;
  }

  char FractText[5];
  snprintf(FractText, 3, "%d", Fract);
  snprintf(temp, sizeof(temp) - 1, "%.2d.%c%c", Whole, FractText[0], FractText[1]);
  return temp;
}

//--------------------------------------------------------------------------------
// PRIVATE
//--------------------------------------------------------------------------------
void Sensor::SerialPrintWithDeviceAddr(const char *str) {
  Serial.printf("[DS1820] (pin: %d) ", pin);
  Serial.printf("%.2X:%.2X:%.2X:%.2X:%.2X:%.2X:%.2X:%.2X ", addr[0],addr[1],addr[2],addr[3],addr[4],addr[5],addr[6],addr[7]);
  Serial.println(str);
}

//---------------------------------------------------------------
void Sensor::startConversion(void) {
  uint8_t present = ds.reset();
  if (!present) {
    SerialPrintWithDeviceAddr("not present at conv");
    error = true;
    return;
  }

  ds.select(addr);
  ds.write(0x44, 1);  // start conversion, with parasite power on at the end
  // delay(1000);  // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
}

//---------------------------------------------------------------
void Sensor::readData(void) {
  uint8_t present = ds.reset();
  if (!present) {
    SerialPrintWithDeviceAddr("not present at read");
    error = true;
    return;
  }

  ds.select(addr);
  ds.write(0xBE);  // Read Scratchpad

  uint8_t data[12];
  for (uint8_t i = 0; i < 9; i++) {  // we need 9 bytes
    data[i] = ds.read();
  }

  uint8_t LowByte = data[0];
  uint8_t HighByte = data[1];
  int16_t TReading = (HighByte << 8) + LowByte;

  // calc temperature:
  SignBit = TReading & 0x8000;  // test most sig bit
  int32_t tmp = TReading;
  tmp *= 625;
  tmp /= 100;
  Tc_100 = (tmp & 0xFFFF) | SignBit;
  Whole = Tc_100 / 100;  // separate off the whole and fractional portions
  Fract = Tc_100 % 100;
  checkResult();
}

//---------------------------------------------------------------
void Sensor::checkResult() {
  if (Whole > 100 || Fract >= 100) {
    Serial.print("S:");
    Serial.print(SignBit);
    Serial.print(" T:");
    Serial.print(Tc_100);
    Serial.print(" F:");
    Serial.print(Fract);
    Serial.print(" W:");
    Serial.println(Whole);
    SignBit = 0;
    Tc_100 = 0;
    Fract = 0;
    Whole = 0;
    error = true;
    dataReady = false;
    SerialPrintWithDeviceAddr("read fail");
  } else {
    error = false;
    dataReady = true;
    // SerialPrintWithDeviceAddr("read ok");
  }
}

}  // namespace
