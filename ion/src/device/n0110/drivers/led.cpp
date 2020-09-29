#include <ion/led.h>
#include <ion/battery.h>
#include <ion/exam_mode_unprivileged.h>
#include <ion/usb.h>

namespace Ion {
namespace LED {

KDColor updateColorWithPlugAndCharge() {
  KDColor ledColor = getColor();
  if (Ion::ExamMode::FetchExamMode() == 0) { // If exam mode is on, we do not update the LED with the plugged/charging state
    if (USB::isPlugged()) {
      ledColor = Battery::isCharging() ? KDColorOrange : KDColorGreen;
    } else {
      ledColor = KDColorBlack;
    }
    setColor(ledColor);
  }
  return ledColor;
}

}
}
