#include "rc6_protocol.h"
#include "esphome/core/log.h"

namespace esphome {
namespace remote_base {

static const char *TAG = "remote.rc6";

static const uint8_t T1_US =444;
static const uint8_t T2_US =889;
static const uint8_t HDR_MARK_US =2666;
static const uint8_t HDR_SPACE_US =889;
static const uint8_t RPT_LENGTH_US = 46000;
static const uint32_t TOGGLE_MASK = 0x10000UL;
static const uint32_t TOGGLE_MASK_36 = 0x8000;

void RC6Protocol::encode(RemoteTransmitData *dst, const RC6Data &data) {
  static bool TOGGLE = false;
  dst->set_carrier_frequency(36000);
  
  uint8_t nbits = data.nbits;
  uint64_t out_data = data.data;
  
  // Toggle press depress bit
  if(nbits == 36 && TOGGLE) {
    out_data = out_data ^ TOGGLE_MASK_36;
  } else if(TOGGLE){
    out_data = out_data ^ TOGGLE_MASK;
  }
  // RC6 Leader
  dst->mark(HDR_MARK_US);
  dst->space(HDR_SPACE_US);
  // Start Bit
  dst->mark(T1_US);
  dst->space(T1_US);

  //Data
  for (uint32_t i = 1, mask = 1UL << (nbits - 1); mask != 0; i++, mask >>= 1) {
      int t = (i == 4) ? (T2_US) : (T1_US);
      if (out_data & mask) {
        dst->mark(t);
        dst->space(t);
      } else {
        dst->space(t);
        dst->mark(t);      
      }
    }
  // End with signal free
  dst->space(HDR_MARK_US);

  TOGGLE = !TOGGLE;
}
optional<RC6Data> RC6Protocol::decode(RemoteReceiveData src) {
  RC6Data out{
      .data = 0,
      .nbits = 0,
  };

  uint8_t nbits;
  uint64_t out_data = 0;
  uint8_t used = 0;
  uint8_t offset = 1;

  // RC6 starts with mark with time HDR_MARK_US and space with time HDR_SPACE_US
 if (!(src.expect_mark(HDR_MARK_US) && src.expect_space(HDR_SPACE_US)))
  return {};

  // space mark -> 0
  // mark space -> 1
  // Format after leader:
  // address 4 bits
  // trailer bit 2t (toggel)
  // control 8 bit
  // info 8 bit   

  // Nr. of bits can be 24 or 36(xbox) so get the size
  uint8_t NBITS = src.size;

  for (int bit = NBITS - 2; bit >= 0; bit--) {
    // 4th bit is trailer bit with bitlength 2T
    int t = (bit == 4) ? (T2_US) : (T1_US);
    if (src.expect_space(t) && src.expect_mark(t)) {
      out_data |= 0 << bit;
    } else if (src.expect_mark(t) && src.expect_space(t)) {
      out_data |= 1 << bit;
    } else {
      return {};
    }
	}
  out.data = out_data;
  out.nbits = nbits;
  return out;
}
void RC6Protocol::dump(const RC6Data &data) {
  ESP_LOGD(TAG, "Received RC6: data=0x%02X, nbits=0x%02X", data.data, data.nbits);
}

}  // namespace remote_base
}  // namespace esphome
