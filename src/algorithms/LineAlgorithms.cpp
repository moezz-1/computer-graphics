// =============================================================================
// algorithms/LineAlgorithms.cpp
// Implements DDA and Bresenham Line Drawing Algorithms from scratch.
// No built-in line drawing functions are used; every pixel is computed manually.
// =============================================================================

#include "../include/CGApp.h"
#include "LineAlgorithms.h"
#include <cmath>
#include <chrono>

namespace LineAlgorithms {

// ─── Helper: record a single pixel ───────────────────────────────────────────
static void emitPixel(int x, int y, const Colour& c,
                      std::vector<DrawnPixel>& out)
{
    out.push_back({x, y, c});
}

// ─── Helper: draw collected pixels via OpenGL points ─────────────────────────
void renderPixels(const std::vector<DrawnPixel>& pixels, float ptSize)
{
    glPointSize(ptSize);
    glBegin(GL_POINTS);
    for (const auto& p : pixels) {
        glColor4f(p.col.r, p.col.g, p.col.b, p.col.a);
        glVertex2f(static_cast<float>(p.x), static_cast<float>(p.y));
    }
    glEnd();
}

// =============================================================================
// DDA (Digital Differential Analyzer) Line Algorithm
// -----------------------------------------------------------------------------
// Theory:
//   Given endpoints (x0,y0) and (x1,y1):
//     dx = x1 - x0,  dy = y1 - y0
//     steps = max(|dx|, |dy|)          (number of pixels to plot)
//     xInc  = dx / steps               (x increment per step)
//     yInc  = dy / steps               (y increment per step)
//   Starting at (x0,y0), we add xInc/yInc each step and round to nearest int.
//
// Advantages : Simple, intuitive.
// Disadvantages: Uses floating-point division, slower than Bresenham.
// Time complexity: O(max(|dx|, |dy|))
// =============================================================================
DrawResult DDA(float x0, float y0, float x1, float y1,
               const Colour& col, std::vector<DrawnPixel>& pixels)
{
    auto t0 = std::chrono::high_resolution_clock::now();

    float dx    = x1 - x0;
    float dy    = y1 - y0;
    float steps = std::max(std::abs(dx), std::abs(dy));

    float xInc  = (steps == 0) ? 0 : dx / steps;
    float yInc  = (steps == 0) ? 0 : dy / steps;

    float x = x0, y = y0;
    int   count = 0;

    for (int i = 0; i <= static_cast<int>(steps); ++i) {
        emitPixel(static_cast<int>(std::round(x)),
                  static_cast<int>(std::round(y)), col, pixels);
        x += xInc;
        y += yInc;
        ++count;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    return { "DDA", ms, count };
}

// =============================================================================
// Bresenham Line Algorithm
// -----------------------------------------------------------------------------
// Theory:
//   Uses only integer arithmetic.  At each step the algorithm decides whether
//   the next pixel is to the right (E) or diagonally (NE/SE) by maintaining
//   a decision variable  p:
//
//     p_initial = 2*|dy| - |dx|
//     if p < 0:   plot (x+1, y),       p_next = p + 2*|dy|
//     else:       plot (x+1, y+ystep), p_next = p + 2*|dy| - 2*|dx|
//
//   The case where |dy| > |dx| is handled by swapping x/y roles.
//
// Advantages : Integer-only, very fast, no rounding error.
// Time complexity: O(max(|dx|, |dy|))
// =============================================================================
DrawResult Bresenham(float x0f, float y0f, float x1f, float y1f,
                     const Colour& col, std::vector<DrawnPixel>& pixels)
{
    auto t0 = std::chrono::high_resolution_clock::now();

    int x0 = static_cast<int>(std::round(x0f));
    int y0 = static_cast<int>(std::round(y0f));
    int x1 = static_cast<int>(std::round(x1f));
    int y1 = static_cast<int>(std::round(y1f));

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;   // step direction in x
    int sy = (y0 < y1) ? 1 : -1;   // step direction in y

    int err = dx - dy;              // initial decision variable
    int count = 0;

    while (true) {
        emitPixel(x0, y0, col, pixels);
        ++count;

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;

        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    return { "Bresenham", ms, count };
}

} // namespace LineAlgorithms
