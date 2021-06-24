#ifndef ION_DEVICE_N0110_SHARED_DRIVERS_CONFIG_PERSISTING_BYTES_H
#define ION_DEVICE_N0110_SHARED_DRIVERS_CONFIG_PERSISTING_BYTES_H

#include <ion/src/device/shared/drivers/config/persisting_bytes.h>

namespace Ion {
namespace Device {
namespace PersistingBytes {
namespace Config {

// PersistingBytes buffer is filled with: 0x00, 0xFF, 0xFF,..., 0xFF
#define PERSISTING_BYTES_BUFFER_CONTENT byte32KFirst, byte32K

constexpr static int BufferSize = 64*1024;

}
}
}
}

#endif
