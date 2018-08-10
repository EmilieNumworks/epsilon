#include <poincare/left_parenthesis_layout_node.h>
#include <poincare/allocation_failure_layout_node.h>
#include <assert.h>
#include <stdlib.h>

namespace Poincare {

const uint8_t topLeftCurve[ParenthesisLayoutNode::k_parenthesisCurveHeight][ParenthesisLayoutNode::k_parenthesisCurveWidth] = {
  {0xFF, 0xFF, 0xFF, 0xF9, 0x66},
  {0xFF, 0xFF, 0xEB, 0x40, 0x9A},
  {0xFF, 0xF2, 0x40, 0xBF, 0xFF},
  {0xFF, 0x49, 0xB6, 0xFF, 0xFF},
  {0xA9, 0x5A, 0xFF, 0xFF, 0xFF},
  {0x45, 0xBE, 0xFF, 0xFF, 0xFF},
  {0x11, 0xEE, 0xFF, 0xFF, 0xFF},
};

const uint8_t bottomLeftCurve[ParenthesisLayoutNode::k_parenthesisCurveHeight][ParenthesisLayoutNode::k_parenthesisCurveWidth] = {
  {0x11, 0xEE, 0xFF, 0xFF, 0xFF},
  {0x45, 0xBE, 0xFF, 0xFF, 0xFF},
  {0xA9, 0x5A, 0xFF, 0xFF, 0xFF},
  {0xFF, 0x49, 0xB6, 0xFF, 0xFF},
  {0xFF, 0xF2, 0x40, 0xBF, 0xFF},
  {0xFF, 0xFF, 0xEB, 0x40, 0x9A},
  {0xFF, 0xFF, 0xFF, 0xF9, 0x66},
};

void LeftParenthesisLayoutNode::RenderWithChildHeight(KDCoordinate childHeight, KDContext * ctx, KDPoint p, KDColor expressionColor, KDColor backgroundColor) {
  KDCoordinate parenthesisHeight = ParenthesisLayoutNode::HeightGivenChildHeight(childHeight);
  KDRect frame(p.x()+ParenthesisLayoutNode::k_externWidthMargin,
      p.y()+ParenthesisLayoutNode::k_externHeightMargin,
      ParenthesisLayoutNode::k_parenthesisCurveWidth,
      ParenthesisLayoutNode::k_parenthesisCurveHeight);

  ctx->blendRectWithMask(frame, expressionColor, (const uint8_t *)topLeftCurve, (KDColor *)(ParenthesisLayoutNode::s_parenthesisWorkingBuffer));

  frame = KDRect(p.x()+ParenthesisLayoutNode::k_externWidthMargin,
      p.y() + parenthesisHeight - ParenthesisLayoutNode::k_parenthesisCurveHeight - ParenthesisLayoutNode::k_externHeightMargin,
      ParenthesisLayoutNode::k_parenthesisCurveWidth,
      ParenthesisLayoutNode::k_parenthesisCurveHeight);

  ctx->blendRectWithMask(frame, expressionColor, (const uint8_t *)bottomLeftCurve, (KDColor *)(ParenthesisLayoutNode::s_parenthesisWorkingBuffer));

  ctx->fillRect(KDRect(p.x()+ParenthesisLayoutNode::k_externWidthMargin,
        p.y()+ParenthesisLayoutNode::k_parenthesisCurveHeight+ParenthesisLayoutNode::k_externHeightMargin,
        ParenthesisLayoutNode::k_lineThickness,
        parenthesisHeight - 2*(ParenthesisLayoutNode::k_parenthesisCurveHeight+ParenthesisLayoutNode::k_externHeightMargin)),
      expressionColor);
}

LeftParenthesisLayoutNode * LeftParenthesisLayoutNode::FailedAllocationStaticNode() {
  static AllocationFailureLayoutNode<LeftParenthesisLayoutNode> failure;
  TreePool::sharedPool()->registerStaticNodeIfRequired(&failure);
  return &failure;
}

bool LeftParenthesisLayoutNode::isCollapsable(int * numberOfOpenParenthesis, bool goingLeft) const {
  if (*numberOfOpenParenthesis == 0 && goingLeft) {
    return false;
  }
  *numberOfOpenParenthesis = goingLeft ? *numberOfOpenParenthesis - 1 : *numberOfOpenParenthesis + 1;
  return true;
}

void LeftParenthesisLayoutNode::render(KDContext * ctx, KDPoint p, KDColor expressionColor, KDColor backgroundColor) {
  RenderWithChildHeight(ParenthesisLayoutNode::ChildHeightGivenLayoutHeight(layoutSize().height()), ctx, p, expressionColor, backgroundColor);
}

}
