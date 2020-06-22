#ifndef SETTING_COUNTRY_CONTROLLER_H
#define SETTING_COUNTRY_CONTROLLER_H

#include <escher.h>
#include "../../shared/country_controller.h"

namespace Settings {

class CountryController : public Shared::CountryController {
public:
  CountryController(Responder * parentResponder, KDCoordinate verticalMargin);
  bool handleEvent(Ion::Events::Event event) override;
};

}

#endif
