extern "C" {
#include <assert.h>
#include <float.h>
#include <stdlib.h>
}
#include <poincare/complex.h>
#include <poincare/division.h>
#include <poincare/expression.h>
#include <poincare/allocation_failure_evaluation_node.h>
#include <poincare/undefined.h>
#include <poincare/infinity.h>
#include <poincare/decimal.h>
#include <poincare/multiplication.h>
#include <poincare/opposite.h>
#include <poincare/addition.h>
#include <poincare/subtraction.h>
#include <poincare/power.h>
#include <poincare/symbol.h>
#include <ion.h>
#include <cmath>

namespace Poincare {

template<typename T> using AllocationFailureComplexNode = AllocationFailureEvaluationNode<ComplexNode, T>;

template<typename T>
ComplexNode<T> * ComplexNode<T>::FailedAllocationStaticNode() {
  static AllocationFailureComplexNode<T> failure;
  TreePool::sharedPool()->registerStaticNodeIfRequired(&failure);
  return &failure;
}

template<typename T>
void ComplexNode<T>::setComplex(std::complex<T> c) {
  this->real(c.real());
  this->imag(c.imag());
  if (this->real() == -0) {
    this->real(0);
  }
  if (this->imag() == -0) {
    this->imag(0);
  }
}

template<typename T>
T ComplexNode<T>::toScalar() const {
  if (this->imag() == 0.0) {
    return this->real();
  }
  return NAN;
}

template<typename T>
Expression ComplexNode<T>::complexToExpression(Preferences::ComplexFormat complexFormat) const {
  if (std::isnan(this->real()) || std::isnan(this->imag())) {
    return Undefined();
  }
  switch (complexFormat) {
    case Preferences::ComplexFormat::Cartesian:
    {
      Expression real;
      Expression imag;
      if (this->real() != 0 || this->imag() == 0) {
        real = Number::DecimalNumber<T>(this->real());
      }
      if (this->imag() != 0) {
        if (this->imag() == 1.0 || this->imag() == -1) {
          imag = Symbol(Ion::Charset::IComplex);
        } else if (this->imag() > 0) {
          imag = Multiplication(Number::DecimalNumber(this->imag()), Symbol(Ion::Charset::IComplex));
        } else {
          imag = Multiplication(Number::DecimalNumber(-this->imag()), Symbol(Ion::Charset::IComplex));
        }
      }
      if (!imag.isDefined()) {
        return real;
      } else if (!real.isDefined()) {
        if (this->imag() > 0) {
          return imag;
        } else {
          return Opposite(imag);
        }
        return imag;
      } else if (this->imag() > 0) {
        return Addition(real, imag);
      } else {
        return Subtraction(real, imag);
      }
    }
    default:
    {
      assert(complexFormat == Preferences::ComplexFormat::Polar);
      Expression norm;
      Expression exp;
      T r = std::abs(*this);
      T th = std::arg(*this);
      if (r != 1 || th == 0) {
        norm = Number::DecimalNumber(r);
      }
      if (r != 0 && th != 0) {
        Expression arg;
        if (th == 1.0) {
          arg = Symbol(Ion::Charset::IComplex);
        } else if (th == -1.0) {
          arg = Opposite(Symbol(Ion::Charset::IComplex));
        } else if (th > 0) {
          arg = Multiplication(Number::DecimalNumber(th), Symbol(Ion::Charset::IComplex));
        } else {
          arg = Opposite(Multiplication(Number::DecimalNumber(-th), Symbol(Ion::Charset::IComplex)));
        }
        exp = Power(Symbol(Ion::Charset::Exponential), arg);
      }
      if (!exp.isDefined()) {
        return norm;
      } else if (!norm.isDefined()) {
        return exp;
      } else {
        return Multiplication(norm, exp);
      }
    }
  }
}

template<typename T>
Complex<T>::Complex(std::complex<T> c) :
  Evaluation<T>(TreePool::sharedPool()->createTreeNode<ComplexNode<T>>())
{
  static_cast<ComplexNode<T> *>(this->node())->setComplex(c);
}

template class ComplexNode<float>;
template class ComplexNode<double>;
template class Complex<float>;
template class Complex<double>;

}
