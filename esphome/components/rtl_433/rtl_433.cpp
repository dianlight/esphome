#include "rtl_433.h"
#include "esphome/core/log.h"
#include <cinttypes>
#include <cstring>

namespace esphome {
namespace rtl_433 {

static const char *const TAG = "rtl_433";

RTL433Component *__instance;

void RTL433Component::rtl_433_Callback(char *message) {
  DynamicJsonDocument RFrtl_433_ESPdata(JSON_MSG_BUFFER);
  char *org_message = strdupa(message);
  DeserializationError err = deserializeJson(RFrtl_433_ESPdata, message);
  if (err) {
    ESP_LOGW(TAG, "deserializeJson() failed :  %s [%s]", err.c_str(), message);
    switch (err.code()) {
      case DeserializationError::InvalidInput:
        ESP_LOGW(TAG, "deserializeJson() Invalid JSON input!");
        break;
      case DeserializationError::NoMemory:
        ESP_LOGW(TAG, "deserializeJson() Not enough memory");
        break;
      default:
        ESP_LOGW(TAG, "deserializeJson() Deserialization failed");
        break;
    }
  } else {
    if (RFrtl_433_ESPdata.containsKey("id")) {
      ESP_LOGI(TAG, "Parserable: %s", org_message);
      if (__instance->includes_.empty() ||
          std::find(__instance->includes_.begin(), __instance->includes_.end(),
                    RFrtl_433_ESPdata["id"].as<std::string>()) != __instance->includes_.end()) {
        /* Data Sensor Enrichment*/
        if (strcmp(RFrtl_433_ESPdata["protocol"], "Efergy e2 classic") == 0) {
          if (RFrtl_433_ESPdata["current"].is<double>()) {
            __instance->current_sensor_->publish_state(RFrtl_433_ESPdata["current"].as<double>());
            __instance->power_sensor_->publish_state(RFrtl_433_ESPdata["current"].as<double>() * 220.0);
            __instance->battery_sensor_->publish_state(RFrtl_433_ESPdata["battery_ok"].as<int>() * 100);
          }
        }
      }
    } else {
      ESP_LOGW(TAG, "Unparserable: %s", org_message);
    }
  }
}

void RTL433Component::setup() {
  // this->spi_setup();
  rf.initReceiver(RF_MODULE_RECEIVER_GPIO, RF_MODULE_FREQUENCY);
  __instance = this;
  rf.setCallback(&RTL433Component::rtl_433_Callback, this->messageBuffer, JSON_MSG_BUFFER);
  rf.enableReceiver();
  // rf.getModuleStatus();
}

void RTL433Component::loop() { rf.loop(); }

void RTL433Component::dump_config() {
  ESP_LOGCONFIG(TAG, "rtl_433:");
  ESP_LOGCONFIG(TAG, " RadioModule: %s", STR_MODULE);
  ESP_LOGCONFIG(TAG, " Frequency: %d", RF_MODULE_FREQUENCY);
  for (auto &afilter : this->includes_) {
    ESP_LOGCONFIG(TAG, "  Filter Device: %s", afilter.c_str());
  }
  LOG_SENSOR("", "Current", this->current_sensor_);
  LOG_SENSOR("", "Power", this->power_sensor_);
  LOG_SENSOR("", "Battery", this->battery_sensor_);
}

}  // namespace rtl_433
}  // namespace esphome
