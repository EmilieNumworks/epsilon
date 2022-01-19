#ifndef SOLVER_CONTROLLERS_SIMPLE_INTEREST_CONTROLLER_H
#define SOLVER_CONTROLLERS_SIMPLE_INTEREST_CONTROLLER_H

#include <apps/shared/button_with_separator.h>
#include <escher/highlight_cell.h>
#include <escher/message_table_cell_with_editable_text_with_message.h>
#include <escher/stack_view_controller.h>
#include <escher/text_field_delegate.h>
#include "finance_result_controller.h"
#include <escher/button_delegate.h>
#include "../gui/year_popup_data_source.h"
#include "../gui/message_table_cell_with_sublabel_and_dropdown.h"
#include <apps/shared/page_controller.h>
#include "../model/data.h"

using namespace Escher;

namespace Solver {

class SimpleInterestController : public Shared::SelectableListViewPage,
                             public Escher::ButtonDelegate,
                             public Escher::TextFieldDelegate,
                             public DropdownCallback {
public:
  SimpleInterestController(Escher::StackViewController * parent,
                           InputEventHandlerDelegate * handler,
                           FinanceResultController * financeResultController,
                           FinanceData * data);
  const char * title() override;
  void didBecomeFirstResponder() override;
  bool handleEvent(Ion::Events::Event event) override;
  int reusableCellCount(int type) override;
  void willDisplayCellForIndex(Escher::HighlightCell * cell, int index) override;
  int typeAtIndex(int index) override;
  KDCoordinate nonMemoizedRowHeight(int j) override;
  HighlightCell * reusableCell(int i, int type) override;
  int numberOfRows() const override { return k_indexOfNext + 1; }
  bool buttonAction() override;
  ViewController::TitlesDisplay titlesDisplay() override { return ViewController::TitlesDisplay::DisplayLastTwoTitles; }


  bool textFieldDidReceiveEvent(Escher::TextField * textField, Ion::Events::Event event) override { return false; }
  bool textFieldShouldFinishEditing(Escher::TextField * textField,
                                    Ion::Events::Event event) override;
  bool textFieldDidFinishEditing(Escher::TextField * textField,
                                 const char * text,
                                 Ion::Events::Event event) override;

  // DropdownCallback
  void onDropdownSelected(int selectedRow) override;

private:
  SimpleInterestParameter paramaterAtIndex(int index) const;
  SimpleInterestData * simpleInterestData() const { assert(m_data->isSimpleInterest); return &(m_data->m_data.m_simpleInterestData); }
  int stackTitleStyleStep() const override { return 1; }

  // TODO Hugo : Justify this number
  constexpr static int k_indexOfYear = 3;
  constexpr static int k_indexOfNext = k_indexOfYear + 1;

  constexpr static int k_inputCellType = 0;
  constexpr static int k_dropdownCellType = 1;
  constexpr static int k_confirmCellType = 2;

  // TODO Hugo : Justify this number
  constexpr static int k_numberOfReusableInputs = 4; // Visible cell max
  static_assert(k_numberOfReusableInputs <= k_indexOfYear + 1, "Too many reusable inputs");

  YearPopupDataSource m_yearDataSource;

  Escher::MessageTableCellWithEditableTextWithMessage m_cells[k_numberOfReusableInputs];
  MessageTableCellWithSublabelAndDropdown m_year;
  Shared::ButtonWithSeparator m_next;

  static constexpr int k_titleBufferSize = 1 + Ion::Display::Width / 7; // KDFont::SmallFont->glyphSize().width() = 7
  char m_titleBuffer[k_titleBufferSize];

  FinanceResultController * m_financeResultController;
  FinanceData * m_data;
};

}  // namespace Solver

#endif /* SOLVER_CONTROLLERS_SIMPLE_INTEREST_CONTROLLER_H */