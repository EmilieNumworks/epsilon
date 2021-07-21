#include "homogeneity_table_view_controller.h"

#include "probability/controllers/input_homogeneity_controller.h"

namespace Probability {

HomogeneityTableViewController::HomogeneityTableViewController(
    Escher::Responder * parent,
    HomogeneityStatistic * statistic,
    InputEventHandlerDelegate * inputEventHandlerDelegate,
    DynamicTableViewDataSourceDelegate * dataSourceDelegate,
    Escher::SelectableTableViewDelegate * tableDelegate) :
    TableViewController(parent),
    m_innerTableData(&m_table, inputEventHandlerDelegate, statistic, this, dataSourceDelegate),
    m_tableData(&m_innerTableData, tableDelegate),
    m_table(this, &m_tableData, &m_selectionDataSource, &m_tableData),
    m_statistic(statistic) {
  m_selectionDataSource.selectColumn(-1);
}

void HomogeneityTableViewController::didBecomeFirstResponder() {
  if (m_table.selectedColumn() < 0) {
    m_table.selectColumn(1);
  }
  m_innerTableData.recomputeDimensions();
  TableViewController::didBecomeFirstResponder();
}

bool HomogeneityTableViewController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Backspace) {
    // Delete value
    int row = m_table.selectedRow() - 1, col = m_table.selectedColumn() - 1;
    m_statistic->setParameterAtPosition(row, col, HomogeneityStatistic::k_undefinedValue);

    // Delete last row / column
    m_table.deselectTable();
    if (row == m_innerTableData.numberOfRows() - 2) {
      m_innerTableData.recomputeDimensions();
    }
    if (col == m_innerTableData.numberOfColumns() - 2) {
      m_innerTableData.recomputeDimensions();
    }
    m_table.selectCellAtClippedLocation(col + 1, row + 1, false);
    m_table.reloadCellAtLocation(col + 1, row + 1);
    return true;
  }
  return false;
}

bool HomogeneityTableViewController::textFieldDidFinishEditing(Escher::TextField * textField,
                                                               const char * text,
                                                               Ion::Events::Event event) {
  float p;
  if (textFieldDelegateApp()->hasUndefinedValue(text, p, false, false)) {
    return false;
  }
  int row = m_table.selectedRow(), column = m_table.selectedColumn();
  m_statistic->setParameterAtPosition(row - 1, column - 1, p);

  m_table.deselectTable();
  // Add row
  if (row == m_innerTableData.numberOfRows() &&
      m_innerTableData.numberOfRows() < HomogeneityStatistic::k_maxNumberOfRows) {
    m_innerTableData.recomputeDimensions();
  }
  // Add column
  if (column == m_innerTableData.numberOfColumns() &&
      m_innerTableData.numberOfColumns() < HomogeneityStatistic::k_maxNumberOfColumns) {
    m_innerTableData.recomputeDimensions();
  }

  m_table.selectCellAtLocation(column, row + 1);
  m_table.reloadData(false);  // TODO why needed ?
  return true;
}

}  // namespace Probability
