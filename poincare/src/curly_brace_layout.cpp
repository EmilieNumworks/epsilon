#include <poincare/curly_brace_layout.h>

namespace Poincare {

constexpr uint8_t topLeftCurve[CurlyBraceLayoutNode::k_curveHeight][CurlyBraceLayoutNode::k_curveWidth] = {
  {0xBD, 0x33, 0x13, 0x00, 0x00},
  {0x31, 0x74, 0xDA, 0xFF, 0xFF},
  {0x00, 0xDC, 0xFF, 0xFF, 0xFF},
  {0x00, 0xDE, 0xFF, 0xFF, 0xFF},
  {0x00, 0xEF, 0xFF, 0xFF, 0xFF},
  {0x02, 0xF3, 0xFF, 0xFF, 0xFF},
};

constexpr uint8_t topRightCurve[CurlyBraceLayoutNode::k_curveHeight][CurlyBraceLayoutNode::k_curveWidth] = {
  {0x00, 0x00, 0x13, 0x33, 0xBD},
  {0xFF, 0xFF, 0xDA, 0x74, 0x31},
  {0xFF, 0xFF, 0xFF, 0xDC, 0x00},
  {0xFF, 0xFF, 0xFF, 0xDE, 0x00},
  {0xFF, 0xFF, 0xFF, 0xEF, 0x00},
  {0xFF, 0xFF, 0xFF, 0xF3, 0x02},
};

constexpr uint8_t bottomLeftCurve[CurlyBraceLayoutNode::k_curveHeight][CurlyBraceLayoutNode::k_curveWidth] = {
  {0x02, 0xF3, 0xFF, 0xFF, 0xFF},
  {0x00, 0xEF, 0xFF, 0xFF, 0xFF},
  {0x00, 0xDE, 0xFF, 0xFF, 0xFF},
  {0x00, 0xDC, 0xFF, 0xFF, 0xFF},
  {0x31, 0x74, 0xDA, 0xFF, 0xFF},
  {0xBD, 0x33, 0x13, 0x00, 0x00},
};

constexpr uint8_t bottomRightCurve[CurlyBraceLayoutNode::k_curveHeight][CurlyBraceLayoutNode::k_curveWidth] = {
  {0xFF, 0xFF, 0xFF, 0xF3, 0x02},
  {0xFF, 0xFF, 0xFF, 0xEF, 0x00},
  {0xFF, 0xFF, 0xFF, 0xDE, 0x00},
  {0xFF, 0xFF, 0xFF, 0xDC, 0x00},
  {0xFF, 0xFF, 0xDA, 0x74, 0x31},
  {0x00, 0x00, 0x13, 0x33, 0xBD},
};

constexpr uint8_t leftCenter[CurlyBraceLayoutNode::k_centerHeight][CurlyBraceLayoutNode::k_centerWidth] = {
  {0xF2, 0xB8, 0x11},
  {0x00, 0x00, 0xB3},
  {0xF2, 0xB8, 0x11},
};

constexpr uint8_t rightCenter[CurlyBraceLayoutNode::k_centerHeight][CurlyBraceLayoutNode::k_centerWidth] = {
  {0x11, 0xB8, 0xF2},
  {0xB3, 0x00, 0x00},
  {0x11, 0xB8, 0xF2},
};

void CurlyBraceLayoutNode::RenderWithChildHeight(bool left, KDCoordinate childHeight, KDContext * ctx, KDPoint p, KDColor expressionColor, KDColor backgroundColor) {
  // Compute margins and dimensions for each part
  KDColor workingBuffer[k_curveHeight * k_curveWidth];
  assert(k_curveHeight * k_curveWidth >= k_centerHeight * k_centerWidth);
  constexpr KDCoordinate curveHorizontalOffset = k_centerWidth - k_barWidth;
  constexpr KDCoordinate centerHorizontalOffset = k_curveWidth - k_barWidth;
  KDCoordinate leftMargin, curveLeftOffset, barLeftOffset, centerLeftOffset;
  const uint8_t * topCurve, * bottomCurve, * centerPiece;
  if (left) {
    leftMargin = k_horizontalExternalMargin;
    curveLeftOffset = curveHorizontalOffset;
    barLeftOffset = curveHorizontalOffset;
    centerLeftOffset = 0;
    topCurve = &topLeftCurve[0][0];
    bottomCurve = &bottomLeftCurve[0][0];
    centerPiece = &leftCenter[0][0];
  } else {
    leftMargin = k_horizontalInternalMargin;
    curveLeftOffset = 0;
    barLeftOffset = centerHorizontalOffset;
    centerLeftOffset = centerHorizontalOffset;
    topCurve = &topRightCurve[0][0];
    bottomCurve = &bottomRightCurve[0][0];
    centerPiece = &rightCenter[0][0];
  }
  KDCoordinate height = HeightGivenChildHeight(childHeight);
  assert(height > 2 * k_curveHeight + k_centerHeight);
  KDCoordinate bothBarsHeight = height - 2 * k_curveHeight - k_centerHeight;
  KDCoordinate topBarHeight = bothBarsHeight / 2;
  KDCoordinate bottomBarHeight = (bothBarsHeight + 1) / 2;
  assert(topBarHeight == bottomBarHeight || topBarHeight + 1 == bottomBarHeight);

  // Top curve
  KDCoordinate dy = p.y();
  KDRect frame(
      p.x() + leftMargin + curveLeftOffset,
      dy,
      k_curveWidth,
      k_curveHeight);
  ctx->blendRectWithMask(frame, expressionColor, topCurve, workingBuffer);

  // Top bar
  dy += k_curveHeight;
  frame = KDRect(
      p.x() + leftMargin + barLeftOffset,
      dy,
      k_barWidth,
      topBarHeight);
  ctx->fillRect(frame, expressionColor);

  // Center piece
  dy += topBarHeight;
  frame = KDRect(
      p.x() + leftMargin + centerLeftOffset,
      dy,
      k_centerWidth,
      k_centerHeight);
  ctx->blendRectWithMask(frame, expressionColor, centerPiece, workingBuffer);

  // Bottom bar
  dy += k_centerHeight;
  frame = KDRect(
      p.x() + leftMargin + barLeftOffset,
      dy,
      k_barWidth,
      bottomBarHeight);
  ctx->fillRect(frame, expressionColor);

  // Bottom curve
  dy += bottomBarHeight;
  frame = KDRect(
      p.x() + leftMargin + curveLeftOffset,
      dy,
      k_curveWidth,
      k_curveHeight);
  ctx->blendRectWithMask(frame, expressionColor, bottomCurve, workingBuffer);
}

}
