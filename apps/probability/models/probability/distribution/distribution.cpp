#include "distribution.h"
#include "binomial_distribution.h"
#include "chi_squared_distribution.h"
#include "exponential_distribution.h"
#include "fisher_distribution.h"
#include "geometric_distribution.h"
#include "normal_distribution.h"
#include "poisson_distribution.h"
#include "student_distribution.h"
#include "uniform_distribution.h"
#include <poincare/solver.h>
#include <cmath>
#include <float.h>

namespace Probability {

bool Distribution::Initialize(Distribution * distribution, Type type) {
  if (distribution->type() == type) {
    return false;
  }
  distribution->~Distribution();
  switch (type) {
    case Type::Binomial:
      new (distribution) BinomialDistribution();
      break;
    case Type::Uniform:
      new (distribution) UniformDistribution();
      break;
    case Type::Exponential:
      new (distribution) ExponentialDistribution();
      break;
    case Type::Normal:
      new (distribution) NormalDistribution();
      break;
    case Type::ChiSquared:
      new (distribution) ChiSquaredDistribution();
      break;
    case Type::Student:
      new (distribution) StudentDistribution();
      break;
    case Type::Geometric:
      new (distribution) GeometricDistribution();
      break;
    case Type::Poisson:
      new (distribution) PoissonDistribution();
      break;
    case Type::Fisher:
      new (distribution) FisherDistribution();
      break;
    default:
      assert(false);
  }
  return true;
}

void Distribution::setParameterAtIndex(double f, int index) {
  Inference::setParameterAtIndex(f, index);
  computeCurveViewRange();
}

double Distribution::cumulativeDistributiveFunctionAtAbscissa(double x) const {
  if (!isContinuous()) {
    return Poincare::Solver::CumulativeDistributiveFunctionForNDefinedFunction<double>(x,
        [](double k, Poincare::Context * context, const void * auxiliary) {
          const Distribution * distribution = static_cast<const Distribution *>(auxiliary);
          return distribution->evaluateAtDiscreteAbscissa(k);
        },
        nullptr, this);
  }
  return 0.0;
}

double Distribution::rightIntegralFromAbscissa(double x) const {
  if (isContinuous()) {
    return 1.0 - cumulativeDistributiveFunctionAtAbscissa(x);
  }
  return 1.0 - cumulativeDistributiveFunctionAtAbscissa(x-1.0);
}

double Distribution::finiteIntegralBetweenAbscissas(double a, double b) const {
  if (b < a) {
    return 0.0;
  }
  if (a == b) {
    return evaluateAtDiscreteAbscissa(a);
  }
  if (isContinuous()) {
    return cumulativeDistributiveFunctionAtAbscissa(b) - cumulativeDistributiveFunctionAtAbscissa(a);
  }
  int start = std::round(a);
  int end = std::round(b);
  double result = 0.0;
  for (int k = start; k <=end; k++) {
    result += evaluateAtDiscreteAbscissa(k);
    /* Avoid too long loop */
    if (k-start > k_maxNumberOfOperations) {
      break;
    }
    if (result >= k_maxProbability) {
      result = 1.0;
      break;
    }
  }
  return result;
}

double Distribution::cumulativeDistributiveInverseForProbability(double * p) {
  if (*p > 1.0 - DBL_EPSILON) {
    return INFINITY;
  }
  if (isContinuous()) {
    return 0.0;
  }
  if (*p < DBL_EPSILON) {
    return -1.0;
  }
  return Poincare::Solver::CumulativeDistributiveInverseForNDefinedFunction<double>(p,
      [](double k, Poincare::Context * context, const void * auxiliary) {
        const Distribution * distribution = static_cast<const Distribution *>(auxiliary);
        return distribution->evaluateAtDiscreteAbscissa(k);
      }, nullptr, this);
}

double Distribution::rightIntegralInverseForProbability(double * probability) {
  if (isContinuous()) {
    double f = 1.0 - *probability;
    return cumulativeDistributiveInverseForProbability(&f);
  }
  if (*probability >= 1.0) {
    return 0.0;
  }
  if (*probability <= 0.0) {
    return INFINITY;
  }
  double p = 0.0;
  int k = 0;
  double delta = 0.0;
  do {
    delta = std::fabs(1.0-*probability-p);
    p += evaluateAtDiscreteAbscissa(k++);
    if (p >= k_maxProbability && std::fabs(1.0-*probability-p) <= delta) {
      *probability = 0.0;
      return k;
    }
  } while (std::fabs(1.0-*probability-p) <= delta && k < k_maxNumberOfOperations);
  if (k == k_maxNumberOfOperations) {
    *probability = 1.0;
    return INFINITY;
  }
  *probability = 1.0 - (p - evaluateAtDiscreteAbscissa(k-1));
  if (std::isnan(*probability)) {
    return NAN;
  }
  return k-1.0;
}

double Distribution::evaluateAtDiscreteAbscissa(int k) const {
  assert(isContinuous()); // Discrete distribution override this method
  return 0.0;
}

double Distribution::cumulativeDistributiveInverseForProbabilityUsingIncreasingFunctionRoot(double * p, double ax, double bx) {
  assert(ax < bx);
  if (*p > 1.0 - DBL_EPSILON) {
    return INFINITY;
  }
  if (*p < DBL_EPSILON) {
    return -INFINITY;
  }
  const void * pack[2] = { this, p };
  Poincare::Coordinate2D<double> result = Poincare::Solver::IncreasingFunctionRoot(
      ax, bx, DBL_EPSILON,
      [](double x, Poincare::Context * context, const void * auxiliary) {
        const void * const * pack = static_cast<const void * const *>(auxiliary);
        const Distribution * distribution = static_cast<const Distribution *>(pack[0]);
        const double * proba = static_cast<const double *>(pack[1]);
        return distribution->cumulativeDistributiveFunctionAtAbscissa(x) - *proba; // This needs to be an increasing function
      },
      nullptr, pack);
  /* Either no result was found, the precision is ok or the result was outside
   * the given ax bx bounds */
   if (!(std::isnan(result.x2()) || std::fabs(result.x2()) <= FLT_EPSILON || std::fabs(result.x1()- ax) < FLT_EPSILON || std::fabs(result.x1() - bx) < FLT_EPSILON)) {
     /* We would like to put this as an assertion, but sometimes we do get
      * false result: we replace them with inf to make the problem obvious to
      * the student. */
     assert(false);  // TODO this assert is used to hunt a case where that happens. If it doesn't, then we can remove this block of code
     return *p > 0.5 ? INFINITY : -INFINITY;
   }
   return result.x1();
}

}
