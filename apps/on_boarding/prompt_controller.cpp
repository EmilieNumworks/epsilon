#include "prompt_controller.h"
#include "../apps_container.h"
#include <assert.h>

using namespace Escher;

namespace OnBoarding {

PromptController::MessageViewWithSkip::MessageViewWithSkip(const I18n::Message * messages, const KDColor * colors, uint8_t numberOfMessages) :
  MessageView(messages, colors, numberOfMessages),
  m_skipView(KDFont::SmallFont, I18n::Message::Skip, KDContext::k_alignRight, KDContext::k_alignCenter)
{
}

int PromptController::MessageViewWithSkip::numberOfSubviews() const {
 return MessageView::numberOfSubviews() + 2;
}

View * PromptController::MessageViewWithSkip::subviewAtIndex(int index) {
  uint8_t numberOfMainMessages = MessageView::numberOfSubviews();
  if (index < numberOfMainMessages) {
    return MessageView::subviewAtIndex(index);
  }
  if (index == numberOfMainMessages) {
    return &m_skipView;
  }
  if (index == numberOfMainMessages + 1) {
    return &m_okView;
  }
  assert(false);
  return nullptr;
}

void PromptController::MessageViewWithSkip::layoutSubviews(bool force) {
  // Layout the main message
  MessageView::layoutSubviews();
  // Layout the "skip (OK)"
  KDCoordinate height = bounds().height();
  KDCoordinate width = bounds().width();
  KDCoordinate textHeight = KDFont::SmallFont->glyphSize().height();
  KDSize okSize = m_okView.minimalSizeForOptimalDisplay();
  m_skipView.setFrame(KDRect(0, height-k_bottomMargin-textHeight, width-okSize.width()-k_okMargin-k_skipMargin, textHeight), force);
  m_okView.setFrame(KDRect(width - okSize.width()-k_okMargin, height-okSize.height()-k_okMargin, okSize), force);
}

PromptController::PromptController(const I18n::Message * messages, const KDColor * colors, uint8_t numberOfMessages) :
  ViewController(nullptr),
  m_messageViewWithSkip(messages, colors, numberOfMessages)
{
}

bool PromptController::handleEvent(Ion::Events::Event event) {
  if (event != Ion::Events::Back && event != Ion::Events::OnOff && event != Ion::Events::USBPlug && event != Ion::Events::USBEnumeration) {
    Container::activeApp()->dismissModalViewController();
    AppsContainer * appsContainer = AppsContainer::sharedAppsContainer();
    if (appsContainer->activeApp()->snapshot() == appsContainer->onBoardingAppSnapshot()) {
      appsContainer->switchToBuiltinApp(appsContainer->appSnapshotAtIndex(0));
    }
    return true;
  }
  return false;
}

}
