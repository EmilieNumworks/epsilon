#include "complex_graph_cell.h"

using namespace Shared;
using namespace Poincare;

namespace Calculation {

ComplexGraphView::ComplexGraphView(ComplexModel * complexModel) :
  CurveView(complexModel),
  m_complex(complexModel)
{
}

void ComplexGraphView::drawRect(KDContext * ctx, KDRect rect) const {
  ctx->fillRect(rect, KDColorWhite);
  drawGrid(ctx, rect);
  drawAxes(ctx, rect);
  // Draw graduations
  drawLabelsAndGraduations(ctx, rect, Axis::Vertical, true);
  drawLabelsAndGraduations(ctx, rect, Axis::Horizontal, true);

  float real = m_complex->real();
  float imag = m_complex->imag();
  float ph = std::arg(*m_complex);
  /* Draw the segment from the origin to the dot (real, imag) of equation
   * x(t) = t*real and y(t) = t*imag with t in [0,1] */
  drawCurve(ctx, rect, 0.0f, 1.0f, 0.01f,
      [](float t, void * model, void * context) {
        ComplexModel * complexModel = (ComplexModel *)model;
        return Poincare::Coordinate2D<float>(complexModel->real()*t, complexModel->imag()*t);
      }, m_complex, nullptr, false, Palette::GreyDark, false);
  /* Draw the partial ellipse indicating the angle theta
   * - the ellipse parameters are a = |real|/5 and b = |imag|/5,
   * - the parametric ellipse equation is x(t) = a*cos(θ*t) and y(t) = b*sin(θ*t)
   * - we draw the ellipse for t in [0,1] to represent it from the abscissa axis
   * to the phase of the complex
   */
  drawCurve(ctx, rect, 0.0f, 1.0f, 0.01f,
      [](float t, void * model, void * context) {
        ComplexModel * complexModel = (ComplexModel *)model;
        float ph = *(float *)context;
        float factor = 5.0f;
        float a = std::fabs(complexModel->real())/factor;
        float b = std::fabs(complexModel->imag())/factor;
        return Poincare::Coordinate2D<float>(a*std::cos(t*ph), b*std::sin(t*ph));
    }, m_complex, &ph, false, Palette::GreyDark, false);
  // Draw dashed segment to indicate real and imaginary
  drawSegment(ctx, rect, Axis::Vertical, real, 0.0f, imag, Palette::Red, 1, 3);
  drawSegment(ctx, rect, Axis::Horizontal, imag, 0.0f, real, Palette::Red, 1, 3);
  // Draw complex position on the plan
  drawDot(ctx, rect, real, imag, Palette::Red, true);
  // Draw labels
  // 're(z)' label
  drawLabel(ctx, rect, real, 0.0f, "re(z)", Palette::Red, CurveView::RelativePosition::None, imag >= 0.0f ? CurveView::RelativePosition::Before : CurveView::RelativePosition::After);
  // 'im(z)' label
  drawLabel(ctx, rect, 0.0f, imag, "im(θ)", Palette::Red, real >= 0.0f ? CurveView::RelativePosition::Before : CurveView::RelativePosition::After, CurveView::RelativePosition::None);
  // '|z|' label, the relative horizontal position of this label depends on the quadrant
  CurveView::RelativePosition verticalPosition = imag < 0.0f && real >= 0.0f ? CurveView::RelativePosition::Before : CurveView::RelativePosition::After;
  drawLabel(ctx, rect, real/2.0f, imag/2.0f, "|z|", Palette::Red, CurveView::RelativePosition::None, verticalPosition);
  // 'arg(z)' label, the absolute and relative horizontal/vertical positions of this label depends on the quadrant
  /* factor is the ratio of the angle where we position the label
   * For the right half plan, we position the label close to the abscissa axis
   * and for the left half plan, we position the label at the half angle. The
   * relative position is chosen accordingly. */
  float factor = real >= 0.0f ? 0.0f : 0.5f;
  CurveView::RelativePosition horizontalPosition = real >= 0.0f ? CurveView::RelativePosition::After : CurveView::RelativePosition::None;
  verticalPosition = imag >= 0.0f ? CurveView::RelativePosition::After : CurveView::RelativePosition::Before;
  drawLabel(ctx, rect, std::fabs(real)/5.0f*std::cos(factor*ph), std::fabs(imag)/5.0f*std::sin(factor*ph), "arg(z)", Palette::Red, horizontalPosition, verticalPosition);
}

}
