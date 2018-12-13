#include <poincare/complex_cartesian.h>
#include <poincare/addition.h>
#include <poincare/arc_tangent.h>
#include <poincare/cosine.h>
#include <poincare/constant.h>
#include <poincare/division.h>
#include <poincare/multiplication.h>
#include <poincare/naperian_logarithm.h>
#include <poincare/rational.h>
#include <poincare/square_root.h>
#include <poincare/sine.h>
#include <poincare/sign_function.h>
#include <poincare/subtraction.h>
#include <poincare/power.h>
#include <assert.h>
#include <cmath>

namespace Poincare {

Expression ComplexCartesianNode::shallowReduce(Context & context, Preferences::AngleUnit angleUnit, ReductionTarget target) {
  return ComplexCartesian(this).shallowReduce(context, angleUnit);
}

Expression ComplexCartesianNode::shallowBeautify(Context & context, Preferences::AngleUnit angleUnit) {
  return ComplexCartesian(this).shallowBeautify(context, angleUnit);
}


bool isZero(const Expression e) {
  return e.type() == ExpressionNode::Type::Rational && static_cast<const Rational &>(e).isZero();
}
bool isOne(const Expression e) {
  return e.type() == ExpressionNode::Type::Rational && static_cast<const Rational &>(e).isOne();
}
bool isMinusOne(const Expression e) {
  return e.type() == ExpressionNode::Type::Rational && static_cast<const Rational &>(e).isMinusOne();
}

Expression ComplexCartesian::shallowReduce(Context & context, Preferences::AngleUnit angleUnit) {
  if (imag().isRationalZero()) {
    Expression r = real();
    replaceWithInPlace(r);
    return r;
  }
  return *this;
}

Expression ComplexCartesian::shallowBeautify(Context & context, Preferences::AngleUnit angleUnit) {
  Expression a = real();
  Expression b = imag();
  Expression oppositeA = a.makePositiveAnyNegativeNumeralFactor(context, angleUnit);
  Expression oppositeB = b.makePositiveAnyNegativeNumeralFactor(context, angleUnit);
  a = oppositeA.isUninitialized() ? a : oppositeA;
  b = oppositeB.isUninitialized() ? b : oppositeB;
  Expression e = Expression::CreateComplexExpression(a, b, Preferences::ComplexFormat::Cartesian,
      a.type() == ExpressionNode::Type::Undefined || b.type() == ExpressionNode::Type::Undefined,
      isZero(a), isOne(a), isZero(b), isOne(b), isMinusOne(b),
      !oppositeA.isUninitialized(),
      !oppositeB.isUninitialized()
    );
  replaceWithInPlace(e);
  return e;
}

Expression ComplexCartesian::squareNorm(Context & context, Preferences::AngleUnit angleUnit, ExpressionNode::ReductionTarget target) {
  Expression a = real();
  Expression b = imag();
  Expression a2 = Power(a, Rational(2));
  Expression b2 = Power(b, Rational(2));
  Addition add(a2, b2);
  a2.shallowReduce(context, angleUnit, target);
  b2.shallowReduce(context, angleUnit, target);
  return add;
}

Expression ComplexCartesian::norm(Context & context, Preferences::AngleUnit angleUnit, ExpressionNode::ReductionTarget target) {
  Expression n2 = squareNorm(context, angleUnit, target);
  Expression n =  SquareRoot::Builder(n2);
  n2.shallowReduce(context, angleUnit, target);
  return n;
}

Expression ComplexCartesian::argument(Context & context, Preferences::AngleUnit angleUnit, ExpressionNode::ReductionTarget target) {
  Expression a = real();
  Expression b = imag();
  if (!b.isRationalZero()) {
    // if b != 0, argument = sign(b) * Pi/2 - arctan(a/b)
    // First, compute arctan(a/b) or (Pi/180)*arctan(a/b)
    Expression divab = Division(a, b.clone());
    Expression arcTangent = ArcTangent::Builder(divab);
    divab.shallowReduce(context, angleUnit, target);
    if (angleUnit == Preferences::AngleUnit::Degree) {
      Expression temp = arcTangent.degreeToRadian();
      arcTangent.shallowReduce(context, angleUnit, target);
      arcTangent = temp;
    }
    // Then, compute sign(b) * Pi/2 - arctan(a/b)
    Expression signb = SignFunction::Builder(b);
    Expression signbPi2 = Multiplication(Rational(1,2), signb, Constant(Ion::Charset::SmallPi));
    signb.shallowReduce(context, angleUnit, target);
    Expression sub = Subtraction(signbPi2, arcTangent);
    signbPi2.shallowReduce(context, angleUnit, target);
    arcTangent.shallowReduce(context, angleUnit, target);
    return sub;
  } else {
    // if b == 0, argument = (1-sign(a))*Pi/2
    Expression signa = SignFunction::Builder(a).shallowReduce(context, angleUnit);
    Subtraction sub(Rational(1), signa);
    signa.shallowReduce(context, angleUnit, target);
    Multiplication mul(Rational(1,2), Constant(Ion::Charset::SmallPi), sub);
    sub.shallowReduce(context, angleUnit, target);
    return mul;
  }
}

ComplexCartesian ComplexCartesian::inverse(Context & context, Preferences::AngleUnit angleUnit, ExpressionNode::ReductionTarget target) {
  Expression a = real();
  Expression b = imag();
  // 1/(a+ib) = a/(a^2+b^2)+i*(-b/(a^2+b^2))
  Expression denominatorReal = clone().convert<ComplexCartesian>().squareNorm(context, angleUnit, target);
  Expression denominatorImag = denominatorReal.clone();
  Expression denominatorRealInv = Power(denominatorReal, Rational(-1));
  denominatorReal.shallowReduce(context, angleUnit, target);
  Expression denominatorImagInv = Power(denominatorImag, Rational(-1));
  denominatorImag.shallowReduce(context, angleUnit, target);
  Multiplication A(a, denominatorRealInv);
  denominatorRealInv.shallowReduce(context, angleUnit, target);
  Expression numeratorImag = Multiplication(Rational(-1), b);
  Multiplication B(numeratorImag, denominatorImagInv);
  numeratorImag.shallowReduce(context, angleUnit, target);
  denominatorImagInv.shallowReduce(context, angleUnit, target);
  ComplexCartesian result(A,B);
  A.shallowReduce(context, angleUnit, target);
  B.shallowReduce(context, angleUnit, target);
  return result;
}

Multiplication ComplexCartesian::squareRootHelper(Expression e, Context & context, Preferences::AngleUnit angleUnit, ExpressionNode::ReductionTarget target) {
  //(1/2)*sqrt(2*e)
  Multiplication doubleE(Rational(2), e);
  e.shallowReduce(context, angleUnit, target);
  Expression sqrt = SquareRoot::Builder(doubleE);
  doubleE.shallowReduce(context, angleUnit, target);
  Multiplication result(Rational(1,2), sqrt);
  sqrt.shallowReduce(context, angleUnit, target);
  return result;
}

ComplexCartesian ComplexCartesian::squareRoot(Context & context, Preferences::AngleUnit angleUnit, ExpressionNode::ReductionTarget target) {
  Expression a = real();
  Expression b = imag();
  // A: (1/2)*sqrt(2*(sqrt(a^2+b^2)+a))
  // B: (1/2)*sqrt(2*(sqrt(a^2+b^2)-a))*sign(b)
  Expression normA = clone().convert<ComplexCartesian>().norm(context, angleUnit, target);
  Expression normB = normA.clone();
  // A = (1/2)*sqrt(2*(sqrt(a^2+b^2)+a))
  Addition normAdda(normA, a.clone());
  normA.shallowReduce(context, angleUnit, target);
  Multiplication A = squareRootHelper(normAdda, context, angleUnit, target);
  // B = B: (1/2)*sqrt(2*(sqrt(a^2+b^2)-a))
  Subtraction normSuba(normB, a);
  normB.shallowReduce(context, angleUnit, target);
  Multiplication B = squareRootHelper(normSuba, context, angleUnit, target);
  // B = B: (1/2)*sqrt(2*(sqrt(a^2+b^2)-a))*sign(b)
  Expression signb = SignFunction::Builder(b);
  B.addChildAtIndexInPlace(signb, B.numberOfChildren(), B.numberOfChildren());
  signb.shallowReduce(context, angleUnit, target);
  ComplexCartesian result = ComplexCartesian::Builder(A, B);
  A.shallowReduce(context, angleUnit, target);
  B.shallowReduce(context, angleUnit, target);
  return result;
}


ComplexCartesian ComplexCartesian::powerInteger(int n, Context & context, Preferences::AngleUnit angleUnit, ExpressionNode::ReductionTarget target) {
  return ComplexCartesian();
}

ComplexCartesian ComplexCartesian::multiply(ComplexCartesian & other, Context & context, Preferences::AngleUnit angleUnit, ExpressionNode::ReductionTarget target) {
  Expression a = real();
  Expression b = imag();
  Expression c = other.real();
  Expression d = other.imag();
  // (a+ib) * (c+id) = (ac-bd)+i*(ad+bc)
  // Compute ac-bd
  Expression ac =  Multiplication(a.clone(), c.clone());
  Expression bd =  Multiplication(b.clone(), d.clone());
  Subtraction A(ac, bd);
  ac.shallowReduce(context, angleUnit, target);
  bd.shallowReduce(context, angleUnit, target);
  // Compute ad+bc
  Expression ad =  Multiplication(a, d);
  Expression bc =  Multiplication(b, c);
  Addition B(ad, bc);
  ad.shallowReduce(context, angleUnit, target);
  bc.shallowReduce(context, angleUnit, target);
  ComplexCartesian result = ComplexCartesian::Builder(A, B);
  A.shallowReduce(context, angleUnit, target);
  B.shallowReduce(context, angleUnit, target);
  return result;
}

Expression ComplexCartesian::powerHelper(Expression norm, Expression trigo, Context & context, Preferences::AngleUnit angleUnit, ExpressionNode::ReductionTarget target) {
  Multiplication m(norm, trigo);
  norm.shallowReduce(context, angleUnit, target);
  trigo.shallowReduce(context, angleUnit, target);
  return m;
}

ComplexCartesian ComplexCartesian::power(ComplexCartesian & other, Context & context, Preferences::AngleUnit angleUnit, ExpressionNode::ReductionTarget target) {
  Expression r = clone().convert<ComplexCartesian>().norm(context, angleUnit, target);
  Expression rclone = r.clone();
  Expression th = argument(context, angleUnit, target);
  Expression thclone = th.clone();
  Expression c = other.real();
  Expression d = other.imag();
  // R = r^c*e^(-th*d)
  Expression rpowc = Power(rclone, c);
  rclone.shallowReduce(context, angleUnit, target);
  Expression thmuld = Multiplication(Rational(-1), thclone, d.clone());
  thclone.shallowReduce(context, angleUnit, target);
  Expression exp = Power(Constant(Ion::Charset::Exponential), thmuld);
  thmuld.shallowReduce(context, angleUnit, target);
  Multiplication norm(rpowc, exp);
  rpowc.shallowReduce(context, angleUnit, target);
  exp.shallowReduce(context, angleUnit, target);

  // TH = d*ln(r)+c*th
  Expression lnr = NaperianLogarithm::Builder(r);
  r.shallowReduce(context, angleUnit, target);
  Multiplication dlnr(d, lnr);
  lnr.shallowReduce(context, angleUnit, target);
  Multiplication thc(th, c);
  th.shallowReduce(context, angleUnit, target);
  Expression argument = Addition(thc, dlnr);
  thc.shallowReduce(context, angleUnit, target);
  dlnr.shallowReduce(context, angleUnit, target);

  if (angleUnit == Preferences::AngleUnit::Degree) {
    Expression temp = argument.radianToDegree();
    argument.shallowReduce(context, angleUnit, target);
    argument = temp;
  }
  // Result = (norm*cos(argument), norm*sin(argument))
  Expression normClone = norm.clone();
  Expression argClone = argument.clone();
  Expression cos = Cosine::Builder(argClone);
  argClone.shallowReduce(context, angleUnit, target);
  Expression normcosarg = powerHelper(normClone, cos, context, angleUnit, target);
  Expression sin = Sine::Builder(argument);
  argument.shallowReduce(context, angleUnit, target);
  Expression normsinarg = powerHelper(norm, sin, context, angleUnit, target);
  ComplexCartesian result = ComplexCartesian::Builder(normcosarg, normsinarg);
  normcosarg.shallowReduce(context, angleUnit, target);
  normsinarg.shallowReduce(context, angleUnit, target);
  return result;
}

}
