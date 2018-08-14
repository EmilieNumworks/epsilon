#include <poincare/integer.h>
#include <poincare/ieee754.h>
#include <poincare/layout_helper.h>
#include <poincare/char_layout_node.h>
#include <poincare/rational.h>
#include <cmath>
#include <utility>
extern "C" {
#include <stdlib.h>
#include <string.h>
#include <assert.h>
}
#include <cmath>
#include <poincare/ieee754.h>

namespace Poincare {

static inline int max(int x, int y) { return (x>y ? x : y); }

uint8_t log2(native_uint_t v) {
  constexpr int nativeUnsignedIntegerBitCount = 8*sizeof(native_uint_t);
  static_assert(nativeUnsignedIntegerBitCount < 256, "uint8_t cannot contain the log2 of a native_uint_t");
  for (uint8_t i=0; i<nativeUnsignedIntegerBitCount; i++) {
    if (v < ((native_uint_t)1<<i)) {
      return i;
    }
  }
  return 32;
}

static inline char char_from_digit(native_uint_t digit) {
  return '0'+digit;
}

static inline int8_t sign(bool negative) {
  return 1 - 2*(int8_t)negative;
}

/* Natural Integer Abstract */

// Layout

int NaturalIntegerAbstract::serialize(char * buffer, int bufferSize) const {
  if (bufferSize == 0) {
    return -1;
  }
  buffer[bufferSize-1] = 0;
  if (isInfinity()) {
    return PrintFloat::convertFloatToText<float>(INFINITY, buffer, bufferSize, PrintFloat::k_numberOfStoredSignificantDigits, Preferences::PrintFloatMode::Decimal);
  }

  Integer base(10);
  Integer ref(this);
  IntegerDivision d = Integer::Division(ref, base);
  int size = 0;
  if (bufferSize == 1) {
    return 0;
  }
  if (isZero()) {
    buffer[size++] = '0';
  }
  while (!(d.remainder.isZero() &&
        d.quotient.isZero())) {
    char c = char_from_digit(d.remainder.digit(0));
    if (size >= bufferSize-1) {
      return strlcpy(buffer, "undef", bufferSize);
    }
    buffer[size++] = c;
    d = Integer::Division(d.quotient, base);
  }
  buffer[size] = 0;

  // Flip the string
  for (int i=0, j=size-1 ; i < j ; i++, j--) {
    char c = buffer[i];
    buffer[i] = buffer[j];
    buffer[j] = c;
  }
  return size;
}

HorizontalLayoutRef NaturalIntegerAbstract::createLayout() const {
  char buffer[k_maxNumberOfDigitsBase10];
  int numberOfChars = serialize(buffer, k_maxNumberOfDigitsBase10);
  return LayoutHelper::String(buffer, numberOfChars);
}

// Approximation

template<typename T>
T NaturalIntegerAbstract::approximate() const {
  if (m_numberOfDigits == 0) {
    /* This special case for 0 is needed, because the current algorithm assumes
     * that the big integer is non zero, thus puts the exponent to 126 (integer
     * area), the issue is that when the mantissa is 0, a "shadow bit" is
     * assumed to be there, thus 126 0x000000 is equal to 0.5 and not zero.
     */
    return (T)0.0;
  }
  assert(sizeof(T) == 4 || sizeof(T) == 8);
  /* We're generating an IEEE 754 compliant float(double).
  * We can tell that:
  * - the sign depends on m_negative
  * - the exponent is the length of our BigInt, in bits - 1 + 127 (-1+1023);
  * - the mantissa is the beginning of our BigInt, discarding the first bit
  */

  if (isInfinity()) {
    return INFINITY;
  }

  native_uint_t lastDigit = digit(m_numberOfDigits-1);
  uint8_t numberOfBitsInLastDigit = log2(lastDigit);

  uint16_t exponent = IEEE754<T>::exponentOffset();
  /* Escape case if the exponent is too big to be stored */
  assert(m_numberOfDigits > 0);
  if (((int)m_numberOfDigits-1)*32+numberOfBitsInLastDigit-1> IEEE754<T>::maxExponent()-IEEE754<T>::exponentOffset()) {
    return INFINITY;
  }
  exponent += (m_numberOfDigits-1)*32;
  exponent += numberOfBitsInLastDigit-1;

  uint64_t mantissa = 0;
  /* Shift the most significant int to the left of the mantissa. The most
   * significant 1 will be ignore at the end when inserting the mantissa in
   * the resulting uint64_t (as required by IEEE754). */
  assert(IEEE754<T>::size()-numberOfBitsInLastDigit >= 0 && IEEE754<T>::size()-numberOfBitsInLastDigit < 64); // Shift operator behavior is undefined if the right operand is negative, or greater than or equal to the length in bits of the promoted left operand
  mantissa |= ((uint64_t)lastDigit << (IEEE754<T>::size()-numberOfBitsInLastDigit));
  size_t digitIndex = 2;
  int numberOfBits = numberOfBitsInLastDigit;
  /* Complete the mantissa by inserting, from left to right, every digit of the
   * Integer from the most significant one to the last from. We break when
   * the mantissa is complete to avoid undefined right shifting (Shift operator
   * behavior is undefined if the right operand is negative, or greater than or
   * equal to the length in bits of the promoted left operand). */
  while (m_numberOfDigits >= digitIndex && numberOfBits < IEEE754<T>::size()) {
    lastDigit = digit(m_numberOfDigits-digitIndex);
    numberOfBits += 32;
    if (IEEE754<T>::size() > numberOfBits) {
      assert(IEEE754<T>::size()-numberOfBits > 0 && IEEE754<T>::size()-numberOfBits < 64);
      mantissa |= ((uint64_t)lastDigit << (IEEE754<T>::size()-numberOfBits));
    } else {
      mantissa |= ((uint64_t)lastDigit >> (numberOfBits-IEEE754<T>::size()));
    }
    digitIndex++;
  }

  T result = IEEE754<T>::buildFloat(false, exponent, mantissa);

  /* If exponent is 255 and the float is undefined, we have exceed IEEE 754
   * representable float. */
  if (exponent == IEEE754<T>::maxExponent() && std::isnan(result)) {
    return INFINITY;
  }
  return result;
}

// Properties

int NaturalIntegerAbstract::NumberOfBase10Digits(const NaturalIntegerAbstract * i) {
  assert(!i->isInfinity());
  int numberOfDigits = 1;
  Integer ref(i);
  Integer base(10);
  IntegerDivision d = Integer::Division(ref, base);
  while (!d.quotient.isZero()) {
    ref = d.quotient;
    d = Integer::Division(ref, base);
    numberOfDigits++;
  }
  return numberOfDigits;
}

// Arithmetic

int8_t NaturalIntegerAbstract::ucmp(const NaturalIntegerAbstract * a, const NaturalIntegerAbstract * b) {
  if (a->m_numberOfDigits < b->m_numberOfDigits) {
    return -1;
  } else if (a->m_numberOfDigits > b->m_numberOfDigits) {
    return 1;
  }
  for (uint16_t i = 0; i < a->m_numberOfDigits; i++) {
    // Digits are stored most-significant last
    native_uint_t aDigit = a->digit(a->m_numberOfDigits-i-1);
    native_uint_t bDigit = b->digit(b->m_numberOfDigits-i-1);
    if (aDigit < bDigit) {
      return -1;
    } else if (aDigit > bDigit) {
      return 1;
    }
  }
  return 0;
}

Integer NaturalIntegerAbstract::usum(const NaturalIntegerAbstract * a, const NaturalIntegerAbstract * b, bool subtract) {
  size_t size = max(a->m_numberOfDigits, b->m_numberOfDigits);
  if (!subtract) {
    // Addition can overflow
    size++;
  }
  // Overflow
  if (size > k_maxNumberOfDigits + 1) {
    return Integer::Overflow();
  }
  native_uint_t digits[k_maxNumberOfDigits+1];
  bool carry = false;
  for (size_t i = 0; i < size; i++) {
    native_uint_t aDigit = (i >= a->m_numberOfDigits ? 0 : a->digit(i));
    native_uint_t bDigit = (i >= b->m_numberOfDigits ? 0 : b->digit(i));
    native_uint_t result = (subtract ? aDigit - bDigit - carry : aDigit + bDigit + carry);
    digits[i] = result;
    if (subtract) {
      carry = (aDigit < result) || (carry && aDigit == result); // There's been an underflow
    } else {
      carry = (aDigit > result) || (bDigit > result); // There's been an overflow
    }
  }
  while (digits[size-1] == 0 && size>0) {
    size--;
  }
  if (size > k_maxNumberOfDigits) {
    return Integer::Overflow();
  }
  return Integer(digits, size, false);
}

Integer NaturalIntegerAbstract::umult(const NaturalIntegerAbstract * a, const NaturalIntegerAbstract * b){
  size_t size = a->m_numberOfDigits + b->m_numberOfDigits;
  // Overflow
  if (size > k_maxNumberOfDigits + 1) {
    return Integer::Overflow();
  }

  native_uint_t digits[k_maxNumberOfDigits+1];
  memset(digits, 0, size*sizeof(native_uint_t));

  double_native_uint_t carry = 0;
  for (size_t i=0; i<a->m_numberOfDigits; i++) {
    double_native_uint_t aDigit = a->digit(i);
    carry = 0;
    for (size_t j=0; j<b->m_numberOfDigits; j++) {
      double_native_uint_t bDigit = b->digit(j);
      /* The fact that aDigit and bDigit are double_native is very important,
       * otherwise the product might end up being computed on single_native size
       * and then zero-padded. */
      double_native_uint_t p = aDigit*bDigit + carry + (double_native_uint_t)(digits[i+j]); // TODO: Prove it cannot overflow double_native type
      native_uint_t * l = (native_uint_t *)&p;
      digits[i+j] = l[0];
      carry = l[1];
    }
    digits[i+b->m_numberOfDigits] += carry;
  }

  while (digits[size-1] == 0 && size>0) {
    size--;
  }
  // Overflow
  if (size > k_maxNumberOfDigits + 1) {
    return Integer::Overflow();
  }
  return Integer(digits, size, false);
}

// TODO: OPTIMIZE
IntegerDivision NaturalIntegerAbstract::udiv(const NaturalIntegerAbstract * numerator, const NaturalIntegerAbstract * denominator) {
  /* Modern Computer Arithmetic, Richard P. Brent and Paul Zimmermann
   * (Algorithm 1.6) */
  assert(!denominator->isZero());
  if (ucmp(numerator,denominator) < 0) {
    IntegerDivision div = {.quotient = Integer(0), .remainder = Integer(numerator)};
    return div;
  }

  Integer A(numerator);
  Integer B(denominator);
  native_int_t base = 1 << 16;
  // TODO: optimize by just swifting digit and finding 2^kB that makes B normalized
  native_int_t d = base/(native_int_t)(B.node()->halfDigit(B.node()->numberOfHalfDigits()-1)+1);
  A = Integer::Multiplication(Integer(d), A);
  B = Integer::Multiplication(Integer(d), B);

  int n = B.node()->numberOfHalfDigits();
  int m = A.node()->numberOfHalfDigits()-n;
  half_native_uint_t qDigits[2*k_maxNumberOfDigits]; // qDigits is a half_native_uint_t array
  memset(qDigits, 0, 2*k_maxNumberOfDigits*sizeof(half_native_uint_t));
  Integer betam = IntegerWithHalfDigitAtIndex(1, m+1);
  Integer betaMB = Integer::Multiplication(betam, B); // TODO: can swift all digits by m! B.swift16(mg)
  if (Integer::NaturalOrder(A,betaMB) > 0) {
    qDigits[m] = 1;
    A = Integer::Subtraction(A, betaMB);
  }
  for (int j = m-1; j >= 0; j--) {
    native_uint_t qj2 = ((native_uint_t)A.node()->halfDigit(n+j)*base+(native_uint_t)A.node()->halfDigit(n+j-1))/(native_uint_t)B.node()->halfDigit(n-1);
    half_native_uint_t baseMinus1 = (1 << 16) -1;
    qDigits[j] = qj2 < (native_uint_t)baseMinus1 ? (half_native_uint_t)qj2 : baseMinus1;
    Integer factor = qDigits[j] > 0 ? IntegerWithHalfDigitAtIndex(qDigits[j], j+1) : Integer(0);
    A = Integer::Subtraction(A, Integer::Multiplication(factor, B));
    Integer m = Integer::Multiplication(IntegerWithHalfDigitAtIndex(1, j+1), B);
    while (A.sign() == ExpressionNode::Sign::Negative) {
      qDigits[j] = qDigits[j]-1;
      A = Integer::Addition(A, m);
    }
  }
  int qNumberOfDigits = m+1;
  while (qDigits[qNumberOfDigits-1] == 0 && qNumberOfDigits > 1) {
    qNumberOfDigits--;
  }
  int qNumberOfDigitsInBase32 = qNumberOfDigits%2 == 1 ? qNumberOfDigits/2+1 : qNumberOfDigits/2;
  IntegerDivision div = {.quotient = Integer((native_uint_t *)qDigits, qNumberOfDigitsInBase32, false), .remainder = A};
  if (d != 1 && !div.remainder.isZero()) {
    Integer dInteger(d);
    div.remainder = udiv(div.remainder.node(), dInteger.node()).quotient;
  }
  return div;
}

Integer NaturalIntegerAbstract::upow(const NaturalIntegerAbstract * i, const NaturalIntegerAbstract * j) {
  // TODO: optimize with dichotomia
  Integer index(j);
  Integer result(1);
  while (!index.isZero()) {
    result = Integer::Multiplication(result, i);
    index = Integer::Subtraction(index, Integer(1));
  }
  return result;
}

Integer NaturalIntegerAbstract::ufact(const NaturalIntegerAbstract * i) {
  Integer j(2);
  Integer result(1);
  while (ucmp(i,j.node()) > 0) {
    result = Integer::Multiplication(j, result);
    j = Integer::Addition(j, Integer(1));
  }
  return result;
}

Integer NaturalIntegerAbstract::IntegerWithHalfDigitAtIndex(half_native_uint_t halfDigit, int index) {
  assert(halfDigit != 0);
  // Overflow
  int indexInBase32 = index%2 == 1 ? index/2+1 : index/2;
  if (indexInBase32 > k_maxNumberOfDigits) {
    return Integer::Overflow();
  }
  half_native_uint_t digits[k_maxNumberOfDigits+1];
  memset(digits, 0, indexInBase32*sizeof(native_uint_t));
  assert(index > 0);
  digits[index-1] = halfDigit;
  return Integer((native_uint_t *)digits, indexInBase32, false);
}

/* Natural Integer Pointer */

NaturalIntegerPointer::NaturalIntegerPointer(native_uint_t * buffer, size_t size) :
  NaturalIntegerAbstract(size > k_maxNumberOfDigits ? k_maxNumberOfDigits+1 : size),
  m_digits(size > k_maxNumberOfDigits || size == 0 ? nullptr : buffer)
{}

/* Integer Node */

IntegerNode * IntegerNode::FailedAllocationStaticNode() {
  static AllocationFailureIntegerNode failure;
  TreePool::sharedPool()->registerStaticNodeIfRequired(&failure);
  return &failure;
}

void IntegerNode::setDigits(const native_uint_t * digits, size_t size, bool negative) {
  if (digits) {
    memcpy(m_digits, digits, size*sizeof(native_uint_t));
  }
  m_numberOfDigits = size;
  m_negative = negative;
}

template<typename T>
T IntegerNode::templatedApproximate() const {
  T a = NaturalIntegerAbstract::approximate<T>();
  return m_negative ? -a : a;
}

Expression IntegerNode::shallowReduce(Context & context, Preferences::AngleUnit angleUnit) const {
  return Integer(this).shallowReduce(context, angleUnit);
}

// Layout

LayoutRef IntegerNode::createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  HorizontalLayoutRef naturalLayout = NaturalIntegerAbstract::createLayout();
  if (m_negative) {
    naturalLayout.addChildAtIndex(CharLayoutRef('-'), 0, naturalLayout.numberOfChildren(), nullptr);
  }
  return naturalLayout;
}

int IntegerNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  if (bufferSize == 0) {
    return -1;
  }
  buffer[bufferSize-1] = 0;
  int numberOfChar = 0;
  if (m_negative) {
    buffer[numberOfChar++] = '-';
  }
  if (numberOfChar >= bufferSize-1) {
    return bufferSize-1;
  }
  numberOfChar += NaturalIntegerAbstract::serialize(buffer+numberOfChar, bufferSize-numberOfChar);
  return numberOfChar;
}

size_t IntegerNode::size() const {
  return m_numberOfDigits*sizeof(native_uint_t)+sizeof(IntegerNode);
}

Expression IntegerNode::setSign(Sign s, Context & context, Preferences::AngleUnit angleUnit) const {
  return Integer(this).setSign(s, context, angleUnit);
}

void IntegerNode::setNegative(bool negative) {
  if (numberOfDigits() == 0) { // Zero cannot be negative
    return;
  }
  m_negative = negative;
}

int IntegerNode::NaturalOrder(const IntegerNode * i, const IntegerNode * j) {
  if (i->sign() == Sign::Negative && j->sign() == Sign::Positive) {
    return -1;
  }
  if (i->sign() == Sign::Positive && j->sign() == Sign::Negative) {
    return 1;
  }
  return ::Poincare::sign(i->sign() == Sign::Negative)*ucmp(i, j);
}

