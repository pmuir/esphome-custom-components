#include "Wire.h"
#include "esphome.h"
#include "LeafSens.h"

#define address 0x61 // default address for the sensor
#define wait_period 300 // the time in ms to wait to read the data after requesting a new reading - this is stated by the docs as 100ms, but in the code it's either 300ms or 400ms. 300ms seems to work.

// The various states the component can be in
enum LeafWetnessSensorState { 
    REQUEST, // Request a new measurement
    WAITING, // Waiting for the measurement
    READY, // Ready to request the measurement value
    READ, // Requesting the measurement value
    IDLE // There is no request in progress
};

/*
 * An ESPHome component for the I2C leaf sensor made by Tinovi. 
 * It's based on their Arduino example code which is in LeadArduioI2C but replaces the various delays
 * they use with a very basic state machine, and uses millis() to work out when the sensor is ready
 * to avoid blocking the loop.
 * 
 * It publishes both the temperature reading in (degrees celsius) and the wetness reading (%).
 *
 * To use it, enable the I2C bus:
 *
 * ```
 * i2c:
 *     scan: true
 *     sda: GPIO32
 *     scl: GPIO33
 * ```
 *
 * Add the includes under `esphome:`
 * 
 * ```
 * includes:
 *   - custom_components/tinovi-leaf-sensor/tinovi_leaf_wetness.h
 *   - custom_components/tinovi-leaf-sensor/LeafArduinoI2c/LeafSens.h
 * ```
 * 
 * and the custom component:
 * 
 * ```
 * sensor:
 *   - platform: custom
 *     lambda: |-
 *       auto sensor = new LeafWetness(5000);
 *       App.register_component(sensor);
 *       return {sensor->temperature_sensor, sensor->wetness_sensor};
 * 
 *     sensors:
 *       - name: "Temperature Sensor"
 *         unit_of_measurement: °C
 *         accuracy_decimals: 1
 *       - name: "Wetness Sensor"
 *         unit_of_measurement: "%"
 *         accuracy_decimals: 1
 */
class LeafWetness : public PollingComponent, public Sensor {
    public:

    Sensor *temperature_sensor = new Sensor(); // The ESPHome temperature sensor
    Sensor *wetness_sensor = new Sensor(); // The ESPHome wetness sensor

    unsigned long startRequest = 0UL; // The time the REQUEST state is entered
    LeafWetnessSensorState state = IDLE; // The sensor state machine

    LeafWetness(int pollingInterval) : PollingComponent(pollingInterval) {}

    // Define when to start the component
    float get_setup_priority() const override { 
        return esphome::setup_priority::BUS; 
    }

    void setup() override {
        // This will be called by App.setup()
        // It includes a call to Wire.begin()
    }
    void update() override {
        // This is called every pollingInterval to get a new value
        // The work is done in loop()
        state = REQUEST; // Put the sensor into the REQUEST state to start a measurement
    }

    void loop() {
        // The state machine
        ESP_LOGVV("tinovi_leaf_wetness", "STATE: %d", state);
        switch(state) {
            case REQUEST:
                // Tell the sensor to start a measurement
                Wire.beginTransmission(address);
                Wire.write(REG_READ_ST);
                Wire.endTransmission();
                state = WAITING;
                startRequest = millis();
                break;
            case WAITING:
                // Wait for the measurement to be complete
                if (wait_period < millis() - startRequest) {
                    state = READY;
                }
                break;
            case READY:
                // Tell the sensor to send the measurement
                Wire.beginTransmission(address); 
                Wire.write(REG_DATA);
                Wire.endTransmission();
                Wire.requestFrom(address, 4);
                state = READ;
            case READ:
                // Read the measurement and publish it
                if(Wire.available() == 4){
                    for (int k = 0; k < 2; k++){
                        int16_t ret;
                        byte *pointer = (byte *)&ret;
                        pointer[0] = Wire.read();
                        pointer[1] = Wire.read();
                        float value = ret / 100.0;
                        switch (k) {
                            case 0:
                                wetness_sensor->publish_state(value);
                                break;
                            case 1:
                                temperature_sensor->publish_state(value);
                                break;
                        }
                    }
                    state = IDLE;
                }
                break;
        }
    } 

};
