#include "efergy_e2_classic_protocol.h"
#include "esphome/core/log.h"

namespace esphome {
namespace remote_base {

static const char *const TAG = "remote.efergy_e2_classic";

// Protocol timing constants from rtl_433
static const uint32_t BIT_SHORT_US = 64;
static const uint32_t BIT_LONG_US = 136;
static const uint32_t SYNC_US = 500;
static const uint32_t GAP_LIMIT_US = 200;

static uint8_t add_bytes(const uint8_t *bytes, uint8_t len) {
  uint16_t sum = 0;
  for (uint8_t i = 0; i < len; i++) {
    sum += bytes[i];
  }
  return sum & 0xFF;
}

void EfergyE2ClassicProtocol::encode(RemoteTransmitData *dst, const EfergyE2ClassicData &data) {
  uint8_t bytes[8];

  // Byte 0: Start bits (00)
  bytes[0] = 0x00;

  // Bytes 1-2: Device id (address)
  bytes[1] = data.address & 0xFF;
  bytes[2] = (data.address >> 8) & 0xFF;

  // Byte 3: Learn mode, sending interval and battery status
  bytes[3] = ((data.learn & 0x01) << 7) | ((data.battery & 0x01) << 6) | (((data.interval / 6 - 1) & 0x03) << 4);

  // Bytes 4-6: Current power consumption
  // Calculate the exponent and mantissa
  uint8_t exponent = 0;
  float current_val = data.current;

  // Find appropriate exponent (values between -3 and 4)
  for (exponent = 0; exponent < 20; exponent++) {
    float divisor = (1 << (15 - exponent));
    if (current_val * divisor < 65536.0f) {
      break;
    }
  }

  uint16_t current_adc = (uint16_t) (current_val * (1 << (15 - exponent)));
  bytes[4] = (current_adc >> 8) & 0xFF;
  bytes[5] = current_adc & 0xFF;
  bytes[6] = exponent;

  // Byte 7: Checksum
  bytes[7] = add_bytes(bytes, 7);

  dst->set_carrier_frequency(0);
  dst->reserve(64 * 2 + 2);  // 64 bits + sync

  // Send sync pulse
  dst->mark(SYNC_US);
  dst->space(GAP_LIMIT_US);

  // Send 8 bytes (64 bits)
  for (uint8_t byte_idx = 0; byte_idx < 8; byte_idx++) {
    for (int8_t bit = 7; bit >= 0; bit--) {
      if (bytes[byte_idx] & (1 << bit)) {
        // Bit 1: Long pulse
        dst->mark(BIT_LONG_US);
        dst->space(BIT_SHORT_US);
      } else {
        // Bit 0: Short pulse
        dst->mark(BIT_SHORT_US);
        dst->space(BIT_LONG_US);
      }
    }
  }
}

optional<EfergyE2ClassicData> EfergyE2ClassicProtocol::decode(RemoteReceiveData src) {
  EfergyE2ClassicData out{
      .address = 0,
      .learn = 0,
      .interval = 0,
      .battery = 0,
      .current = 0.0f,
  };

  uint8_t bytes[8] = {0};

  // Look for sync pulse
  if (!src.expect_mark(SYNC_US)) {
    return {};
  }
  if (!src.expect_space(GAP_LIMIT_US)) {
    return {};
  }

  // Read 64 bits (8 bytes)
  for (uint8_t byte_idx = 0; byte_idx < 8; byte_idx++) {
    uint8_t byte_val = 0;
    for (uint8_t bit = 0; bit < 8; bit++) {
      byte_val <<= 1;

      if (src.expect_mark(BIT_LONG_US) && src.expect_space(BIT_SHORT_US)) {
        // Bit 1
        byte_val |= 1;
      } else if (src.expect_mark(BIT_SHORT_US) && src.expect_space(BIT_LONG_US)) {
        // Bit 0
        byte_val |= 0;
      } else {
        return {};
      }
    }
    bytes[byte_idx] = byte_val;
  }

  // Align data if needed (search for start bits 0000 or 1111)
  uint8_t shift_count = 0;
  while ((bytes[0] & 0xF0) != 0xF0 && (bytes[0] & 0xF0) != 0x00 && shift_count < 8) {
    for (uint8_t i = 0; i < 7; i++) {
      bytes[i] <<= 1;
      bytes[i] |= (bytes[i + 1] & 0x80) >> 7;
    }
    bytes[7] <<= 1;
    shift_count++;
  }

  // Check if data needs to be inverted
  if (bytes[0] & 0xF0) {
    for (uint8_t i = 0; i < 8; i++) {
      bytes[i] = ~bytes[i];
    }
  }

  // Sanity check: reject if too many null bytes
  uint8_t zero_count = 0;
  for (uint8_t i = 0; i < 8; i++) {
    if (bytes[i] == 0) {
      zero_count++;
    }
  }
  if (zero_count > 5) {
    return {};
  }

  // Verify checksum
  uint8_t checksum = add_bytes(bytes, 7);
  if (checksum == 0) {
    return {};  // Reject all zeros
  }
  if (checksum != bytes[7]) {
    return {};
  }

  // Parse data
  out.address = bytes[2] << 8 | bytes[1];
  out.learn = (bytes[3] & 0x80) >> 7;
  out.interval = (((bytes[3] & 0x30) >> 4) + 1) * 6;
  out.battery = (bytes[3] & 0x40) >> 6;

  uint8_t fact = 15 - bytes[6];
  if (fact < 7 || fact > 23) {
    return {};  // Invalid exponent
  }

  out.current = (float) (bytes[4] << 8 | bytes[5]) / (1 << fact);

  return out;
}

void EfergyE2ClassicProtocol::dump(const EfergyE2ClassicData &data) {
  ESP_LOGI(TAG, "Received Efergy e2 Classic: address=0x%04X, battery=%d, current=%.2fA, interval=%ds, learn=%s",
           data.address, data.battery, data.current, data.interval, data.learn ? "YES" : "NO");
}

}  // namespace remote_base
}  // namespace esphome
