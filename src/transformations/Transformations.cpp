// =============================================================================
// transformations/Transformations.cpp
// Complete 2D geometric transformations using 3×3 homogeneous matrices.
// Supports: Translation, Rotation, Scaling, Reflection, Shearing
// Chaining: matrices are multiplied so any sequence can be combined.
// =============================================================================

#include "../include/CGApp.h"
#include "Transformations.h"
#include <cmath>

namespace Transforms {

// ─── 3×3 matrix (row-major) ───────────────────────────────────────────────────
//  [ m[0] m[1] m[2] ]   [ a b c ]
//  [ m[3] m[4] m[5] ] = [ d e f ]
//  [ m[6] m[7] m[8] ]   [ g h i ]

using Mat3 = std::array<float, 9>;

static Mat3 identity()
{
    return { 1,0,0,
             0,1,0,
             0,0,1 };
}

// Matrix × Matrix
static Mat3 mul(const Mat3& A, const Mat3& B)
{
    Mat3 C{};
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            for (int k = 0; k < 3; ++k)
                C[r*3+c] += A[r*3+k] * B[k*3+c];
    return C;
}

// Matrix × Point (homogeneous, w = 1)
static Point2D mulPt(const Mat3& M, const Point2D& p)
{
    float x = M[0]*p.x + M[1]*p.y + M[2];
    float y = M[3]*p.x + M[4]*p.y + M[5];
    float w = M[6]*p.x + M[7]*p.y + M[8];
    return { x/w, y/w };
}

// ─── Build individual matrices ────────────────────────────────────────────────

// Translation matrix  T(tx, ty)
//  [ 1  0  tx ]
//  [ 0  1  ty ]
//  [ 0  0   1 ]
static Mat3 translationMatrix(float tx, float ty)
{
    return { 1,0,tx,
             0,1,ty,
             0,0, 1 };
}

// Rotation matrix  R(theta)  — angle in degrees
//  [ cos θ  -sin θ  0 ]
//  [ sin θ   cos θ  0 ]
//  [   0       0    1 ]
static Mat3 rotationMatrix(float deg)
{
    float rad = deg * static_cast<float>(M_PI) / 180.0f;
    float c = std::cos(rad), s = std::sin(rad);
    return { c,-s,0,
             s, c,0,
             0, 0,1 };
}

// Scaling matrix  S(sx, sy)
//  [ sx  0  0 ]
//  [  0 sy  0 ]
//  [  0  0  1 ]
static Mat3 scalingMatrix(float sx, float sy)
{
    return { sx, 0,0,
              0,sy,0,
              0, 0,1 };
}

// Reflection about X-axis: sy = -1
// Reflection about Y-axis: sx = -1
// Reflection about origin: sx = sy = -1
// Reflection about y = x:  swap x and y
static Mat3 reflectionMatrix(ReflectAxis axis)
{
    switch (axis) {
        case ReflectAxis::X:      return scalingMatrix( 1,-1);
        case ReflectAxis::Y:      return scalingMatrix(-1, 1);
        case ReflectAxis::Origin: return scalingMatrix(-1,-1);
        case ReflectAxis::YeqX:   return { 0,1,0, 1,0,0, 0,0,1 };
        case ReflectAxis::YeqNX:  return { 0,-1,0,-1,0,0, 0,0,1 };
    }
    return identity();
}

// Shearing matrix  Sh(shx, shy)
//  [ 1   shx  0 ]
//  [ shy  1   0 ]
//  [ 0    0   1 ]
static Mat3 shearingMatrix(float shx, float shy)
{
    return { 1,shx,0,
             shy,1,0,
             0,  0,1 };
}

// ─── Apply a transformation matrix to a point list ───────────────────────────
static std::vector<Point2D> applyMatrix(const Mat3& M,
                                         const std::vector<Point2D>& pts)
{
    std::vector<Point2D> out;
    out.reserve(pts.size());
    for (const auto& p : pts)
        out.push_back(mulPt(M, p));
    return out;
}

// ─── Rotation about an arbitrary pivot  ──────────────────────────────────────
//   T(-px,-py)  ·  R(theta)  ·  T(px,py)
static Mat3 rotateAboutPivot(float deg, float px, float py)
{
    Mat3 T1  = translationMatrix(-px, -py);
    Mat3 R   = rotationMatrix(deg);
    Mat3 T2  = translationMatrix( px,  py);
    return mul(T2, mul(R, T1));
}

// =============================================================================
// Public API
// =============================================================================

std::vector<Point2D> translate(const std::vector<Point2D>& pts,
                                float tx, float ty)
{
    return applyMatrix(translationMatrix(tx, ty), pts);
}

std::vector<Point2D> rotate(const std::vector<Point2D>& pts,
                             float deg, float px, float py)
{
    return applyMatrix(rotateAboutPivot(deg, px, py), pts);
}

std::vector<Point2D> scale(const std::vector<Point2D>& pts,
                            float sx, float sy, float px, float py)
{
    // Scale about pivot: T(-px,-py) · S · T(px,py)
    Mat3 T1 = translationMatrix(-px, -py);
    Mat3 S  = scalingMatrix(sx, sy);
    Mat3 T2 = translationMatrix( px,  py);
    return applyMatrix(mul(T2, mul(S, T1)), pts);
}

std::vector<Point2D> reflect(const std::vector<Point2D>& pts,
                              ReflectAxis axis)
{
    return applyMatrix(reflectionMatrix(axis), pts);
}

std::vector<Point2D> shear(const std::vector<Point2D>& pts,
                            float shx, float shy)
{
    return applyMatrix(shearingMatrix(shx, shy), pts);
}

// Chain multiple transforms defined as lambda sequence
std::vector<Point2D> chain(const std::vector<Point2D>& pts,
                            const std::vector<std::function<Mat3()>>& ops)
{
    Mat3 M = identity();
    for (auto& op : ops)
        M = mul(op(), M);     // right-to-left composition
    return applyMatrix(M, pts);
}

// Compute centroid (for pivot default)
Point2D centroid(const std::vector<Point2D>& pts)
{
    Point2D c{0,0};
    for (const auto& p : pts) { c.x += p.x; c.y += p.y; }
    float n = static_cast<float>(pts.size());
    return { c.x/n, c.y/n };
}

} // namespace Transforms
