// =============================================================================
// clipping/Clipping.cpp
// Cohen–Sutherland Line Clipping and Sutherland–Hodgman Polygon Clipping
// =============================================================================

#include "../include/CGApp.h"
#include "Clipping.h"

namespace Clipping {

// =============================================================================
// Cohen–Sutherland Line Clipping Algorithm
// -----------------------------------------------------------------------------
// Theory:
//   Divides 2D space into 9 regions using a 4-bit outcode per endpoint:
//       bit 3 (TOP)    = above yMax
//       bit 2 (BOTTOM) = below yMin
//       bit 1 (RIGHT)  = right of xMax
//       bit 0 (LEFT)   = left of xMin
//
//   Loop:
//     1. Compute outcodes for both endpoints.
//     2. If both codes = 0000 → trivially accept.
//     3. If (code0 AND code1) ≠ 0 → trivially reject (both outside same side).
//     4. Otherwise clip: pick an endpoint outside the window, compute
//        intersection with the boundary crossing its set bit, replace endpoint.
//
//   Intersection formulae (parametric form):
//     x = x0 + t*(x1-x0),   y = y0 + t*(y1-y0)
//     For LEFT:   t = (xMin - x0) / (x1 - x0)
//     For RIGHT:  t = (xMax - x0) / (x1 - x0)
//     For BOTTOM: t = (yMin - y0) / (y1 - y0)
//     For TOP:    t = (yMax - y0) / (y1 - y0)
// =============================================================================

static const int CS_LEFT   = 1;   // 0001
static const int CS_RIGHT  = 2;   // 0010
static const int CS_BOTTOM = 4;   // 0100
static const int CS_TOP    = 8;   // 1000

static int computeOutcode(float x, float y, const ClipRect& w)
{
    int code = 0;
    if      (x < w.xMin) code |= CS_LEFT;
    else if (x > w.xMax) code |= CS_RIGHT;
    if      (y < w.yMin) code |= CS_BOTTOM;
    else if (y > w.yMax) code |= CS_TOP;
    return code;
}

CSResult CohenSutherland(float x0, float y0, float x1, float y1,
                          const ClipRect& w)
{
    CSResult res;
    res.accepted = false;
    res.x0 = x0; res.y0 = y0;
    res.x1 = x1; res.y1 = y1;

    int c0 = computeOutcode(x0, y0, w);
    int c1 = computeOutcode(x1, y1, w);
    res.code0 = c0;
    res.code1 = c1;

    while (true) {
        if (!(c0 | c1)) {           // trivially accept
            res.accepted = true;
            res.x0 = x0; res.y0 = y0;
            res.x1 = x1; res.y1 = y1;
            break;
        } else if (c0 & c1) {      // trivially reject
            break;
        } else {
            // Choose the outside endpoint
            int cOut = c0 ? c0 : c1;
            float x, y;

            if (cOut & CS_TOP) {
                x = x0 + (x1-x0) * (w.yMax - y0) / (y1 - y0);
                y = w.yMax;
            } else if (cOut & CS_BOTTOM) {
                x = x0 + (x1-x0) * (w.yMin - y0) / (y1 - y0);
                y = w.yMin;
            } else if (cOut & CS_RIGHT) {
                y = y0 + (y1-y0) * (w.xMax - x0) / (x1 - x0);
                x = w.xMax;
            } else { // LEFT
                y = y0 + (y1-y0) * (w.xMin - x0) / (x1 - x0);
                x = w.xMin;
            }

            if (cOut == c0) {
                x0 = x; y0 = y;
                c0 = computeOutcode(x0, y0, w);
            } else {
                x1 = x; y1 = y;
                c1 = computeOutcode(x1, y1, w);
            }
        }
    }
    return res;
}

// =============================================================================
// Sutherland–Hodgman Polygon Clipping Algorithm
// -----------------------------------------------------------------------------
// Theory:
//   Clips a polygon against each of the four half-planes of the clip window
//   one at a time.  For each clip edge:
//
//   For each edge (s → p) of the subject polygon:
//     case 1: both inside  → output p
//     case 2: s inside, p outside → output intersection
//     case 3: both outside → output nothing
//     case 4: s outside, p inside → output intersection then p
//
//   Intersection with a clip edge (horizontal/vertical line):
//     Parametric: I = s + t*(p-s),  t solved for the boundary x or y value.
//
//   Can clip against arbitrary convex windows by changing the "inside" test.
// =============================================================================

// Clip polygon against a single vertical/horizontal edge
// axis: 0 = left (x >= xMin), 1 = right (x <= xMax),
//       2 = bottom (y >= yMin), 3 = top (y <= yMax)
static bool insideHalfPlane(const Point2D& p, int axis, const ClipRect& w)
{
    switch (axis) {
        case 0: return p.x >= w.xMin;
        case 1: return p.x <= w.xMax;
        case 2: return p.y >= w.yMin;
        case 3: return p.y <= w.yMax;
    }
    return false;
}

static Point2D clipEdgeIntersect(const Point2D& s, const Point2D& p,
                                  int axis, const ClipRect& w)
{
    float t;
    switch (axis) {
        case 0: t = (w.xMin - s.x) / (p.x - s.x); break;
        case 1: t = (w.xMax - s.x) / (p.x - s.x); break;
        case 2: t = (w.yMin - s.y) / (p.y - s.y); break;
        case 3: t = (w.yMax - s.y) / (p.y - s.y); break;
        default: t = 0;
    }
    return { s.x + t*(p.x-s.x), s.y + t*(p.y-s.y) };
}

static std::vector<Point2D> clipAgainstEdge(const std::vector<Point2D>& poly,
                                              int axis, const ClipRect& w)
{
    std::vector<Point2D> out;
    if (poly.empty()) return out;

    for (size_t i = 0; i < poly.size(); ++i) {
        const Point2D& s = poly[i];
        const Point2D& p = poly[(i+1) % poly.size()];

        bool sIn = insideHalfPlane(s, axis, w);
        bool pIn = insideHalfPlane(p, axis, w);

        if (sIn && pIn) {
            out.push_back(p);                                    // case 1
        } else if (sIn && !pIn) {
            out.push_back(clipEdgeIntersect(s, p, axis, w));    // case 2
        } else if (!sIn && pIn) {
            out.push_back(clipEdgeIntersect(s, p, axis, w));    // case 4
            out.push_back(p);
        }
        // case 3: both outside → output nothing
    }
    return out;
}

std::vector<Point2D> SutherlandHodgman(const std::vector<Point2D>& polygon,
                                        const ClipRect& w,
                                        std::vector<SHStage>& stages)
{
    stages.clear();
    std::vector<Point2D> current = polygon;
    stages.push_back({ "Input", current });

    const char* names[] = {"Left","Right","Bottom","Top"};
    for (int axis = 0; axis < 4; ++axis) {
        current = clipAgainstEdge(current, axis, w);
        stages.push_back({ names[axis], current });
        if (current.empty()) break;
    }
    return current;
}

} // namespace Clipping
