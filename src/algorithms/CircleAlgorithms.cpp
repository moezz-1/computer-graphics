// =============================================================================
// algorithms/CircleAlgorithms.cpp
// Midpoint Circle Algorithm and Bresenham Circle Algorithm from scratch.
// Both exploit 8-way symmetry of a circle to compute only 1/8 of the perimeter.
// =============================================================================

#include "../include/CGApp.h"
#include "CircleAlgorithms.h"
#include <chrono>

namespace CircleAlgorithms {

// ─── Helper: plot all 8 symmetric points ─────────────────────────────────────
static void plot8(int cx, int cy, int x, int y,
                  const Colour& c, std::vector<DrawnPixel>& out)
{
    // 8-way symmetry: (±x, ±y) and (±y, ±x)
    auto p = [&](int px, int py){ out.push_back({cx+px, cy+py, c}); };
    p( x, y); p(-x, y); p( x,-y); p(-x,-y);
    p( y, x); p(-y, x); p( y,-x); p(-y,-x);
}

// =============================================================================
// Midpoint Circle Algorithm
// -----------------------------------------------------------------------------
// Theory (derivation from implicit equation f(x,y) = x²+y²-r²):
//   Start at (0, r).  Decision variable measures which candidate pixel
//   (E or SE) is closer to the true circle:
//
//     p_initial = 1 - r
//     if p < 0:  choose E,  p_next = p + 2*x + 3
//     else:      choose SE, p_next = p + 2*(x-y) + 5;  y--
//
//   x is incremented every step.  Loop until x >= y (octant boundary).
//
// Time complexity: O(r)
// =============================================================================
DrawResult Midpoint(int cx, int cy, int r,
                    const Colour& col, std::vector<DrawnPixel>& pixels)
{
    auto t0 = std::chrono::high_resolution_clock::now();

    int x = 0, y = r;
    int p = 1 - r;          // initial decision parameter
    int count = 0;

    plot8(cx, cy, x, y, col, pixels);
    count += 8;

    while (x < y) {
        ++x;
        if (p < 0) {
            p += 2 * x + 1;
        } else {
            --y;
            p += 2 * (x - y) + 1;
        }
        plot8(cx, cy, x, y, col, pixels);
        count += 8;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    return { "Midpoint Circle", ms, count };
}

// =============================================================================
// Bresenham Circle Algorithm
// -----------------------------------------------------------------------------
// Theory:
//   Very similar to Midpoint but the decision parameter is initialised as
//   p = 3 - 2r and updated with purely integer arithmetic:
//
//     p_initial = 3 - 2r
//     if p <= 0: p_next = p + 4*x + 6
//     else:      p_next = p + 4*(x-y) + 10;  y--
//
//   This formulation avoids the "1 - r" fractional origin of the Midpoint
//   method for very small radii.
//
// Time complexity: O(r)
// =============================================================================
DrawResult Bresenham(int cx, int cy, int r,
                     const Colour& col, std::vector<DrawnPixel>& pixels)
{
    auto t0 = std::chrono::high_resolution_clock::now();

    int x = 0, y = r;
    int p = 3 - 2 * r;      // initial decision parameter (Bresenham variant)
    int count = 0;

    plot8(cx, cy, x, y, col, pixels);
    count += 8;

    while (x <= y) {
        ++x;
        if (p <= 0) {
            p += 4 * x + 6;
        } else {
            --y;
            p += 4 * (x - y) + 10;
        }
        plot8(cx, cy, x, y, col, pixels);
        count += 8;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    return { "Bresenham Circle", ms, count };
}

} // namespace CircleAlgorithms
