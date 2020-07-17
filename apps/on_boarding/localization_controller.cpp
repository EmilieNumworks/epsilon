#include "localization_controller.h"
#include <algorithm>
#include <apps/apps_container.h>

namespace OnBoarding {

bool LocalizationController::handleEvent(Ion::Events::Event event) {
  if (Shared::LocalizationController::handleEvent(event)) {
    if (mode() == Mode::Language) {
      setMode(Mode::Country);
      viewWillAppear();
    } else {
      assert(mode() == Mode::Country);
      AppsContainer * appsContainer = AppsContainer::sharedAppsContainer();
      if (appsContainer->promptController()) {
        Container::activeApp()->displayModalViewController(appsContainer->promptController(), 0.5f, 0.5f);
      } else {
        appsContainer->switchTo(appsContainer->appSnapshotAtIndex(0));
      }
    }
    return true;
  }
  if (event == Ion::Events::Back) {
    if (mode() == Mode::Country) {
      setMode(Mode::Language);
      viewWillAppear();
    }
    return true;
  }
  return false;
}

}
