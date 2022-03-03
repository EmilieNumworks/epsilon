#include "double_pair_store.h"
#include <cmath>
#include <assert.h>
#include <stddef.h>
#include <ion.h>
#include <algorithm>

namespace Shared {

void DoublePairStore::set(double f, int series, int i, int j) {
  assert(series >= 0 && series < k_numberOfSeries);
  if (j >= k_maxNumberOfPairs) {
    return;
  }
  assert(j <= std::max(m_numberOfValues[series][0], m_numberOfValues[series][1]));
  int otherI = i == 0 ? 1 : 0;
  m_data[series][i][j] = f;
  if (j == m_numberOfValues[series][i]) {
    m_numberOfValues[series][i]++;
  }
  if (j >= m_numberOfValues[series][otherI]) {
    for (int k = m_numberOfValues[series][otherI] ; k <= j ; k++) {
      m_data[series][otherI][j] = defaultValue(series, otherI, j);
      m_numberOfValues[series][otherI]++;
    }
  }
}

int DoublePairStore::numberOfPairs() const {
  int result = 0;
  for (int i = 0; i < k_numberOfSeries; i++) {
    if (seriesIsValid(i)) {
      result += m_numberOfValues[i][0];
    }
  }
  return result;
}

int DoublePairStore::numberOfPairsOfSeries(int series) const {
  assert(series >= 0 && series < k_numberOfSeries);
  return std::min(m_numberOfValues[series][0], m_numberOfValues[series][1]);
}

void DoublePairStore::deleteColumn(int series, int column) {
  /* We reset all values to 0 to ensure the correctness of the checksum.*/
  for (int k = 0 ; k < m_numberOfValues[series][column] ; k++) {
    m_data[series][column][k] = 0.0;
  }
  m_numberOfValues[series][column] = 0;
}

void DoublePairStore::deletePairOfSeriesAtIndex(int series, int j) {
   for (int k = j; k < m_numberOfValues[series][0] - 1; k++) {
    m_data[series][0][k] = m_data[series][0][k+1];
  }

  for (int k = j; k < m_numberOfValues[series][1] - 1; k++) {
    m_data[series][1][k] = m_data[series][1][k+1];
  }

  /* We reset the values of the empty row to ensure the correctness of the
   * checksum. */
  if (j < m_numberOfValues[series][0]) {
     m_numberOfValues[series][0]--;
     m_data[series][0][m_numberOfValues[series][0]] = 0;
  }
  if (j < m_numberOfValues[series][1]) {
     m_numberOfValues[series][1]--;
     m_data[series][1][m_numberOfValues[series][1]] = 0;
  }
}

void DoublePairStore::deleteAllPairsOfSeries(int series) {
  assert(series >= 0 && series < k_numberOfSeries);
  deleteColumn(series, 0);
  deleteColumn(series, 1);
}

void DoublePairStore::deleteAllPairs() {
  for (int i = 0; i < k_numberOfSeries; i ++) {
    deleteAllPairsOfSeries(i);
  }
}

void DoublePairStore::resetColumn(int series, int i) {
  assert(series >= 0 && series < k_numberOfSeries);
  assert(i == 0 || i == 1);
  for (int k = 0; k < m_numberOfValues[series][i]; k++) {
    m_data[series][i][k] = defaultValue(series, i, k);
  }
}

void DoublePairStore::makeColumnsEqualLength(int series) {
  assert(series >= 0 && series < k_numberOfSeries);
  int longerColumn = m_numberOfValues[series][0] <= m_numberOfValues[series][1] ? 1 : 0;
  int shorterColumn = longerColumn == 0 ? 1 : 0;
  for (int i = m_numberOfValues[series][shorterColumn] ; i < m_numberOfValues[series][longerColumn] ; i++) {
    m_data[series][shorterColumn][i] = 0.0;
    m_numberOfValues[series][shorterColumn]++;
  }
}


bool DoublePairStore::isEmpty() const {
  for (int i = 0; i < k_numberOfSeries; i++) {
    if (seriesIsValid(i)) {
      return false;
    }
  }
  return true;
}

int DoublePairStore::numberOfNonEmptySeries() const {
  int nonEmptySeriesCount = 0;
  for (int i = 0; i< k_numberOfSeries; i++) {
    if (seriesIsValid(i)) {
      nonEmptySeriesCount++;
    }
  }
  return nonEmptySeriesCount;
}


int DoublePairStore::indexOfKthNonEmptySeries(int k) const {
  assert(k >= 0 && k < numberOfNonEmptySeries());
  int nonEmptySeriesCount = 0;
  for (int i = 0; i < k_numberOfSeries; i++) {
    if (seriesIsValid(i)) {
      if (nonEmptySeriesCount == k) {
        return i;
      }
      nonEmptySeriesCount++;
    }
  }
  assert(false);
  return 0;
}

double DoublePairStore::sumOfColumn(int series, int i, bool lnOfSeries) const {
  assert(series >= 0 && series < k_numberOfSeries);
  assert(i == 0 || i == 1);
  double result = 0;
  for (int k = 0; k < m_numberOfValues[series][i]; k++) {
    result += lnOfSeries ? log(m_data[series][i][k]) : m_data[series][i][k];
  }
  return result;
}

bool DoublePairStore::seriesNumberOfAbscissaeGreaterOrEqualTo(int series, int i) const {
  assert(series >= 0 && series < k_numberOfSeries);
  int count = 0;
  for (int j = 0; j < m_numberOfValues[series][0]; j++) {
    if (count >= i) {
      return true;
    }
    double currentAbsissa = m_data[series][0][j];
    bool firstOccurence = true;
    for (int k = 0; k < j; k++) {
      if (m_data[series][0][k] == currentAbsissa) {
        firstOccurence = false;
        break;
      }
    }
    if (firstOccurence) {
      count++;
    }
  }
  return count >= i;
}

uint32_t DoublePairStore::storeChecksum() const {
  uint32_t checkSumPerSeries[k_numberOfSeries];
  for (int i = 0; i < k_numberOfSeries; i++) {
    checkSumPerSeries[i] = storeChecksumForSeries(i);
  }
  return Ion::crc32Word(checkSumPerSeries, k_numberOfSeries);
}

uint32_t DoublePairStore::storeChecksumForSeries(int series) const {
  /* Ideally, we would compute the checksum of the first m_numberOfPairs pairs.
   * However, the two values of a pair are not stored consecutively. We thus
   * compute the checksum of the x values of the pairs, then we compute the
   * checksum of the y values of the pairs, and finally we compute the checksum
   * of the checksums.
   * We cannot simply put "empty" values to 0 and compute the checksum of the
   * whole data, because adding or removing (0, 0) "real" data pairs would not
   * change the checksum. */
  size_t dataLengthInBytesPerDataColumn = std::min(m_numberOfValues[series][0], m_numberOfValues[series][1])*sizeof(double);
  assert((dataLengthInBytesPerDataColumn & 0x3) == 0); // Assert that dataLengthInBytes is a multiple of 4
  uint32_t checkSumPerColumn[k_numberOfColumnsPerSeries];
  for (int i = 0; i < k_numberOfColumnsPerSeries; i++) {
    checkSumPerColumn[i] = Ion::crc32Word((uint32_t *)m_data[series][i], dataLengthInBytesPerDataColumn/sizeof(uint32_t));
  }
  return Ion::crc32Word(checkSumPerColumn, k_numberOfColumnsPerSeries);
}

double DoublePairStore::defaultValue(int series, int i, int j) const {
  assert(series >= 0 && series < k_numberOfSeries);
  if(i == 0 && j > 1) {
    // constant step increment
    return 2*m_data[series][i][j-1]-m_data[series][i][j-2];
  } else {
    return 0.0;
  }
}

}
