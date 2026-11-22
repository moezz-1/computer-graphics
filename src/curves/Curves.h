#pragma once
// =============================================================================
// curves/Curves.h
// =============================================================================
#include "../include/CGApp.h"

namespace Curves {

// Bezier
Point2D                         deCasteljau(const std::vector<Point2D>& pts, float t);
std::vector<Point2D>            bezierCurve(const std::vector<Point2D>& ctrl, int segments=200);
std::vector<std::vector<Point2D>> deCasteljauLevels(const std::vector<Point2D>& ctrl, float t);

// B-Spline
Point2D              bsplinePoint(const std::vector<Point2D>& ctrl,
                                   const std::vector<float>& knots, int degree, float t);
std::vector<Point2D> bsplineCurve(const std::vector<Point2D>& ctrl,
                                   int degree=3, int segments=200);

// Utility
int closestControlPoint(const std::vector<Point2D>& pts, float mx, float my,
                         float threshold=12.0f);

} // namespace Curves
