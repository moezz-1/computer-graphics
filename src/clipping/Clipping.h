#pragma once
// =============================================================================
// clipping/Clipping.h
// =============================================================================
#include "../include/CGApp.h"

namespace Clipping {

// Cohen–Sutherland result
struct CSResult {
    bool  accepted;
    float x0, y0, x1, y1;   // clipped endpoints (if accepted)
    int   code0, code1;      // outcode bits (for visualisation)
};

CSResult CohenSutherland(float x0, float y0, float x1, float y1,
                          const ClipRect& w);

// Sutherland–Hodgman intermediate stage
struct SHStage {
    std::string          name;
    std::vector<Point2D> polygon;
};

std::vector<Point2D> SutherlandHodgman(const std::vector<Point2D>& polygon,
                                        const ClipRect& w,
                                        std::vector<SHStage>& stages);

} // namespace Clipping
