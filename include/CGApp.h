#pragma once
// =============================================================================
// CGApp.h — Master header for Computer Graphics Application
// Prof. Dr. Hassan A. Youness Alansary — Computer Graphics Project
// =============================================================================
// Declares all subsystems, data structures, and global state used throughout
// the application.  Every .cpp file should include this header.
// =============================================================================

#ifdef _WIN32
  #include <windows.h>
#endif

#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <string>
#include <functional>
#include <chrono>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <stack>
#include <iomanip>

// ─── Mathematical constants ───────────────────────────────────────────────────
#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

// ─── Window dimensions ───────────────────────────────────────────────────────
constexpr int WINDOW_W = 1280;
constexpr int WINDOW_H = 720;

// ─── Colour structure ─────────────────────────────────────────────────────────
struct Colour {
    float r, g, b, a;
    Colour(float r=1,float g=1,float b=1,float a=1):r(r),g(g),b(b),a(a){}
    static Colour White()   { return {1,1,1,1}; }
    static Colour Black()   { return {0,0,0,1}; }
    static Colour Red()     { return {0.93f,0.29f,0.29f,1}; }
    static Colour Green()   { return {0.39f,0.60f,0.13f,1}; }
    static Colour Blue()    { return {0.22f,0.54f,0.87f,1}; }
    static Colour Yellow()  { return {0.95f,0.78f,0.15f,1}; }
    static Colour Cyan()    { return {0.11f,0.62f,0.65f,1}; }
    static Colour Orange()  { return {0.84f,0.37f,0.11f,1}; }
    static Colour Gray()    { return {0.53f,0.53f,0.53f,1}; }
    static Colour Purple()  { return {0.47f,0.34f,0.85f,1}; }
};

// ─── 2D Point ─────────────────────────────────────────────────────────────────
struct Point2D {
    float x, y;
    Point2D(float x=0,float y=0):x(x),y(y){}
    Point2D operator+(const Point2D& o) const { return {x+o.x, y+o.y}; }
    Point2D operator-(const Point2D& o) const { return {x-o.x, y-o.y}; }
    Point2D operator*(float s)          const { return {x*s,   y*s  }; }
    float   dot(const Point2D& o)       const { return x*o.x + y*o.y; }
    float   length()                    const { return std::sqrt(x*x+y*y); }
};

// ─── Shape types ─────────────────────────────────────────────────────────────
enum class ShapeType { None, Line, Circle, Triangle, Rectangle, Polygon };

// ─── Algorithm identifiers ───────────────────────────────────────────────────
enum class LineAlgo   { DDA, Bresenham };
enum class CircleAlgo { Midpoint, Bresenham };

// ─── A drawn pixel (for step-by-step visualisation) ──────────────────────────
struct DrawnPixel {
    int x, y;
    Colour col;
};

// ─── Generic shape record ─────────────────────────────────────────────────────
struct Shape {
    ShapeType type      = ShapeType::None;
    std::vector<Point2D> pts;          // control/vertex points
    Colour    colour    = Colour::White();
    LineAlgo  lineAlgo  = LineAlgo::Bresenham;
    CircleAlgo circAlgo = CircleAlgo::Midpoint;
    float     radius    = 0;
    bool      selected  = false;
};

// ─── Clipping window ─────────────────────────────────────────────────────────
struct ClipRect {
    float xMin, yMin, xMax, yMax;
    ClipRect(float x1=-100,float y1=-100,float x2=100,float y2=100)
        :xMin(x1),yMin(y1),xMax(x2),yMax(y2){}
};

// ─── Transformation record ────────────────────────────────────────────────────
struct TransformRecord {
    std::string description;
    std::vector<Point2D> before, after;
};

// ─── Performance sample ───────────────────────────────────────────────────────
struct PerfSample {
    std::string label;
    double      timeMs;
    int         pixelCount;
};

// ─── Application mode ─────────────────────────────────────────────────────────
enum class AppMode {
    Drawing,
    Transformations,
    Clipping,
    Curves,
    Performance
};

// =============================================================================
// Global application state  (defined in main.cpp)
// =============================================================================
namespace AppState {
    extern AppMode           mode;
    extern std::vector<Shape> shapes;
    extern Shape              currentShape;
    extern bool               drawing;

    // Canvas / view
    extern float   panX, panY, zoom;
    extern bool    showGrid, darkMode, showFPS;
    extern int     frameCount;
    extern double  fps;

    // Undo / redo
    extern std::stack<std::vector<Shape>> undoStack;
    extern std::stack<std::vector<Shape>> redoStack;

    // Selected colour for new shapes
    extern Colour  drawColour;

    // Active algorithms
    extern LineAlgo   activeLineAlgo;
    extern CircleAlgo activeCircAlgo;

    // Clipping
    extern ClipRect   clipWindow;
    extern bool       showClipWindow;

    // Curve control points
    extern std::vector<Point2D> curvePoints;
    extern int                  bsplineDegree;

    // Step-by-step
    extern bool              stepMode;
    extern int               stepIndex;
    extern std::vector<DrawnPixel> stepPixels;

    // Performance history
    extern std::vector<PerfSample> perfHistory;

    // Mouse
    extern float   mouseX, mouseY;
    extern bool    mouseDown;

    // Transformation history
    extern std::vector<TransformRecord> transformHistory;

    // Status message
    extern std::string statusMsg;
}
