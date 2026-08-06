import re

with open('NumberStationOS/ESP8266SAM.cpp', 'r') as f:
    content = f.read()

new_content = content.replace(
'''  // Say it!
  output = out;
  SetInput(input);
  SAMMain(OutputByteCallback, (void*)this);
  delete samdata;
  return true;''',
'''  // Say it!
  output = out;
  SetInput(input);
  SAMMain(OutputByteCallback, (void*)this);
  delete samdata;
  out->stop(); // Ensure audio finishes
  return true;'''
)

with open('NumberStationOS/ESP8266SAM.cpp', 'w') as f:
    f.write(new_content)

print("Patch 2 applied.")
