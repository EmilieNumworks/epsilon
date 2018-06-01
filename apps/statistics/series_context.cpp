#include "series_context.h"
#include <poincare/decimal.h>
#include <assert.h>
#include <string.h>

using namespace Poincare;
using namespace Shared;

namespace Statistics {

void SeriesContext::setExpressionForSymbolName(const Expression * expression, const Symbol * symbol, Context & context) {
  m_parentContext->setExpressionForSymbolName(expression, symbol, context);
}

const Expression * SeriesContext::expressionForSymbol(const Symbol * symbol) {
  if (Symbol::isSeriesSymbol(symbol->name())) {
    const char * seriesName = Symbol::textForSpecialSymbols(symbol->name());
    assert(strlen(seriesName) == 2);

    int series = (int)(seriesName[1] - '0') - 1;
    assert(series >= 0 && series < FloatPairStore::k_numberOfSeries);

    assert((seriesName[0] == 'V') || (seriesName[0] == 'N'));
    int storeI = seriesName[0] == 'V' ? 0 : 1;

    assert(m_seriesPairIndex >= 0);
    assert(m_seriesPairIndex < m_store->numberOfPairsOfSeries(series));

    Expression * result = new Decimal(m_store->get(series, storeI, m_seriesPairIndex));
    assert(result != nullptr);
    return result;
  } else {
    return m_parentContext->expressionForSymbol(symbol);
  }
}

}
