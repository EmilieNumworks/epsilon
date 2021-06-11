#include "type_controller.h"

#include <apps/i18n.h>
#include <assert.h>
#include <escher/selectable_list_view_controller.h>
#include <escher/view_controller.h>
#include <ion/events.h>
#include <string.h>

#include "categorical_type_controller.h"
#include "hypothesis_controller.h"
#include "probability/app.h"
#include "probability/models/data.h"
#include "probability/text_helpers.h"
#include "type_controller.h"

using namespace Probability;

TypeController::TypeController(StackViewController * parent,
                               HypothesisController * hypothesisController,
                               InputController * inputController) :
    SelectableListViewPage(parent),
    m_hypothesisController(hypothesisController),
    m_inputController(inputController),
    m_contentView(&m_selectableTableView, &m_description) {
  m_cells[k_indexOfTTest].setMessage(I18n::Message::TTest);
  m_cells[k_indexOfTTest].setSubtitle(I18n::Message::Recommended);
  m_cells[k_indexOfPooledTest].setMessage(I18n::Message::PooledTTest);
  m_cells[k_indexOfPooledTest].setSubtitle(I18n::Message::RarelyUsed);
  m_cells[k_indexOfZTest].setMessage(I18n::Message::ZTest);
  m_cells[k_indexOfZTest].setSubtitle(I18n::Message::RarelyUsed);

  m_description.setBackgroundColor(Palette::WallScreen);
  m_description.setTextColor(Palette::GrayDark);
  m_description.setAlignment(0.5, 0);
  m_description.setFont(KDFont::SmallFont);
  m_description.setMessage(I18n::Message::TypeDescr);
}

void TypeController::didBecomeFirstResponder() {
  Probability::App::app()->snapshot()->navigation()->setPage(Data::Page::Type);
  selectRowAccordingToType(App::app()->snapshot()->data()->testType());
  Container::activeApp()->setFirstResponder(&m_selectableTableView);
}

bool TypeController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::OK || event == Ion::Events::EXE || event == Ion::Events::Right) {
    Escher::ViewController * view;
    Data::TestType t;
    switch (selectedRow()) {
      case k_indexOfTTest:
        t = Data::TestType::TTest;
        break;
      case k_indexOfPooledTest:
        t = Data::TestType::PooledTTest;
        break;
      case k_indexOfZTest:
        t = Data::TestType::ZTest;
        break;
    }
    if (App::app()->snapshot()->navigation()->subapp() == Data::SubApp::Intervals) {
      view = m_inputController;
    } else {
      view = m_hypothesisController;
    }
    assert(view != nullptr);
    initializeStatistic(App::app()->snapshot()->data()->test(), t);
    App::app()->snapshot()->data()->setTestType(t);
    openPage(view);
    return true;
  }
  return false;
}

void TypeController::selectRowAccordingToType(Data::TestType t) {
  int row = -1;
  switch (t) {
    case Data::TestType::TTest:
      row = k_indexOfTTest;
      break;
    case Data::TestType::PooledTTest:
      row = k_indexOfPooledTest;
      break;
    case Data::TestType::ZTest:
      row = k_indexOfZTest;
      break;
  }
  assert(row >= 0);
  selectRow(row);
}

Escher::View * TypeView::subviewAtIndex(int i) {
  // TODO rm ?
  switch (i) {
    case 0:
      return m_list;
    case 1:
      return m_description;
    default:
      assert(false);
      return nullptr;
  }
}

const char * TypeController::title() {
  // TODO replace with messages
  sprintf(m_titleBuffer, "Test on %s", testToText(App::app()->snapshot()->data()->test()));
  return m_titleBuffer;
}

void Probability::TypeController::initializeStatistic(Data::Test test, Data::TestType type) {
  if (test == Data::Test::OneMean) {
    if (type == Data::TestType::TTest) {
      new (App::app()->snapshot()->data()->statistic()) OneMeanTStatistic();
    } else if (type == Data::TestType::ZTest) {
      new (App::app()->snapshot()->data()->statistic()) OneMeanZStatistic();
    }
  } else if (test == Data::Test::TwoMeans) {
    if (type == Data::TestType::TTest) {
      new (App::app()->snapshot()->data()->statistic()) TwoMeansTStatistic();

    } else if (type == Data::TestType::ZTest) {
      new (App::app()->snapshot()->data()->statistic()) TwoMeansZStatistic();
    }
  }
}