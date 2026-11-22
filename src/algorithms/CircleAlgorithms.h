#pragma once
// =============================================================================
// algorithms/CircleAlgorithms.h
// =============================================================================
#include "../include/CGApp.h"

namespace CircleAlgorithms {

struct DrawResult {
    std::string algoName;
    double      timeMs;
    int         pixelCount;
};

DrawResult Midpoint (int cx, int cy, int r,
                     const Colour& col, std::vector<DrawnPixel>& pixels);

DrawResult Bresenham(int cx, int cy, int r,
                     const Colour& col, std::vector<DrawnPixel>& pixels);

} // namespace CircleAlgorithms
