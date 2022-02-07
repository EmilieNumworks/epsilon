#ifndef ION_DEVICE_INCLUDE_N0120_CONFIG_BOARD_H
#define ION_DEVICE_INCLUDE_N0120_CONFIG_BOARD_H

#include <stdint.h>
#include <stddef.h>

namespace Ion {
namespace Device {
namespace Board {

// RAM
constexpr uint32_t SRAMAddress = 0x24000000;
constexpr uint32_t SRAMLength = 0x40000; // 256kB

}
}
}

#endif

