#ifndef ION_DEVICE_N0110_CONFIG_INTERNAL_FLASH_H
#define ION_DEVICE_N0110_CONFIG_INTERNAL_FLASH_H

#include <shared/drivers/internal_flash.h>
#include <regs/regs.h>

namespace Ion {
namespace Device {
namespace InternalFlash {
namespace Config {

static constexpr int NumberOfForbiddenFirstSectors = 2;
static constexpr uint32_t StartAddress = 0x08000000;
static constexpr uint32_t EndAddress = 0x08010000;
static constexpr uint32_t TotalSize = EndAddress - StartAddress;
static constexpr int NumberOfSectors = 4;
static constexpr uint32_t SectorAddresses[NumberOfSectors+1] = {
  0x08000000, 0x08004000, 0x08008000, 0x0800C000,
  0x08010000
};

/* The Device is powered by a 2.8V LDO. This allows us to perform writes to the
 * Flash 32 bits at once. */
constexpr Regs::FLASH::CR::PSIZE MemoryAccessWidth = Regs::FLASH::CR::PSIZE::X32;
typedef uint32_t MemoryAccessType;

static constexpr uint32_t OTPStartAddress = 0x1FF07800;
static constexpr uint32_t OTPLocksAddress = 0x1FF07A00;
static constexpr int NumberOfOTPBlocks = 16;
static constexpr uint32_t OTPBlockSize = 0x20;
constexpr uint32_t OTPAddress(int block, int index) { return OTPStartAddress + block * OTPBlockSize + index * sizeof(uint32_t); };
constexpr uint32_t OTPLockAddress(int block) { return OTPLocksAddress + block; }

}
}
}
}

#endif
