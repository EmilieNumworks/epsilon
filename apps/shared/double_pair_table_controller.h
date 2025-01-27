#ifndef SHARED_DOUBLE_PAIR_TABLE_CONTROLLER_H
#define SHARED_DOUBLE_PAIR_TABLE_CONTROLLER_H

#include "double_pair_store.h"
#include "prefaced_twice_table_view.h"
#include "tab_table_controller.h"
#include <escher/table_view_data_source.h>
#include <escher/button_row_controller.h>
#include <escher/alternate_empty_view_controller.h>
#include <apps/i18n.h>

namespace Shared {

class DoublePairTableController : public TabTableController, public Escher::TableViewDataSource, public Escher::ButtonRowDelegate, public Escher::AlternateEmptyViewDefaultDelegate, public Escher::SelectableTableViewDelegate, public PrefacedTableView::MarginDelegate {
public:
  constexpr static int k_titleNumberOfChars = 22;

  DoublePairTableController(Escher::Responder * parentResponder, Escher::ButtonRowController * header);

  // TableViewDataSource
  KDCoordinate rowHeight(int j) override { return k_cellHeight; }
  KDCoordinate cumulatedHeightFromIndex(int j) override { return j*rowHeight(0); }
  int indexFromCumulatedHeight(KDCoordinate offsetY) override { return (offsetY-1) / rowHeight(0); }

  // AlternateEmptyViewDefaultDelegate
  bool isEmpty() const override { return !store()->hasValidSeries(); }
  I18n::Message emptyMessage() override { return I18n::Message::NoValueToCompute; }
  Responder * defaultController() override { return tabController(); }

  // ViewController
  const char * title() override { return I18n::translate(I18n::Message::StatTab); }
  Escher::View * view() override { return &m_prefacedView; }

  // Responder
  bool handleEvent(Ion::Events::Event event) override;
  void didBecomeFirstResponder() override;

  // PrefacedTableView::MarginDelegate
  KDCoordinate prefaceMargin(Escher::TableView * preface, Escher::TableViewDataSource * prefaceDataSource) override;

protected:
  constexpr static int k_maxNumberOfDisplayableRows = 11;
  static constexpr KDCoordinate k_cellHeight = 20;
  static constexpr KDCoordinate k_margin = 8;
  static constexpr KDCoordinate k_scrollBarMargin = Escher::Metric::CommonRightMargin;

  virtual DoublePairStore * store() const = 0;
  Escher::Responder * tabController() const override { return (parentResponder()->parentResponder()->parentResponder()); }
  Escher::SelectableTableView * selectableTableView() override { return &m_selectableTableView; }

  PrefacedTwiceTableView m_prefacedView;
  Escher::SelectableTableView m_selectableTableView;
};

}

#endif
