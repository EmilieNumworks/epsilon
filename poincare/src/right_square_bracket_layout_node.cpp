#include <poincare/right_square_bracket_layout_node.h>
#include <poincare/allocation_failure_layout_node.h>

namespace Poincare {

RightSquareBracketLayoutNode * RightSquareBracketLayoutNode::FailedAllocationStaticNode() {
  static AllocationFailureLayoutNode<RightSquareBracketLayoutNode> failure;
  TreePool::sharedPool()->registerStaticNodeIfRequired(&failure);
  return &failure;
}

void RightSquareBracketLayoutNode::render(KDContext * ctx, KDPoint p, KDColor expressionColor, KDColor backgroundColor) {
  ctx->fillRect(KDRect(p.x()+k_widthMargin, p.y(), k_lineThickness, childHeight()), expressionColor);
  ctx->fillRect(KDRect(p.x()+k_widthMargin-k_bracketWidth+1, p.y(), k_bracketWidth, k_lineThickness), expressionColor);
  ctx->fillRect(KDRect(p.x()+k_widthMargin-k_bracketWidth+1, p.y() + childHeight(), k_bracketWidth, k_lineThickness), expressionColor);
}

}
