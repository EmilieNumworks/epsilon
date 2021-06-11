#ifndef APPS_PROBABILITY_CONTROLLERS_CATEGORICAL_TYPE_CONTROLLER_H
#define APPS_PROBABILITY_CONTROLLERS_CATEGORICAL_TYPE_CONTROLLER_H

#include <escher/container.h>
#include <escher/highlight_cell.h>
#include <escher/message_table_cell_with_chevron.h>
#include <escher/responder.h>
#include <escher/stack_view_controller.h>
#include <ion/events.h>

#include "probability/gui/selectable_cell_list_controller.h"

namespace Probability {

class InputGoodnessController;
class InputHomogeneityController;

constexpr static int k_numberOfCategoricalCells = 2;

class CategoricalTypeController : public SelectableCellListPage<Escher::MessageTableCellWithChevron,
                                                                k_numberOfCategoricalCells> {
public:
  CategoricalTypeController(Escher::StackViewController * parent,
                            InputGoodnessController * inputGoodnessController,
                            InputHomogeneityController * inputHomogeneityController);
  ViewController::TitlesDisplay titlesDisplay() override { return ViewController::TitlesDisplay::DisplayLastTitles; }
  const char * title() override { return "x2-test"; }
  void didBecomeFirstResponder() override;
  bool handleEvent(Ion::Events::Event event) override;

private:
  constexpr static int k_indexOfGoodnessCell = 0;
  constexpr static int k_indexOfHomogeneityCell = 1;

  InputGoodnessController * m_inputGoodnessController;
  InputHomogeneityController * m_inputHomogeneityController;
};

}  // namespace Probability
#endif /* APPS_PROBABILITY_CONTROLLERS_CATEGORICAL_TYPE_CONTROLLER_H */