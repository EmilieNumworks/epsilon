#ifndef SOLVER_CONTROLLERS_FINANCE_CONTROLLER_H
#define SOLVER_CONTROLLERS_FINANCE_CONTROLLER_H

#include <escher/message_table_cell_with_chevron_and_message.h>
#include <escher/stack_view_controller.h>
#include <ion/events.h>

#include "../gui/selectable_cell_list_controller.h"
#include "simple_interest_controller.h"
#include "compound_interest_controller.h"

namespace Solver {

constexpr static int k_numberOfFinanceCells = 2;

class FinanceController : public SelectableCellListPage<Escher::MessageTableCellWithChevronAndMessage,
                                                     k_numberOfFinanceCells> {
public:
  FinanceController(Escher::StackViewController * parentResponder,
                    SimpleInterestController * simpleInterestController,
                    CompoundInterestController * compoundInterestController);
  void didBecomeFirstResponder() override;
  bool handleEvent(Ion::Events::Event e) override;
  const char * title() override;
  int numberOfRows() const override { return k_numberOfFinanceCells; }

  constexpr static int k_indexOfSimpleIntereset = 0;
  constexpr static int k_indexOfCompoundInterest = 1;

private:
  int stackTitleStyleStep() const override { return 0; }

  SimpleInterestController * m_simpleInterestController;
  CompoundInterestController * m_compoundInterestController;
};

}  // namespace Solver

#endif /* SOLVER_CONTROLLERS_FINANCE_CONTROLLER_H */
