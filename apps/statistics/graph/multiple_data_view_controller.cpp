#include "multiple_data_view_controller.h"
#include <apps/i18n.h>
#include <escher/container.h>
#include <assert.h>

using namespace Shared;
using namespace Escher;

namespace Statistics {

MultipleDataViewController::MultipleDataViewController(Responder * parentResponder, Escher::Responder * tabController, Escher::ButtonRowController * header, Escher::StackViewController * stackViewController, Escher::ViewController * typeViewController, Store * store) :
    ViewController(parentResponder),
    GraphButtonRowDelegate(header, stackViewController, this, typeViewController),
    m_tabController(tabController),
    m_store(store),
    m_selectedSeries(-1),
    m_selectedIndex(MultipleDataView::k_defaultSelectedIndex) {
  assert(numberOfButtons(Escher::ButtonRowController::Position::Top) > 0);
}

void MultipleDataViewController::viewWillAppear() {
  ViewController::viewWillAppear();
  sanitizeSeriesIndex();

  /* Call children's viewWillAppear implementation after sanitizing selected
   * series index and before reloading banner and multipleDataView */
  viewWillAppearBeforeReload();

  header()->setSelectedButton(-1);
  reloadBannerView();
  multipleDataView()->reload();
}

bool MultipleDataViewController::handleEvent(Ion::Events::Event event) {
  if (header()->selectedButton() >= 0) {
    if (event == Ion::Events::Up) {
      header()->setSelectedButton(-1);
      Container::activeApp()->setFirstResponder(tabController());
      return true;
    }
    if (event == Ion::Events::Down) {
      header()->setSelectedButton(-1);
      Container::activeApp()->setFirstResponder(this);
      multipleDataView()->setDisplayBanner(true);
      multipleDataView()->selectDataView(m_selectedSeries);
      highlightSelection();
      reloadBannerView();
      return true;
    }
    return false;
  }
  assert(m_selectedSeries >= 0);
  bool isVerticalEvent = (event == Ion::Events::Down || event == Ion::Events::Up);
  if ((isVerticalEvent || event == Ion::Events::Left || event == Ion::Events::Right)) {
    int direction = (event == Ion::Events::Up || event == Ion::Events::Left) ? -1 : 1;
    if (isVerticalEvent ? moveSelectionVertically(direction) : moveSelectionHorizontally(direction)) {
      if (reloadBannerView()) {
        multipleDataView()->reload();
      }
      return true;
    }
  }
  return false;
}

void MultipleDataViewController::didEnterResponderChain(Responder * firstResponder) {
  assert(m_store->seriesIsValid(m_selectedSeries));
  if (!multipleDataView()->dataViewAtIndex(multipleDataView()->indexOfSubviewAtSeries(m_selectedSeries))->isMainViewSelected()) {
    header()->setSelectedButton(0);
  } else {
    multipleDataView()->setDisplayBanner(true);
    multipleDataView()->selectDataView(m_selectedSeries);
    highlightSelection();
  }
}

void MultipleDataViewController::willExitResponderChain(Responder * nextFirstResponder) {
  if (nextFirstResponder == tabController()) {
    assert(tabController() != nullptr);
    if (header()->selectedButton() >= 0) {
      header()->setSelectedButton(-1);
    } else {
      assert(m_selectedSeries >= 0);
      multipleDataView()->deselectDataView(m_selectedSeries);
      multipleDataView()->setDisplayBanner(false);
    }
  }
}

void MultipleDataViewController::sanitizeSeriesIndex() {
  // Sanitize m_selectedSeries
  if (m_selectedSeries < 0 || !m_store->seriesIsValid(m_selectedSeries)) {
    for (int series = 0; series < Store::k_numberOfSeries; series++) {
      if (m_store->seriesIsValid(series)) {
        m_selectedSeries = series;
        return;
      }
    }
    assert(false);
  }
}

bool MultipleDataViewController::moveSelectionVertically(int direction) {
  int nextSelectedSubview = multipleDataView()->indexOfSubviewAtSeries(m_selectedSeries) + direction;
  if (nextSelectedSubview >= m_store->numberOfValidSeries()) {
    return false;
  }
  multipleDataView()->deselectDataView(m_selectedSeries);
  if (nextSelectedSubview < 0) {
    multipleDataView()->setDisplayBanner(false);
    header()->setSelectedButton(0);
  } else {
    m_selectedSeries = multipleDataView()->seriesOfSubviewAtIndex(nextSelectedSubview);
    m_selectedIndex = MultipleDataView::k_defaultSelectedIndex;
    multipleDataView()->selectDataView(m_selectedSeries);
    highlightSelection();
  }
  return true;
}

}
