#pragma once

#include "esphome/core/component.h"
#include "remote_base.h"

namespace esphome {
namespace remote_base {

struct RC6Data {
  uint8_t data;
  uint8_t nbits;

  bool operator==(const RC6Data &rhs) const { return data == rhs.data && nbits == rhs.nbits; }
};

class RC6Protocol : public RemoteProtocol<RC6Data> {
 public:
  void encode(RemoteTransmitData *dst, const RC6Data &data) override;
  optional<RC6Data> decode(RemoteReceiveData src) override;
  void dump(const RC6Data &data) override;
};

DECLARE_REMOTE_PROTOCOL(RC6)

template<typename... Ts> class RC6Action : public RemoteTransmitterActionBase<Ts...> {
 public:
  TEMPLATABLE_VALUE(uint8_t, data)
  TEMPLATABLE_VALUE(uint8_t, nbits)
  void encode(RemoteTransmitData *dst, Ts... x) override {
    RC6Data data{};
    data.data = this->data_.value(x...);
    data.nbits = this->nbits_.value(x...);
    RC6Protocol().encode(dst, data);
  }
};

}  // namespace remote_base
}  // namespace esphome
