#ifndef ION_DEVICE_KEYBOARD_H
#define ION_DEVICE_KEYBOARD_H

#include <ion/keyboard.h>
#include <ion.h>
#include "regs/regs.h"

namespace Ion {
namespace Keyboard {
namespace Device {

/*  Pin | Role              | Mode
 * -----+-------------------+--------------------
 *  PA0 | Keyboard row A    | Output, open drain
 *  PA1 | Keyboard row B    | Output, open drain
 *  PA2 | Keyboard row C    | Output, open drain
 *  PA3 | Keyboard row D    | Output, open drain
 *  PA4 | Keyboard row E    | Output, open drain
 *  PA5 | Keyboard row F    | Output, open drain
 *  PA6 | Keyboard row G    | Output, open drain
 *  PA7 | Keyboard row H    | Output, open drain
 *  PA8 | Keyboard row I    | Output, open drain
 *  PC0 | Keyboard column 1 | Input, pulled up
 *  PC1 | Keyboard column 2 | Input, pulled up
 *  PC2 | Keyboard column 3 | Input, pulled up
 *  PC3 | Keyboard column 4 | Input, pulled up
 *  PC4 | Keyboard column 5 | Input, pulled up
 *  PC5 | Keyboard column 6 | Input, pulled up
 */

void init();
void shutdown();

constexpr GPIO RowGPIO = GPIOA;
constexpr uint8_t numberOfRows = 9;
constexpr uint8_t RowPins[numberOfRows] = {0, 1, 2, 3, 4, 5, 6, 7, 8};

constexpr GPIO ColumnGPIO = GPIOC;
constexpr uint8_t numberOfColumns = 6;
constexpr uint8_t ColumnPins[numberOfColumns] = {0, 1, 2, 3, 4, 5};

inline uint8_t rowForKey(Key key) {
  return (int)key/numberOfColumns;
}
inline uint8_t columnForKey(Key key) {
  return (int)key%numberOfColumns;
}

inline void activateRow(uint8_t row) {
  /* In open-drain mode, a 0 in the register drives the pin low, and a 1 lets
   * the pin floating (Hi-Z). So we want to set the current row to zero and all
   * the others to 1. */
  uint16_t rowState = ~(1<<row);

  // TODO: Assert pin numbers are sequentials and dynamically find 8 and 0
  Device::RowGPIO.ODR()->setBitRange(numberOfRows-1, 0, rowState);

  // TODO: 100 us seems to work, but wasn't really calculated
  Timing::usleep(100);
}

inline bool columnIsActive(uint8_t column) {
  return !(Device::ColumnGPIO.IDR()->getBitRange(column,column));
}

}
}
}

#endif
