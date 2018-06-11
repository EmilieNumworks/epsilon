#include "trigonometric_model.h"
#include "../../poincare/include/poincare_layouts.h"
#include <math.h>
#include <poincare/preferences.h>
#include <assert.h>

using namespace Poincare;

namespace Regression {

ExpressionLayout * TrigonometricModel::layout() {
  static ExpressionLayout * layout = nullptr;
  if (layout == nullptr) {
    const ExpressionLayout * layoutChildren[] = {
      new CharLayout('a', KDText::FontSize::Small),
      new CharLayout(Ion::Charset::MiddleDot, KDText::FontSize::Small),
      new CharLayout('s', KDText::FontSize::Small),
      new CharLayout('i', KDText::FontSize::Small),
      new CharLayout('n', KDText::FontSize::Small),
      new CharLayout('(', KDText::FontSize::Small),
      new CharLayout('b', KDText::FontSize::Small),
      new CharLayout(Ion::Charset::MiddleDot, KDText::FontSize::Small),
      new CharLayout('X', KDText::FontSize::Small),
      new CharLayout('+', KDText::FontSize::Small),
      new CharLayout('c', KDText::FontSize::Small),
      new CharLayout(')', KDText::FontSize::Small)
    };
    layout = new HorizontalLayout(layoutChildren, 12, false);
  }
  return layout;
}

double TrigonometricModel::evaluate(double * modelCoefficients, double x) const {
  double a = modelCoefficients[0];
  double b = modelCoefficients[1];
  double c = modelCoefficients[2];
  double radianX = Poincare::Preferences::sharedPreferences()->angleUnit() == Poincare::Expression::AngleUnit::Radian ? x : x * M_PI/180.0;
  return a*sin(b*radianX+c);
}

double TrigonometricModel::levelSet(double * modelCoefficients, double y) const {
  return NAN;
}

double TrigonometricModel::partialDerivate(double * modelCoefficients, int derivateCoefficientIndex, double x) const {
  double a = modelCoefficients[0];
  double b = modelCoefficients[1];
  double c = modelCoefficients[2];
  double radianX = Poincare::Preferences::sharedPreferences()->angleUnit() == Poincare::Expression::AngleUnit::Radian ? x : x * M_PI/180.0;
  if (derivateCoefficientIndex == 0) {
    // Derivate: sin(b*x+c)
    return sin(b*radianX+c);
  }
  if (derivateCoefficientIndex == 1) {
    // Derivate: x*a*cos(b*x+c);
    return radianX*a*cos(b*radianX+c);
  }
  if (derivateCoefficientIndex == 2) {
    // Derivate: a*cos(b*x+c)
    return a*cos(b*radianX+c);
  }
  assert(false);
  return 0.0;
}

}
