#ifndef GRAPH_GRAPH_AXIS_PARAMETER_CONTROLLER_H
#define GRAPH_GRAPH_AXIS_PARAMETER_CONTROLLER_H

#include <escher.h>
#include "axis_interval.h"
#include "graph_view.h"
#include "../../float_parameter_controller.h"

namespace Graph {
class AxisParameterController : public FloatParameterController {
public:
  AxisParameterController(Responder * parentResponder, AxisInterval * axisInterval, GraphView * graphView);
  ExpressionTextFieldDelegate * textFieldDelegate() override;
  const char * title() const override;
  int numberOfRows() override;
  TableViewCell * reusableCell(int index) override;
  int reusableCellCount() override;
  void willDisplayCellForIndex(TableViewCell * cell, int index) override;
  bool handleEvent(Ion::Events::Event event) override;
private:
  float parameterAtIndex(int index) override;
  void setParameterAtIndex(int parameterIndex, float f) override;
  constexpr static int k_numberOfTextCell = 4;
  AxisInterval * m_axisInterval;
  EditableTextMenuListCell m_axisCells[k_numberOfTextCell];
  SwitchMenuListCell m_yAutoCell;
  GraphView * m_graphView;
};

}

#endif