/* Integer */

Integer::Integer(size_t size, const native_uint_t * digits, size_t numberOfDigits, bool negative) : Number(TreePool::sharedPool()->createTreeNode<IntegerNode>(size)) {
  if (numberOfDigits == 1 && digits[0] == 0) {
    negative = false;
  }
  node()->setDigits(digits, numberOfDigits, negative);
}

size_t cappedNumberOfDigits(size_t numberOfDigits, bool enableOverflow) {
  if (!enableOverflow && numberOfDigits > NaturalIntegerAbstract::k_maxNumberOfDigits) {
    return NaturalIntegerAbstract::k_maxNumberOfDigits+1;
  }
  return numberOfDigits;
}
Integer::Integer(const native_uint_t * digits, size_t numberOfDigits, bool negative, bool enableOverflow) :
  Integer(sizeof(IntegerNode)+cappedNumberOfDigits(numberOfDigits, enableOverflow)*sizeof(native_uint_t), digits, cappedNumberOfDigits(numberOfDigits, enableOverflow), negative)
{}

Integer::Integer(const char * digits, size_t length, bool negative) :
  Integer(0)
{
  if (digits != nullptr && digits[0] == '-') {
    negative = true;
    digits++;
    length--;
  }
  if (digits != nullptr) {
    Integer base(10);
    for (size_t i = 0; i < length; i++) {
      *this = Multiplication(*this, base);
      *this = Addition(*this, Integer(*digits-'0'));
      digits++;
    }
  }

  setNegative(isZero() ? false : negative);
}

