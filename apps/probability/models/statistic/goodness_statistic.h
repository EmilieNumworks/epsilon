#ifndef APPS_PROBABILITY_MODELS_STATISTIC_GOODNESS_STATISTIC_H
#define APPS_PROBABILITY_MODELS_STATISTIC_GOODNESS_STATISTIC_H

#include "chi2_statistic.h"

namespace Probability {

class GoodnessStatistic : public Chi2Statistic {
public:
  GoodnessStatistic();
  int numberOfStatisticParameters() const override { return k_maxNumberOfRows * 2; }

  void computeTest() override;
  void recomputeData();
  int computeNumberOfRows();
  bool validateInputs() override;

  constexpr static int k_maxNumberOfRows = 10;

protected:
  float * paramArray() override { return m_input; }
  int computeDegreesOfFreedom() { return numberOfValuePairs() - 1; }
  float expectedValue(int index) override;
  float observedValue(int index) override;

private:
  void setExpectedValue(int index, float value);
  void setObservedValue(int index, float value);
  int numberOfValuePairs() override;

  float m_input[k_maxNumberOfRows * 2];
};

}  // namespace Probability

#endif /* APPS_PROBABILITY_MODELS_STATISTIC_GOODNESS_STATISTIC_H */