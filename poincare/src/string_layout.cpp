#include <poincare/string_layout.h>

namespace Poincare {

StringLayoutNode::StringLayoutNode(char * string, int stringSize) {
  m_stringLength = strlcpy(m_string, string, stringSize);
}

int StringLayoutNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return strlcpy(buffer, m_string, bufferSize);
}

size_t StringLayoutNode::size() const {
  return sizeof(StringLayoutNode) + sizeof(char) * m_stringLength;
}

bool StringLayoutNode::protectedIsIdenticalTo(Layout l) {
  assert(l.type() == Type::StringLayout);
  StringLayout & sl = static_cast<StringLayout &>(l);
  return strncmp(m_string, sl.string(), std::max(m_stringLength + 1, sl.stringLength() + 1)) == 0;
}

// Sizing and positioning
KDSize StringLayoutNode::computeSize() {
  KDCoordinate totalHorizontalMargin = 0;
  KDSize glyph = font()->glyphSize();
  return KDSize(m_stringLength * glyph.width() + totalHorizontalMargin, glyph.height());
}

KDCoordinate StringLayoutNode::computeBaseline() {
  return font()->glyphSize().height()/2;
}

void StringLayoutNode::render(KDContext * ctx, KDPoint p, KDColor expressionColor, KDColor backgroundColor, Layout * selectionStart, Layout * selectionEnd, KDColor selectionColor) {
  int bufferSize = m_stringLength + 1;
  char buffer[bufferSize];
  ctx->drawString(buffer, p, font(), expressionColor, backgroundColor);
}

StringLayout StringLayout::Builder(char * string, int stringSize) {
  void * bufferNode = TreePool::sharedPool()->alloc(sizeof(StringLayoutNode) + sizeof(char) * stringSize);
  StringLayoutNode * node = new (bufferNode) StringLayoutNode(string, stringSize);
  TreeHandle h = TreeHandle::BuildWithGhostChildren(node);
  return static_cast<StringLayout &>(h);
}

}


