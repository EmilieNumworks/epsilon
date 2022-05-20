#include "two_proportions_z_test.h"

namespace Inference {

double TwoProportionsZTest::estimateValue(int index) {
  switch (static_cast<EstimatesOrder>(index)) {
  case EstimatesOrder::P1:
    return TwoProportions::X1(parametersArray()) / TwoProportions::N1(parametersArray());
  case EstimatesOrder::P2:
    return TwoProportions::X2(parametersArray()) / TwoProportions::N2(parametersArray());
  case EstimatesOrder::Pooled:
    return (TwoProportions::X1(parametersArray())+TwoProportions::X2(parametersArray())) / (TwoProportions::N1(parametersArray())+TwoProportions::N2(parametersArray()));
  }
}

Poincare::Layout TwoProportionsZTest::estimateLayout(int index) const {
  // contains the layout p1-p2 in which we pick p1, p2 and p
  Poincare::Layout layout = TwoProportions::EstimateLayout(&m_p1p2Layout);
  switch (static_cast<EstimatesOrder>(index)) {
  case EstimatesOrder::P1:
    return layout.childAtIndex(0); // p̂1
  case EstimatesOrder::P2:
    if (m_p2Layout.isUninitialized()) {
      m_p2Layout = layout.childAtIndex(2).clone(); // p̂2
    }
    return m_p2Layout;
  case EstimatesOrder::Pooled:
    return layout.childAtIndex(0).childAtIndex(0); // p̂1 -> p̂
  }
}

I18n::Message TwoProportionsZTest::estimateDescription(int index) {
  switch (static_cast<EstimatesOrder>(index)) {
  case EstimatesOrder::P1:
    return TwoProportions::Sample1ProportionDescription();
  case EstimatesOrder::P2:
    return TwoProportions::Sample2ProportionDescription();
  case EstimatesOrder::Pooled:
    return TwoProportions::PooledProportionDescription();
  }
}

}
