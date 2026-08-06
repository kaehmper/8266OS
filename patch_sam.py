import re

with open('NumberStationOS/ESP8266SAM.cpp', 'r') as f:
    content = f.read()

new_content = content.replace(
'''#include <Arduino.h>
#include "ESP8266SAM.h"''',
'''#include <Arduino.h>
#include "ESP8266SAM.h"
#include "WiFiController.h"

extern bool isBroadcasting;'''
)

new_content = new_content.replace(
'''void ESP8266SAM::OutputByte(unsigned char b)
{
  // Upsample from unsigned 8 bits to signed 16 bits
  int16_t sample[2];
  sample[0] = b;
  sample[0] = (((int16_t)(sample[0] & 0xff)) - 128) << 8;
  sample[1] = sample[0];
  while (!output->ConsumeSample(sample)) yield();
}''',
'''void ESP8266SAM::OutputByte(unsigned char b)
{
  if (!isBroadcasting) return;

  // Upsample from unsigned 8 bits to signed 16 bits
  int16_t sample[2];
  sample[0] = b;
  sample[0] = (((int16_t)(sample[0] & 0xff)) - 128) << 8;
  sample[1] = sample[0];

  while (isBroadcasting && !output->ConsumeSample(sample)) {
      WiFiController::getInstance().update();
      yield();
  }
}'''
)

with open('NumberStationOS/ESP8266SAM.cpp', 'w') as f:
    f.write(new_content)

print("Patch applied.")
