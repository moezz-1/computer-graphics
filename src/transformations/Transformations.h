#pragma once
// =============================================================================
// transformations/Transformations.h
// =============================================================================
#include "../include/CGApp.h"
#include <functional>
#include <array>

namespace Transforms {

using Mat3 = std::array<float, 9>;

enum class ReflectAxis { X, Y, Origin, YeqX, YeqNX };

std::vector<Point2D> translate(const std::vector<Point2D>& pts,
                                float tx, float ty);

std::vector<Point2D> rotate   (const std::vector<Point2D>& pts,
                                float deg, float px=0, float py=0);

std::vector<Point2D> scale    (const std::vector<Point2D>& pts,
                                float sx, float sy, float px=0, float py=0);

std::vector<Point2D> reflect  (const std::vector<Point2D>& pts,
                                ReflectAxis axis);

std::vector<Point2D> shear    (const std::vector<Point2D>& pts,
                                float shx, float shy);

std::vector<Point2D> chain    (const std::vector<Point2D>& pts,
                                const std::vector<std::function<Mat3()>>& ops);

Point2D centroid(const std::vector<Point2D>& pts);

} // namespace Transforms
