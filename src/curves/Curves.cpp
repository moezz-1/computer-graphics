// =============================================================================
// curves/Curves.cpp
// Bezier Curves (de Casteljau) and B-Spline Curves (Cox–de Boor)
// =============================================================================

#include "../include/CGApp.h"
#include "Curves.h"
#include <cmath>

namespace Curves {

// =============================================================================
// Bezier Curve — de Casteljau Algorithm
// -----------------------------------------------------------------------------
// Theory:
//   Given n+1 control points P0…Pn, the curve is defined by:
//     B(t) = Σ_i C(n,i) · (1-t)^(n-i) · t^i · Pi   for t ∈ [0,1]
//
//   de Casteljau evaluates B(t) by successive linear interpolations:
//     P_i^0 = P_i
//     P_i^r = (1-t)·P_i^(r-1) + t·P_{i+1}^(r-1)
//     B(t)  = P_0^n
//
//   This is numerically stable and allows subdivision at any t.
//
// Control point dragging is supported via closestControlPoint().
// =============================================================================

Point2D deCasteljau(const std::vector<Point2D>& pts, float t)
{
    std::vector<Point2D> q = pts;
    int n = static_cast<int>(q.size());
    for (int r = 1; r < n; ++r)
        for (int i = 0; i < n - r; ++i)
            q[i] = q[i] * (1.0f - t) + q[i+1] * t;
    return q[0];
}

std::vector<Point2D> bezierCurve(const std::vector<Point2D>& ctrl,
                                   int segments)
{
    std::vector<Point2D> curve;
    if (ctrl.size() < 2) return curve;
    curve.reserve(segments + 1);
    for (int i = 0; i <= segments; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(segments);
        curve.push_back(deCasteljau(ctrl, t));
    }
    return curve;
}

// Return intermediate polygon at step r of de Casteljau for t
std::vector<std::vector<Point2D>> deCasteljauLevels(
    const std::vector<Point2D>& ctrl, float t)
{
    std::vector<std::vector<Point2D>> levels;
    levels.push_back(ctrl);
    std::vector<Point2D> q = ctrl;
    int n = static_cast<int>(q.size());
    for (int r = 1; r < n; ++r) {
        std::vector<Point2D> next;
        for (int i = 0; i < n - r; ++i)
            next.push_back(q[i] * (1.0f - t) + q[i+1] * t);
        q = next;
        levels.push_back(q);
    }
    return levels;
}

// =============================================================================
// B-Spline Curve — Cox–de Boor Algorithm
// -----------------------------------------------------------------------------
// Theory:
//   B-spline of degree d with n+1 control points P0…Pn and knot vector T:
//     C(t) = Σ_i P_i · N_{i,d}(t)
//
//   Cox–de Boor recurrence:
//     N_{i,0}(t) = 1  if t_i <= t < t_{i+1}, else 0
//     N_{i,d}(t) = (t - t_i)/(t_{i+d} - t_i) · N_{i,d-1}(t)
//                + (t_{i+d+1} - t)/(t_{i+d+1} - t_{i+1}) · N_{i+1,d-1}(t)
//
//   Clamped uniform knot vector: d+1 zeros, n-d+1 equally spaced values, d+1 ones.
//   This ensures the curve passes through the first and last control points.
// =============================================================================

static std::vector<float> clampedKnots(int n, int d)
{
    // Total knots = n + d + 2
    int m = n + d + 2;
    std::vector<float> T(m, 0.0f);
    // First d+1 knots = 0, last d+1 knots = 1
    for (int i = 0; i <= d; ++i)         T[i]         = 0.0f;
    for (int i = m-1-d; i < m; ++i)      T[i]         = 1.0f;
    int inner = n - d;
    for (int i = 1; i <= inner; ++i)
        T[d+i] = static_cast<float>(i) / static_cast<float>(inner+1);
    return T;
}

static float coxDeBoor(const std::vector<float>& T, int i, int d, float t)
{
    if (d == 0)
        return (T[i] <= t && t < T[i+1]) ? 1.0f : 0.0f;

    float left  = 0, right = 0;
    float denom;

    denom = T[i+d] - T[i];
    if (denom > 1e-6f)
        left = (t - T[i]) / denom * coxDeBoor(T, i, d-1, t);

    denom = T[i+d+1] - T[i+1];
    if (denom > 1e-6f)
        right = (T[i+d+1] - t) / denom * coxDeBoor(T, i+1, d-1, t);

    return left + right;
}

Point2D bsplinePoint(const std::vector<Point2D>& ctrl,
                      const std::vector<float>& knots, int degree, float t)
{
    int n = static_cast<int>(ctrl.size()) - 1;
    Point2D pt{0,0};
    for (int i = 0; i <= n; ++i) {
        float b = coxDeBoor(knots, i, degree, t);
        pt.x += b * ctrl[i].x;
        pt.y += b * ctrl[i].y;
    }
    return pt;
}

std::vector<Point2D> bsplineCurve(const std::vector<Point2D>& ctrl,
                                    int degree, int segments)
{
    std::vector<Point2D> curve;
    int n = static_cast<int>(ctrl.size()) - 1;
    if (n < degree) return curve;

    std::vector<float> knots = clampedKnots(n, degree);
    curve.reserve(segments + 1);

    // Use t ∈ [0, 1-ε] to stay within knot span; evaluate at t=1 separately
    for (int i = 0; i < segments; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(segments);
        curve.push_back(bsplinePoint(ctrl, knots, degree, t));
    }
    // Manually add last control point for clamped knots
    curve.push_back(ctrl.back());

    return curve;
}

// Find control point closest to (mx, my) within threshold
int closestControlPoint(const std::vector<Point2D>& pts, float mx, float my,
                         float threshold)
{
    int   best  = -1;
    float bestD = threshold * threshold;
    for (int i = 0; i < (int)pts.size(); ++i) {
        float dx = pts[i].x - mx;
        float dy = pts[i].y - my;
        float d2 = dx*dx + dy*dy;
        if (d2 < bestD) { bestD = d2; best = i; }
    }
    return best;
}

} // namespace Curves
