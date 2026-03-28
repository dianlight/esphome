#pragma once

#include "remote_base.h"

namespace esphome {
namespace remote_base {

struct EfergyE2ClassicData {
  uint16_t address;
  uint8_t learn;
  uint8_t interval;
  uint8_t battery;
  float current;

  bool operator==(const EfergyE2ClassicData &rhs) const {
    return address == rhs.address && learn == rhs.learn && interval == rhs.interval && battery == rhs.battery &&
           current == rhs.current;
  }
};

class EfergyE2ClassicProtocol : public RemoteProtocol<EfergyE2ClassicData> {
 public:
  void encode(RemoteTransmitData *dst, const EfergyE2ClassicData &data) override;
  optional<EfergyE2ClassicData> decode(RemoteReceiveData src) override;
  void dump(const EfergyE2ClassicData &data) override;
};

DECLARE_REMOTE_PROTOCOL(EfergyE2Classic)

template<typename... Ts> class EfergyE2ClassicAction : public RemoteTransmitterActionBase<Ts...> {
 public:
  TEMPLATABLE_VALUE(uint16_t, address)
  TEMPLATABLE_VALUE(uint8_t, learn)
  TEMPLATABLE_VALUE(uint8_t, interval)
  TEMPLATABLE_VALUE(uint8_t, battery)
  TEMPLATABLE_VALUE(float, current)

  void encode(RemoteTransmitData *dst, Ts... x) override {
    EfergyE2ClassicData data{};
    data.address = this->address_.value(x...);
    data.learn = this->learn_.value(x...);
    data.interval = this->interval_.value(x...);
    data.battery = this->battery_.value(x...);
    data.current = this->current_.value(x...);
    EfergyE2ClassicProtocol().encode(dst, data);
  }
};

}  // namespace remote_base
}  // namespace esphome