Integer::Integer(const NaturalIntegerAbstract * naturalInteger) :
  Integer(naturalInteger->digits(), naturalInteger->numberOfDigits(), false)
{
}

Integer::Integer(native_int_t i) : Number() {
  if (i == 0) {
    new (this) Integer((const native_uint_t *)nullptr, 0, false);
    return;
  }
  native_uint_t digits[1];
  digits[0] = i < 0 ? -i : i;
  new (this) Integer(digits, 1, i < 0);
}

Integer::Integer(double_native_int_t i) : Number() {
  if (i == 0) {
    new (this) Integer((const native_uint_t *)nullptr, 0, false);
    return;
  }
  double_native_uint_t j = i < 0 ? -i : i;
  native_uint_t * digits = (native_uint_t *)&j;
  native_uint_t leastSignificantDigit = *digits;
  native_uint_t mostSignificantDigit = *(digits+1);
  native_uint_t digitsArray[2] = {leastSignificantDigit, mostSignificantDigit};
  if (mostSignificantDigit == 0) {
    new (this) Integer(digitsArray, 1, i < 0);
    return;
  }
  new (this) Integer(digitsArray, 2, i < 0);
}

int Integer::extractedInt() const {
  assert(numberOfDigits() == 1 && digit(0) <= k_maxExtractableInteger);
  return node()->sign() == ExpressionNode::Sign::Negative ? -digit(0) : digit(0);
}

