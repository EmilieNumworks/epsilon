#include <poincare/addition.h>
#include <poincare/complex_matrix.h>
#include <poincare/multiplication.h>
#include <poincare/subtraction.h>
#include <poincare/power.h>
#include <poincare/opposite.h>
#include <poincare/undefined.h>
extern "C" {
#include <assert.h>
#include <stdlib.h>
}

namespace Poincare {

Expression::Type Addition::type() const {
  return Type::Addition;
}

Expression * Addition::clone() const {
  return new Addition(operands(), numberOfOperands(), true);
}

/* Simplication */

Expression * Addition::shallowSimplify(Context& context, AngleUnit angleUnit) {
  /* Step 1: Addition is associative, so let's start by merging children which
   * also are additions themselves. */
  int i = 0;
  int initialNumberOfOperands = numberOfOperands();
  while (i < initialNumberOfOperands) {
    Expression * o = editableOperand(i);
    if (o->type() == Type::Addition) {
      mergeOperands(static_cast<Addition *>(o));
      continue;
    }
    i++;
  }

  // Step 2: Sort the operands
  sortOperands(Expression::SimplificationOrder);

  /* Step 3: Factorize like terms. Thanks to the simplification order, those are
   * next to each other at this point. */
  i = 0;
  while (i < numberOfOperands()-1) {
    Expression * o1 = editableOperand(i);
    Expression * o2 = editableOperand(i+1);
    if (o1->type() == Type::Rational && o2->type() == Type::Rational) {
      Rational r1 = *static_cast<Rational *>(o1);
      Rational r2 = *static_cast<Rational *>(o2);
      Rational a = Rational::Addition(r1, r2);
      replaceOperand(o1, new Rational(a), true);
      removeOperand(o2, true);
      continue;
    }
    if (TermsHaveIdenticalNonRationalFactors(o1, o2)) {
      factorizeOperands(o1, o2, context, angleUnit);
      continue;
    }
    i++;
  }

  /* Step 4: Let's remove zeroes if there's any. It's important to do this after
   * having factorized because factorization can lead to new zeroes. For example
   * pi+(-1)*pi. We don't remove the last zero if it's the only operand left
   * though. */
  i = 0;
  while (i < numberOfOperands()) {
    Expression * o = editableOperand(i);
    if (o->type() == Type::Rational && static_cast<Rational *>(o)->isZero() && numberOfOperands() > 1) {
      removeOperand(o, true);
      continue;
    }
    i++;
  }

  // Step 5: Let's remove the addition altogether if it has a single operand
  Expression * result = squashUnaryHierarchy();

  // Step 6: Last but not least, let's put everything under a common denominator
  if (result == this && parent()->type() != Type::Addition) {
    // squashUnaryHierarchy didn't do anything: we're not an unary hierarchy
    result = factorizeOnCommonDenominator(context, angleUnit);
  }

  return result;
}

Expression * Addition::factorizeOnCommonDenominator(Context & context, AngleUnit angleUnit) {
  // We want to turn (a/b+c/d+e/b) into (a*d+b*c+e*d)/(b*d)

  // Step 1: We want to compute the common denominator, b*d
  Multiplication * commonDenominator = new Multiplication();
  for (int i = 0; i < numberOfOperands(); i++) {
    Expression * denominator = operand(i)->cloneDenominator(context, angleUnit);
    if (denominator) {
      // Make commonDenominator = LeastCommonMultiple(commonDenominator, denominator);
      commonDenominator->addMissingFactors(denominator, context, angleUnit);
      delete denominator;
    }
  }
  if (commonDenominator->numberOfOperands() == 0) {
    // If commonDenominator is empty this means that no operand was a fraction.
    return this;
  }

  // Step 2: Create the numerator. We start with this being a/b+c/d+e/b and we
  // want to create numerator = a/b*b*d + c/d*b*d + e/b*b*d
  Addition * numerator = new Addition();
  for (int i=0; i < numberOfOperands(); i++) {
    numerator->addOperand(new Multiplication(operand(i), commonDenominator, true));
  }
  // Step 4: Add the denominator
  Power * inverseDenominator = new Power(commonDenominator, new Rational(-1), false);
  commonDenominator->deepSimplify(context, angleUnit);

  Multiplication * result = new Multiplication(numerator, inverseDenominator, false);
  // Step 3: Simplify the numerator to a*d + c*b + e*d
  numerator->deepSimplify(context, angleUnit);
  inverseDenominator->shallowSimplify(context, angleUnit);

  result->sortOperands(Expression::SimplificationOrder); // TODO: should shallowSimplify?
  return replaceWith(result, true);
}

void Addition::factorizeOperands(Expression * e1, Expression * e2, Context & context, AngleUnit angleUnit) {
  Rational * r = new Rational(Rational::Addition(RationalFactor(e1), RationalFactor(e2)));
  removeOperand(e2, true);
  if (e1->type() == Type::Multiplication) {
    if (e1->operand(0)->type() == Type::Rational) {
      e1->replaceOperand(e1->operand(0), r, true);
    } else {
      static_cast<Multiplication *>(e1)->addOperand(r);
    }
    e1->shallowSimplify(context, angleUnit);
  } else {
    Multiplication * m = new Multiplication(r, e1, true);
    e1->replaceWith(m, true);
    m->shallowSimplify(context, angleUnit);
  }
}

const Rational Addition::RationalFactor(Expression * e) {
  if (e->type() == Type::Multiplication && e->operand(0)->type() == Type::Rational) {
    return *(static_cast<const Rational *>(e->operand(0)));
  }
  return Rational(Integer(1));
}


bool Addition::TermsHaveIdenticalNonRationalFactors(const Expression * e1, const Expression * e2) {
  if (e1->type() == Type::Multiplication && e2->type() == Type::Multiplication) {
    return Multiplication::HaveSameNonRationalFactors(e1, e2);
  }
  const Expression * f1 = (e1->type() == Type::Multiplication && e1->numberOfOperands() == 2 && e1->operand(0)->type() == Type::Rational) ? e1->operand(1) : e1;
  const Expression * f2 = (e2->type() == Type::Multiplication && e2->numberOfOperands() == 2 && e2->operand(0)->type() == Type::Rational) ? e2->operand(1) : e2;
  return f1->isIdenticalTo(f2);
}

Expression * Addition::shallowBeautify(Context & context, AngleUnit angleUnit) {
  int index = 0;
  while (index < numberOfOperands()) {
    // a+(-1)*b+... -> a-b+...
    if (operand(index)->type() == Type::Multiplication && operand(index)->operand(0)->type() == Type::Rational && operand(index)->operand(0)->sign() == Sign::Negative) {
      Multiplication * m = static_cast<Multiplication *>(editableOperand(index));
      if (static_cast<const Rational *>(operand(index)->operand(0))->isMinusOne()) {
        m->removeOperand(m->operand(0), true);
      } else {
        editableOperand(index)->editableOperand(0)->setSign(Sign::Positive, context, angleUnit);
      }
      Expression * subtractant = m->squashUnaryHierarchy();
      if (index == 0) {
        Opposite * o = new Opposite(subtractant, true);
        replaceOperand(subtractant, o, true);
      } else {
        const Expression * op1 = operand(index-1);
        removeOperand(op1, false);
        Subtraction * s = new Subtraction(op1, subtractant->clone(), false);
        replaceOperand(subtractant, s, true);
      }
    }
    index++;
  }
  return squashUnaryHierarchy();
}

/* Evaluation */

template<typename T>
Complex<T> Addition::compute(const Complex<T> c, const Complex<T> d) {
  return Complex<T>::Cartesian(c.a()+d.a(), c.b()+d.b());
}

template Poincare::Complex<float> Poincare::Addition::compute<float>(Poincare::Complex<float>, Poincare::Complex<float>);
template Poincare::Complex<double> Poincare::Addition::compute<double>(Poincare::Complex<double>, Poincare::Complex<double>);

template Poincare::Evaluation<float>* Poincare::Addition::computeOnMatrices<float>(Poincare::Evaluation<float>*, Poincare::Evaluation<float>*);
template Poincare::Evaluation<double>* Poincare::Addition::computeOnMatrices<double>(Poincare::Evaluation<double>*, Poincare::Evaluation<double>*);

template Poincare::Evaluation<float>* Poincare::Addition::computeOnComplexAndMatrix<float>(Poincare::Complex<float> const*, Poincare::Evaluation<float>*);
template Poincare::Evaluation<double>* Poincare::Addition::computeOnComplexAndMatrix<double>(Poincare::Complex<double> const*, Poincare::Evaluation<double>*);

}
