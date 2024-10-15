#include "Wire.h"
#include "esphome.h"
#define address 0x74 // Default address for the sensor
#define request_wait_period 50 // Time to wait for a measurement
#define read_wait_period 20 // Time to wait for a measurement

// The various states the component can be in
enum Sen0590SensorState { 
    REQUEST, // Request a new measurement
    READY, // Ready to request the measurement value
    READ, // Requesting the measurement value
    IDLE // There is no request in progress
};

/*
 * An ESPHome component for the Laser Ranging Sensor (4m) which has the SKU SEN0590 made by DFRobot. 
 * It's based on their Arduino example code which on their wiki 
 * https://wiki.dfrobot.com/Laser_Ranging_Sensor_4m_SKU_SEN0590 but replaces the various delays
 * they use with a very basic state machine, and uses millis() to work out when the sensor is ready
 * to avoid blocking the loop.
 * 
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
 *   - custom_components/dfrobot-sen0590/sen0590.h
 * ```
 * 
 * and the custom component:
 * 
 * ```
 * sensor:
 *   - platform: custom
 *     lambda: |-
 *       auto sensor = new Sen0590(5000);
 *       App.register_component(sensor);
 *       return {sensor};
 * 
 *   sensors:
 *     name: Distance
 *     id: distance
 *     unit_of_measurement: mm
 *     accuracy_decimals: 0
 *     device_class: distance
 *     state_class: measurement
 * ```
 * 
 * The precision on this sensor is dependent on what you are measuring the distance towards (as it
 * depends what the laser can bounce off) so using some filters on the raw value is useful.
 */
class Sen0590 : public PollingComponent, public Sensor {
    public:
    // constructor
    Sen0590(int pollingInterval) : PollingComponent(pollingInterval) {}

    unsigned long startRequest = 0UL; // The time the REQUEST state is entered
    unsigned long startRead = 0UL; // The time the READ state is entered
    Sen0590SensorState state = IDLE; // The sensor state machine

    float get_setup_priority() const override { return esphome::setup_priority::BUS; }

    void setup() override {
        // This will be called by App.setup()
        // ESPHome calls Wire.begin()
    }
    void update() override {
        state = REQUEST;
    }

    void loop() override {
        ESP_LOGVV("sen0590", "STATE: %d", state);
        switch(state) {
            // Request a measurement is made
            case REQUEST:
                Wire.beginTransmission(address);
                Wire.write(0x10);
                Wire.write(0xB0);
                Wire.endTransmission();
                state = READY;
                startRequest = millis();
                break;
            case READY:
                // Wait for the measurement to be ready
                if (request_wait_period > millis() - startRequest) {
                    break;
                }
                // Tell the sensor to send the measurement
                Wire.beginTransmission(address);
                Wire.write(0x02);
                if (Wire.endTransmission() != 0) {
                    return;
                }
                state = READ;
                startRead = millis();
                break;      
            case READ:
                // Wait for the measurement to be ready to read
                if (read_wait_period > millis() - startRead) {
                    break;
                }
                // Read the measurement and publish it
                Wire.requestFrom(address, 2);
                int buf[2] = { 0 };
                for (int i = 0; i < 2; i++) {
                    buf[i] = Wire.read();
                }
                int distance = (buf[0] * 0x100 + buf[1] + 10);
                publish_state(distance);
                state = IDLE;
                break;
        }
    }
};