// Arithmetic

Integer Integer::Multiplication(const Integer & a, const Integer & b) {
  Integer um = IntegerNode::umult(a.node(), b.node());
  um.setNegative(a.sign() != b.sign());
  return um;
}

IntegerDivision Integer::Division(const Integer & numerator, const Integer & denominator) {
  IntegerDivision ud = IntegerNode::udiv(numerator.node(), denominator.node());
  if (numerator.sign() == ExpressionNode::Sign::Positive && denominator.sign() == ExpressionNode::Sign::Positive) {
    return ud;
  }
  if (NaturalOrder(ud.remainder, Integer(0)) == 0) {
    if (numerator.sign() == ExpressionNode::Sign::Positive || denominator.sign() == ExpressionNode::Sign::Positive) {
      ud.quotient.setNegative(true);
    }
    return ud;
  }
  if (numerator.sign() == ExpressionNode::Sign::Negative) {
    if (denominator.sign() == ExpressionNode::Sign::Negative) {
      ud.remainder.setNegative(true);
      ud.quotient = Addition(ud.quotient, Integer(1));
      ud.remainder = Subtraction(ud.remainder, denominator);
   } else {
      ud.quotient.setNegative(true);
      ud.quotient = Subtraction(ud.quotient, Integer(1));
      ud.remainder = Subtraction(denominator, ud.remainder);
    }
  } else {
    assert(denominator.sign() == ExpressionNode::Sign::Negative);
    ud.quotient.setNegative(true);
  }
  return ud;
}

