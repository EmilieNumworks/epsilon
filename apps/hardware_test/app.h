#ifndef HARDWARE_TEST_APP_H
#define HARDWARE_TEST_APP_H

#include <escher.h>
#include "keyboard_test_controller.h"
#include "screen_test_controller.h"
#include "lcd_data_test_controller.h"
#include "led_test_controller.h"
#include "battery_test_controller.h"
#include "serial_number_controller.h"
#include "vblank_test_controller.h"

class AppsContainer;

namespace HardwareTest {

class App : public ::App {
public:
  class Snapshot : public ::App::Snapshot {
  public:
    App * unpack(Container * container) override;
    Descriptor * descriptor() override;
  };
private:
  class WizardViewController : public BankViewController {
  public:
    WizardViewController(Responder * parentResponder);
    int numberOfChildren() override;
    ViewController * childAtIndex(int i) override;
    bool handleEvent(Ion::Events::Event event) override;
  private:
    KeyboardTestController m_keyboardController;
    ScreenTestController m_screenTestController;
    LCDDataTestController m_lcdDataTestController;
    LEDTestController m_ledTestController;
    BatteryTestController m_batteryTestController;
    SerialNumberController m_serialNumberController;
    VBlankTestController m_vBlankTestController;
  };

  App(Container * container, Snapshot * snapshot);
  WizardViewController m_wizardViewController;
};

}

#endif

