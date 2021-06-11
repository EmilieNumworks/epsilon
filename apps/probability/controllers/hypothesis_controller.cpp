#include "hypothesis_controller.h"

#include <apps/apps_container.h>
#include <apps/i18n.h>
#include <apps/shared/poincare_helpers.h>
#include <escher/container.h>
#include <escher/input_event_handler_delegate.h>
#include <escher/invocation.h>
#include <escher/responder.h>
#include <escher/stack_view_controller.h>
#include <poincare/preferences.h>
#include <poincare/print_float.h>

#include <new>

#include "input_controller.h"
#include "probability/app.h"
#include "probability/models/data.h"
#include "probability/text_helpers.h"

using namespace Probability;

HypothesisController::HypothesisController(Escher::StackViewController * parent,
                                           InputController * inputController,
                                           InputEventHandlerDelegate * handler) :
    SelectableListViewPage(parent),
    m_inputController(inputController),
    m_h0(&m_selectableTableView, handler, this),
    m_ha(&m_selectableTableView, handler, this),
    m_next(&m_selectableTableView, I18n::Message::Ok, buttonActionInvocation()) {
  m_h0.setMessage(I18n::Message::H0);
  m_ha.setMessage(I18n::Message::Ha);
}

const char * Probability::HypothesisController::title() {
  const char * testType = testTypeToText(App::app()->snapshot()->data()->testType());
  const char * test = testToText(App::app()->snapshot()->data()->test());
  sprintf(m_titleBuffer, "%s on %s", testType, test);
  return m_titleBuffer;
}

bool Probability::HypothesisController::textFieldShouldFinishEditing(Escher::TextField * textField, Ion::Events::Event event) {
    return (event == Ion::Events::Down && selectedRow() < numberOfRows()-1)
      || (event == Ion::Events::Up && selectedRow() > 0)
      || TextFieldDelegate::textFieldShouldFinishEditing(textField, event);
}

bool Probability::HypothesisController::textFieldDidFinishEditing(Escher::TextField * textField, const char * text, Ion::Events::Event event) {
  if (textField == m_h0.textField()) {
    float h0;
    if (textFieldDelegateApp()->hasUndefinedValue(text, h0, false, false)) {
      return false;
    }
    // TODO could check validity
    App::app()->snapshot()->data()->hypothesisParams()->setFirstParam(h0);
    loadHypothesisParam(true);
    return true;
  }
  return false;
}

HighlightCell * HypothesisController::reusableCell(int i, int type) {
  switch (i) {
    case k_indexOfH0:
      return &m_h0;
    case k_indexOfHa:
      return &m_ha;
    default:
      assert(i == k_indexOfNext);
      return &m_next;
  }
}

void HypothesisController::didBecomeFirstResponder() {
  App::app()->snapshot()->navigation()->setPage(Data::Page::Hypothesis);
  // TODO factor out
  selectCellAtLocation(0, 0);
  loadHypothesisParam();
  Escher::Container::activeApp()->setFirstResponder(&m_selectableTableView);
}

void HypothesisController::buttonAction() {
  storeHypothesisParams();
  openPage(m_inputController);
}

void HypothesisController::loadHypothesisParam(bool h0Only) {
  constexpr int bufferSize = 20;
  char buffer[bufferSize]{0};
  const char * symbol = testToTextSymbol(App::app()->snapshot()->data()->test());
  const char op = static_cast<const char>(App::app()->snapshot()->data()->hypothesisParams()->op());
  int written = sprintf(buffer, "%s%c", symbol, op);
  float p = App::app()->snapshot()->data()->hypothesisParams()->firstParam();
  Poincare::PrintFloat::ConvertFloatToText(p, buffer + written, bufferSize, k_maxInputLength, 5,
                                           Poincare::Preferences::PrintFloatMode::Decimal);
  m_ha.setAccessoryText(buffer);
  if (!h0Only) {
    m_h0.setAccessoryText(buffer + written);
  }
}

void HypothesisController::storeHypothesisParams() {
  // TODO maybe parse at the same time as when checking validity
  // TODO use TextFieldDelegateApp
  HypothesisParams * params = App::app()->snapshot()->data()->hypothesisParams();
  params->setFirstParam(Shared::PoincareHelpers::ApproximateToScalar<float>(
      m_h0.textField()->text(), AppsContainer::sharedAppsContainer()->globalContext()));
  // params->setOp(HypothesisParams::ComparisonOperator::Higher);
}