Integer Integer::Power(const Integer & i, const Integer & j) {
  assert(j.sign() == ExpressionNode::Sign::Positive);
  Integer upow = IntegerNode::upow(i.node(), j.node());
  upow.setNegative(i.sign() == ExpressionNode::Sign::Negative && !j.isEven());
  return upow;
}

Integer Integer::Factorial(const Integer & i) {
  assert(i.sign() == ExpressionNode::Sign::Positive);
  return IntegerNode::ufact(i.node());
}

Integer Integer::addition(const Integer & a, const Integer & b, bool inverseBNegative) {
  bool bNegative = (inverseBNegative ? b.sign() == ExpressionNode::Sign::Positive : b.sign() == ExpressionNode::Sign::Negative);
  if ((a.sign() == ExpressionNode::Sign::Negative) == bNegative) {
    Integer us = IntegerNode::usum(a.node(), b.node(), false);
    us.setNegative(a.sign() == ExpressionNode::Sign::Negative);
    return us;
  } else {
    /* The signs are different, this is in fact a subtraction
     * s = a+b = (abs(a)-abs(b) OR abs(b)-abs(a))
     * 1/abs(a)>abs(b) : s = sign*udiff(a, b)
     * 2/abs(b)>abs(a) : s = sign*udiff(b, a)
     * sign? sign of the greater! */
    if (IntegerNode::ucmp(a.node(), b.node()) >= 0) {
      Integer us = IntegerNode::usum(a.node(), b.node(), true);
      us.setNegative(a.sign() == ExpressionNode::Sign::Negative);
      return us;
    } else {
      Integer us = IntegerNode::usum(b.node(), a.node(), true);
      us.setNegative(bNegative);
      return us;
    }
  }
}

Expression Integer::shallowReduce(Context & context, Preferences::AngleUnit angleUnit) const {
  return Rational(*this, Integer(1));
}

Expression Integer::setSign(ExpressionNode::Sign s, Context & context, Preferences::AngleUnit angleUnit) const {
  Integer signedInteger = *this;
  signedInteger.setNegative(s == ExpressionNode::Sign::Negative);
  return signedInteger;
}

template float IntegerNode::templatedApproximate<float>() const;
template double IntegerNode::templatedApproximate<double>() const;

}
