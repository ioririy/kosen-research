#include <HX711_ADC.h>

#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

////pins:
////1
//const int HX711_dout1 = 4; //mcu > HX711 dout pin
//const int HX711_sck1 = 5; //mcu > HX711 sck pin
////2
//const int HX711_dout2 = 25; //mcu > HX711 dout pin
//const int HX711_sck2 = 26; //mcu > HX711 sck pin
////3
//const int HX711_dout3 = 18; //mcu > HX711 dout pin
//const int HX711_sck3 = 19; //mcu > HX711 sck pin
//1
const int HX711_dout1 = 14; //mcu > HX711 dout pin
const int HX711_sck1 = 12; //mcu > HX711 sck pin
//2
const int HX711_dout2 = 32; //mcu > HX711 dout pin
const int HX711_sck2 = 33; //mcu > HX711 sck pin
//3
const int HX711_dout3 = 25; //mcu > HX711 dout pin
const int HX711_sck3 = 26; //mcu > HX711 sck pin

//HX711 constructor:
HX711_ADC LoadCell1(HX711_dout1, HX711_sck1);
HX711_ADC LoadCell2(HX711_dout2, HX711_sck2);
HX711_ADC LoadCell3(HX711_dout3, HX711_sck3);

const int calVal_eepromAdress = 0;
const int tareOffsetVal_eepromAdress = 4;
unsigned long t = 0;




void setup() {
  Serial.begin(115200); delay(10);
  Serial.println();
  Serial.println("Starting...");

  LoadCell1.begin();
  LoadCell2.begin();
  LoadCell3.begin();
  
  //LoadCell.setReverseOutput();
  float calibrationValue1; // calibration value (see example file "Calibration.ino")
  calibrationValue1 = 442.27; // uncomment this if you want to set the calibration value in the sketch
  float calibrationValue2; // calibration value (see example file "Calibration.ino")
  calibrationValue2 = 467.60; // uncomment this if you want to set the calibration value in the sketch
  float calibrationValue3; // calibration value (see example file "Calibration.ino")
  calibrationValue3 = 463.33; // uncomment this if you want to set the calibration value in the sketch

#if defined(ESP8266)|| defined(ESP32)
  EEPROM.begin(512);
#endif

  //EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch the calibration value from eeprom

  //restore the zero offset value from eeprom:
  long tare_offset = 0;
  EEPROM.get(tareOffsetVal_eepromAdress, tare_offset);
  LoadCell1.setTareOffset(tare_offset);
  LoadCell2.setTareOffset(tare_offset);
  LoadCell3.setTareOffset(tare_offset);
  boolean _tare = true; //set this to false as the value has been resored from eeprom
  //boolean _tare = false; //set this to false as the value has been resored from eeprom

  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  LoadCell1.start(stabilizingtime, _tare);
  if (LoadCell1.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell1.setCalFactor(calibrationValue1); // set calibration value (float)
    Serial.println("Startup is complete");
  }
  
  LoadCell2.start(stabilizingtime, _tare);
  if (LoadCell2.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell2.setCalFactor(calibrationValue2); // set calibration value (float)
    Serial.println("Startup is complete");
  }
  
  LoadCell3.start(stabilizingtime, _tare);
  if (LoadCell3.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell3.setCalFactor(calibrationValue3); // set calibration value (float)
    Serial.println("Startup is complete");
  }
}




float i1 = 0;
float i2 = 0;
float i3 = 0;
float sum = 0;
void loop() {
  const int serialPrintInterval = 50; //increase value to slow down serial print activity
  //const int serialPrintInterval = 1000; //increase value to slow down serial print activity
  
  // check for new data/start next conversion:
  if (LoadCell1.update() && LoadCell2.update() && LoadCell3.update()) {
    if (millis() > t + serialPrintInterval) {
      i1 = LoadCell1.getData();
      i2 = LoadCell2.getData();
      i3 = LoadCell3.getData();
      t = millis();

//      Serial.print("LCD1 : ");
//      Serial.println(i1);
//      Serial.print("LCD2 : ");
//      Serial.println(i2);
//      Serial.print("LCD3 : ");
//      Serial.println(i3);
//      Serial.print("合計  :");
//      Serial.println(i1+i2+i3);
      sum = i1 + i2 + i3;
      Serial.println(String() + i1 + "," + i2 + "," + i3 + "," + sum);
//      Serial.println(String() + sum);
    }
  }

  // receive command from serial terminal, send 't' to initiate tare operation:
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') {
      refreshOffsetValueAndSaveToEEprom();
    }
    if (inByte == 'w') {
//      Serial.println(sum);
    }
  }
}

// zero offset value (tare), calculate and save to EEprom:
void refreshOffsetValueAndSaveToEEprom() {
  long _offset[3] = {0,0,0};
  Serial.println("Calculating tare offset value...");
  LoadCell1.tare(); // calculate the new tare / zero offset value (blocking)
  _offset[0] = LoadCell1.getTareOffset(); // get the new tare / zero offset value
  LoadCell2.tare(); // calculate the new tare / zero offset value (blocking)
  _offset[1] = LoadCell2.getTareOffset(); // get the new tare / zero offset value
  LoadCell3.tare(); // calculate the new tare / zero offset value (blocking)
  _offset[2] = LoadCell3.getTareOffset(); // get the new tare / zero offset value
  
  EEPROM.put(tareOffsetVal_eepromAdress, _offset[0]); // save the new tare / zero offset value to EEprom
  EEPROM.put(tareOffsetVal_eepromAdress, _offset[1]); // save the new tare / zero offset value to EEprom
  EEPROM.put(tareOffsetVal_eepromAdress, _offset[2]); // save the new tare / zero offset value to EEprom
#if defined(ESP8266) || defined(ESP32)
  EEPROM.commit();
#endif
  LoadCell1.setTareOffset(_offset[0]); // set value as library parameter (next restart it will be read from EEprom)
  LoadCell2.setTareOffset(_offset[1]); // set value as library parameter (next restart it will be read from EEprom)
  LoadCell3.setTareOffset(_offset[2]); // set value as library parameter (next restart it will be read from EEprom)
//  Serial.print("New tare offset value:");
//  Serial.print(_offset);
//  Serial.print(", saved to EEprom adr:");
//  Serial.println(tareOffsetVal_eepromAdress);
  Serial.println("compleate");
}
