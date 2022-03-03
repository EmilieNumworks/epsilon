#ifndef PROBABILITY_MODELS_STATISTIC_ONE_PROPORTION_Z_TEST_H
#define PROBABILITY_MODELS_STATISTIC_ONE_PROPORTION_Z_TEST_H

#include "interfaces/distributions.h"
#include "interfaces/significance_tests.h"
#include "test.h"

namespace Probability {

class OneProportionZTest : public Test {
friend class OneProportion;
public:
  SignificanceTestType significanceTestType() const override { return SignificanceTestType::OneProportion; }
  DistributionType distributionType() const override { return DistributionType::Z; }
  I18n::Message title() const override { return OneProportion::Title(); }
  I18n::Message graphTitleFormat() const override { return DistributionZ::GraphTitleFormat(); }

  // Significance Test: OneProportion
  const char * hypothesisSymbol() override { return OneProportion::HypothesisSymbol(); }
  void initParameters() override { OneProportion::InitTestParameters(this); }
  bool authorizedParameterAtIndex(double p, int i) const override { return OneProportion::AuthorizedParameterAtIndex(i, p); }
  void setParameterAtIndex(double p, int index) override {
    p = OneProportion::ProcessParamaterForIndex(p, index);
    Test::setParameterAtIndex(p, index);
  }
  bool isValidH0(double h0) override { return OneProportion::ValidH0(h0); }

  void compute() override { OneProportion::ComputeTest(this); }

  // Distribution: z
  Poincare::Layout testCriticalValueSymbol(const KDFont * font = KDFont::LargeFont) override { return DistributionZ::TestCriticalValueSymbol(font); }
  float canonicalDensityFunction(float x) const override { return DistributionZ::CanonicalDensityFunction(x, m_degreesOfFreedom); }
  double cumulativeDistributiveFunctionAtAbscissa(double x) const override { return DistributionZ::CumulativeNormalizedDistributionFunction(x, m_degreesOfFreedom); }
  double cumulativeDistributiveInverseForProbability(double * p) override { return DistributionZ::CumulativeNormalizedInverseDistributionFunction(*p, m_degreesOfFreedom); }

private:
  // Significance Test: OneProportion
  int numberOfStatisticParameters() const override { return OneProportion::NumberOfParameters(); }
  ParameterRepresentation paramRepresentationAtIndex(int i) const override { return OneProportion::ParameterRepresentationAtIndex(i); }
  double * parametersArray() override { return m_params; }
  // Distribution: z
  float computeYMax() const override { return DistributionZ::YMax(m_degreesOfFreedom); }

  double m_params[OneProportion::k_numberOfParams];};
}  // namespace Probability

#endif /* PROBABILITY_MODELS_STATISTIC_ONE_PROPORTION_Z_TEST_H */