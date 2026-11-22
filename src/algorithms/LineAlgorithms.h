#pragma once
// =============================================================================
// algorithms/LineAlgorithms.h
// =============================================================================
#include "../include/CGApp.h"

namespace LineAlgorithms {

// Result metadata returned after drawing
struct DrawResult {
    std::string algoName;
    double      timeMs;
    int         pixelCount;
};

// Core drawing functions — fill 'pixels' vector, return performance stats
DrawResult DDA(float x0, float y0, float x1, float y1,
               const Colour& col, std::vector<DrawnPixel>& pixels);

DrawResult Bresenham(float x0, float y0, float x1, float y1,
                     const Colour& col, std::vector<DrawnPixel>& pixels);

// Render a pixel list via OpenGL GL_POINTS
void renderPixels(const std::vector<DrawnPixel>& pixels, float ptSize = 1.0f);

} // namespace LineAlgorithms
