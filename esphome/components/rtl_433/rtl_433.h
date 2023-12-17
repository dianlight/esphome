#pragma once

#include <utility>
#include <vector>
#include <rtl_433_ESP.h>

#include "esphome/core/component.h"
// #include "esphome/components/spi/spi.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/automation.h"

namespace esphome {
namespace rtl_433 {

class RTL433Component : public Component /*,
                        public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_HIGH,
                                              spi::CLOCK_PHASE_TRAILING, spi::DATA_RATE_2MHZ> */
{
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  void set_current_sensor(sensor::Sensor *current_sensor) { current_sensor_ = current_sensor; }
  void set_power_sensor(sensor::Sensor *power_sensor) { power_sensor_ = power_sensor; }
  void set_battery_sensor(sensor::Sensor *battery_sensor) { battery_sensor_ = battery_sensor; }
  void add_include_code(std::string _include) { includes_.emplace_back(_include); }
  std::vector<std ::string> includes_;

 protected:
  rtl_433_ESP rf;
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *power_sensor_{nullptr};
  sensor::Sensor *battery_sensor_{nullptr};
  char messageBuffer[JSON_MSG_BUFFER];
  static void rtl_433_Callback(char *message);
};

}  // namespace rtl_433
}  // namespace esphome
