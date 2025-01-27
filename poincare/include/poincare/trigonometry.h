#ifndef POINCARE_TRIGONOMETRY_H
#define POINCARE_TRIGONOMETRY_H

#include <poincare/expression.h>
#include <poincare/evaluation.h>

namespace Poincare {

class Trigonometry final {
public:
  enum class Function {
    Cosine = 0,
    Sine = 1,
  };
  static Expression PiExpressionInAngleUnit(Preferences::AngleUnit angleUnit);
  static double PiInAngleUnit(Preferences::AngleUnit angleUnit);
  static bool isDirectTrigonometryFunction(const Expression & e);
  static bool isInverseTrigonometryFunction(const Expression & e);
  static bool isAdvancedTrigonometryFunction(const Expression & e);
  static bool isInverseAdvancedTrigonometryFunction(const Expression & e);
  static bool AreInverseFunctions(const Expression & directFunction, const Expression & inverseFunction);
  /* Returns a (unreduced) division between pi in each unit, or 1 if the units
   * are the same. */
  static Expression UnitConversionFactor(Preferences::AngleUnit fromUnit, Preferences::AngleUnit toUnit);
  static bool ExpressionIsEquivalentToTangent(const Expression & e);
  static Expression shallowReduceDirectFunction(Expression & e, const ExpressionNode::ReductionContext& reductionContext);
  static Expression shallowReduceInverseFunction(Expression & e,  const ExpressionNode::ReductionContext& reductionContext);
  static Expression shallowReduceAdvancedFunction(Expression & e,  const ExpressionNode::ReductionContext& reductionContext);
  static Expression shallowReduceInverseAdvancedFunction(Expression & e,  const ExpressionNode::ReductionContext& reductionContext);
  static Expression replaceWithAdvancedFunction(Expression & e, Expression & denominator);
  static Expression table(const Expression e, ExpressionNode::Type type,  const ExpressionNode::ReductionContext& reductionContext); // , Function f, bool inverse
  template <typename T> static std::complex<T> ConvertToRadian(const std::complex<T> c, Preferences::AngleUnit angleUnit);
  template <typename T> static std::complex<T> ConvertRadianToAngleUnit(const std::complex<T> c, Preferences::AngleUnit angleUnit);
};

}

#endif
