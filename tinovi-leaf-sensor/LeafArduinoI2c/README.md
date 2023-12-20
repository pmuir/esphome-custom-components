# Tinovi Leaf-I2C Leaf wetness and Temperature i2c sensor Arduino library

## Interfacing from Arduino
**WARNING!!! device supports 2.6v-5v voltage levels**


###wiring to Arduiono:

Arduiono pin #3V3 - sensor red (2.8v-5v)

Arduiono pin #A4 - sensor green (SDA)

Arduiono pin #A5 - sensor white (SCL)

Arduiono pin #GND - sensor black (GND)

pin #GND - shield (GND)

**SDA and SCL lines requires pull-up resitors to 3.3v line, we recommend to use 1.8K resistors, because of long wiring to i2c sensor.**

### API
```
LeafSens();
  //pass i2c addres of sensor, default 0x61
  int init(int address);
  // update i2c address of sesnor
  int newAddress(byte newAddr);
  // hold sesnor in air or put in dry soil and call  (offset Wet=0%)
  int calibrationAir();
  // submerge sesnor in the water or soil with water (offset Wet = 100%)
  int calibrationWater();
  //initate reading, then need to wait for 100ms to let reading to finish
  int newReading();
  float getWet();
  float getTemp();
  //get all values, supply float[2] , return 0-Wet;1-Temp
  void getData(float retVal[]);
```




### Get software

This sample software demonstrates hot to read data from sensor.

Sensor default I2C address is 0x61.

Download Arduino library from [there.](https://github.com/tinovi/LeafArduinoI2c)

<a href="https://tinovi.com/tinovi-shop/"> Shop </a>

