#include <poincare/sine.h>
#include <poincare/complex.h>
#include <poincare/cosine.h>
#include <poincare/derivative.h>
#include <poincare/layout_helper.h>
#include <poincare/multiplication.h>
#include <poincare/serialization_helper.h>
#include <poincare/simplification_helper.h>

#include <cmath>

namespace Poincare {

int SineNode::numberOfChildren() const { return Sine::s_functionHelper.numberOfChildren(); }

template<typename T>
Complex<T> SineNode::computeOnComplex(const std::complex<T> c, Preferences::ComplexFormat, Preferences::AngleUnit angleUnit) {
  std::complex<T> angleInput = Trigonometry::ConvertToRadian(c, angleUnit);
  std::complex<T> res = std::sin(angleInput);
  return Complex<T>::Builder(ApproximationHelper::NeglectRealOrImaginaryPartIfNeglectable(res, angleInput));
}

Layout SineNode::createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return LayoutHelper::Prefix(Sine(this), floatDisplayMode, numberOfSignificantDigits, Sine::s_functionHelper.name());
}

int SineNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return SerializationHelper::Prefix(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, Sine::s_functionHelper.name());
}

Expression SineNode::shallowReduce(const ReductionContext& reductionContext) {
  return Sine(this).shallowReduce(reductionContext);
}

bool SineNode::derivate(const ReductionContext& reductionContext, Symbol symbol, Expression symbolValue) {
  return Sine(this).derivate(reductionContext, symbol, symbolValue);
}

Expression SineNode::unaryFunctionDifferential(const ReductionContext& reductionContext) {
  return Sine(this).unaryFunctionDifferential(reductionContext);
}

Expression Sine::shallowReduce(const ExpressionNode::ReductionContext& reductionContext) {
  {
    Expression e = SimplificationHelper::defaultShallowReduce(*this);
    if (!e.isUninitialized()) {
      return e;
    }
  }
  return Trigonometry::shallowReduceDirectFunction(*this, reductionContext);
}

bool Sine::derivate(const ExpressionNode::ReductionContext& reductionContext, Symbol symbol, Expression symbolValue) {
  Derivative::DerivateUnaryFunction(*this, symbol, symbolValue, reductionContext);
  return true;
}

Expression Sine::unaryFunctionDifferential(const ExpressionNode::ReductionContext& reductionContext) {
  return Multiplication::Builder(Trigonometry::UnitConversionFactor(reductionContext.angleUnit(), Preferences::AngleUnit::Radian), Cosine::Builder(childAtIndex(0).clone()));
}

}
