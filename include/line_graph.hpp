#include <Arduino.h>

#define MAGIC_EMPTY_VALUE_BYTE 0xF1
#define MAGIC_EMPTY_VALUE ((MAGIC_EMPTY_VALUE_BYTE<<8)|MAGIC_EMPTY_VALUE_BYTE)

class LineGraph {
public:
	LineGraph (uint8_t _size, uint8_t _lineHeight) : size(_size), lineHeight(_lineHeight) {
		line = new uint16_t[size];
    memset(line, MAGIC_EMPTY_VALUE_BYTE, 2*size);
	}

	void add (uint16_t newValue) {
	  for (uint8_t i = 0; i < size - 1 ; i++) { // shift values left
	    line[i] = line[i + 1];
	  }
	  line[size - 1] = newValue;
    scaleDots();
	}

  void addEmptyValue () {
    add(MAGIC_EMPTY_VALUE);
  }

  bool valueExists(uint8_t index) {
    if (index > size - 1) {
      return false;
    }
    return line[index] != MAGIC_EMPTY_VALUE;
  }

  uint8_t getValue(uint8_t index) {
    if (index > size - 1) {
      return 0;
    }
    if (scaleHeight == 0) {
      return 0;
    }

    float a = line[index];
    float res = ((a - minValue) / scaleHeight);
    uint8_t ret = uint8_t(res * lineHeight);
    // Serial.print("a:");
    // Serial.print(a);
    // Serial.print(" minV:");
    // Serial.print(minValue);
    // Serial.print(" sH:");
    // Serial.print(scaleHeight);
    // Serial.print(" res:");
    // Serial.print(res);
    // Serial.print(" ret:");
    // Serial.println(ret);
    return ret;
  }

  const uint8_t size;

private:
  void scaleDots() {
    maxValue = 0;
    minValue = UINT16_MAX;

    for (uint8_t i = 0; i < size - 1 ; i++) {
      if (line[i] == MAGIC_EMPTY_VALUE) {
        continue;
      }
      if (line[i] > maxValue) {
        maxValue = line[i];
      }
      if (line[i] < minValue) {
        minValue = line[i];
      }
    }
    scaleHeight = maxValue - minValue;
  }

	uint16_t maxValue = UINT16_MAX;
	uint16_t minValue = 0;
  uint16_t scaleHeight = UINT16_MAX;
	uint16_t *line = NULL;
  const uint8_t lineHeight = 0;
};