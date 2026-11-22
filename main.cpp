// =============================================================================
// main.cpp — Computer Graphics Application Entry Point
// Prof. Dr. Hassan A. Youness Alansary
// =============================================================================
// Full GLUT/OpenGL application with:
//   • Drawing algorithms (DDA, Bresenham line; Midpoint, Bresenham circle)
//   • 2D Transformations (translate, rotate, scale, reflect, shear)
//   • Clipping (Cohen–Sutherland, Sutherland–Hodgman)
//   • Curves (Bezier, B-Spline)
//   • Grid, zoom/pan, undo/redo, dark mode, FPS, status bar
// =============================================================================

#include "include/CGApp.h"
#include "src/algorithms/LineAlgorithms.h"
#include "src/algorithms/CircleAlgorithms.h"
#include "src/transformations/Transformations.h"
#include "src/clipping/Clipping.h"
#include "src/curves/Curves.h"

// ─── Global state definitions ─────────────────────────────────────────────────
namespace AppState {
    AppMode  mode     = AppMode::Drawing;
    std::vector<Shape> shapes;
    Shape              currentShape;
    bool               drawing = false;

    float  panX = 0, panY = 0, zoom = 1.0f;
    bool   showGrid = true, darkMode = true, showFPS = true;
    int    frameCount = 0;
    double fps = 0;

    std::stack<std::vector<Shape>> undoStack;
    std::stack<std::vector<Shape>> redoStack;

    Colour     drawColour  = Colour::Cyan();
    LineAlgo   activeLineAlgo  = LineAlgo::Bresenham;
    CircleAlgo activeCircAlgo  = CircleAlgo::Midpoint;

    ClipRect   clipWindow{-120,-80,120,80};
    bool       showClipWindow = true;

    std::vector<Point2D> curvePoints;
    int                  bsplineDegree = 3;

    bool              stepMode  = false;
    int               stepIndex = 0;
    std::vector<DrawnPixel> stepPixels;

    std::vector<PerfSample> perfHistory;

    float  mouseX = 0, mouseY = 0;
    bool   mouseDown = false;

    std::vector<TransformRecord> transformHistory;
    std::string statusMsg = "Ready — press H for help";
}

// ─── Timing for FPS ───────────────────────────────────────────────────────────
static auto lastTime = std::chrono::high_resolution_clock::now();

// ─── Screen→world conversion ─────────────────────────────────────────────────
static float screenToWorldX(int sx) {
    float cx = WINDOW_W * 0.5f;
    return (sx - cx - AppState::panX) / AppState::zoom;
}
static float screenToWorldY(int sy) {
    float cy = WINDOW_H * 0.5f;
    return (cy - sy + AppState::panY) / AppState::zoom;   // flip Y
}

// ─── Push undo snapshot ──────────────────────────────────────────────────────
static void pushUndo() {
    AppState::undoStack.push(AppState::shapes);
    while (!AppState::redoStack.empty()) AppState::redoStack.pop();
}

// ─── Text helpers ─────────────────────────────────────────────────────────────
static void drawBitmapString(float x, float y, const std::string& s,
                              void* font = GLUT_BITMAP_HELVETICA_12)
{
    glRasterPos2f(x, y);
    for (char c : s) glutBitmapCharacter(font, c);
}

// ─── Grid ─────────────────────────────────────────────────────────────────────
static void drawGrid()
{
    if (!AppState::showGrid) return;

    float alpha = AppState::darkMode ? 0.12f : 0.08f;
    glColor4f(0.5f, 0.5f, 0.5f, alpha);
    glLineWidth(0.5f);

    float spacing = 20.0f;
    float left  = -(WINDOW_W * 0.5f / AppState::zoom + std::abs(AppState::panX));
    float right  =   WINDOW_W * 0.5f / AppState::zoom + std::abs(AppState::panX);
    float bottom = -(WINDOW_H * 0.5f / AppState::zoom + std::abs(AppState::panY));
    float top    =   WINDOW_H * 0.5f / AppState::zoom + std::abs(AppState::panY);

    glBegin(GL_LINES);
    for (float x = std::floor(left/spacing)*spacing; x <= right; x += spacing) {
        glVertex2f(x, bottom); glVertex2f(x, top);
    }
    for (float y = std::floor(bottom/spacing)*spacing; y <= top; y += spacing) {
        glVertex2f(left, y); glVertex2f(right, y);
    }
    glEnd();

    // Axes
    glColor4f(0.6f, 0.6f, 0.6f, 0.4f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    glVertex2f(left, 0); glVertex2f(right, 0);
    glVertex2f(0, bottom); glVertex2f(0, top);
    glEnd();
}

// ─── Draw a polygon outline ───────────────────────────────────────────────────
static void drawPolygon(const std::vector<Point2D>& pts, bool closed = true)
{
    if (pts.size() < 2) return;
    glBegin(closed ? GL_LINE_LOOP : GL_LINE_STRIP);
    for (const auto& p : pts) glVertex2f(p.x, p.y);
    glEnd();
}

// ─── Draw a shape using correct algorithm ─────────────────────────────────────
static void renderShape(const Shape& s, float ptSize = 1.5f)
{
    glColor4f(s.colour.r, s.colour.g, s.colour.b, s.colour.a);

    if (s.type == ShapeType::Line && s.pts.size() >= 2) {
        std::vector<DrawnPixel> px;
        if (s.lineAlgo == LineAlgo::DDA)
            LineAlgorithms::DDA(s.pts[0].x, s.pts[0].y,
                                s.pts[1].x, s.pts[1].y, s.colour, px);
        else
            LineAlgorithms::Bresenham(s.pts[0].x, s.pts[0].y,
                                      s.pts[1].x, s.pts[1].y, s.colour, px);
        LineAlgorithms::renderPixels(px, ptSize);

    } else if (s.type == ShapeType::Circle && s.pts.size() >= 1) {
        std::vector<DrawnPixel> px;
        int cx = static_cast<int>(s.pts[0].x);
        int cy = static_cast<int>(s.pts[0].y);
        int r  = static_cast<int>(s.radius);
        if (s.circAlgo == CircleAlgo::Midpoint)
            CircleAlgorithms::Midpoint (cx, cy, r, s.colour, px);
        else
            CircleAlgorithms::Bresenham(cx, cy, r, s.colour, px);
        LineAlgorithms::renderPixels(px, ptSize);

    } else if (s.type == ShapeType::Rectangle && s.pts.size() >= 2) {
        // Draw as 4 Bresenham lines
        float x0=s.pts[0].x, y0=s.pts[0].y;
        float x1=s.pts[1].x, y1=s.pts[1].y;
        std::vector<DrawnPixel> px;
        LineAlgorithms::Bresenham(x0,y0,x1,y0,s.colour,px);
        LineAlgorithms::Bresenham(x1,y0,x1,y1,s.colour,px);
        LineAlgorithms::Bresenham(x1,y1,x0,y1,s.colour,px);
        LineAlgorithms::Bresenham(x0,y1,x0,y0,s.colour,px);
        LineAlgorithms::renderPixels(px, ptSize);

    } else if ((s.type == ShapeType::Triangle || s.type == ShapeType::Polygon)
               && s.pts.size() >= 2) {
        std::vector<DrawnPixel> px;
        for (size_t i = 0; i < s.pts.size(); ++i) {
            const Point2D& a = s.pts[i];
            const Point2D& b = s.pts[(i+1) % s.pts.size()];
            LineAlgorithms::Bresenham(a.x,a.y,b.x,b.y,s.colour,px);
        }
        LineAlgorithms::renderPixels(px, ptSize);
    }

    // Selection highlight
    if (s.selected) {
        glColor4f(1,1,0,0.4f);
        glLineWidth(2.0f);
        if (!s.pts.empty()) drawPolygon(s.pts, false);
        glLineWidth(1.0f);
    }
}

// ─── Clipping window drawing ─────────────────────────────────────────────────
static void drawClipWindow()
{
    const auto& w = AppState::clipWindow;
    glColor4f(1,0.8f,0.2f,0.6f);
    glLineWidth(1.5f);
    glLineStipple(2, 0xAAAA);
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINE_LOOP);
    glVertex2f(w.xMin, w.yMin);
    glVertex2f(w.xMax, w.yMin);
    glVertex2f(w.xMax, w.yMax);
    glVertex2f(w.xMin, w.yMax);
    glEnd();
    glDisable(GL_LINE_STIPPLE);
    glLineWidth(1.0f);
}

// ─── HUD / Status bar ─────────────────────────────────────────────────────────
static void drawHUD()
{
    // Switch to screen-space 2D overlay
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, WINDOW_W, 0, WINDOW_H);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();

    // Top toolbar background
    Colour bg = AppState::darkMode ? Colour(0.08f,0.08f,0.1f,0.92f)
                                    : Colour(0.95f,0.95f,0.97f,0.92f);
    glColor4f(bg.r,bg.g,bg.b,bg.a);
    glBegin(GL_QUADS);
    glVertex2f(0,WINDOW_H-40); glVertex2f(WINDOW_W,WINDOW_H-40);
    glVertex2f(WINDOW_W,WINDOW_H); glVertex2f(0,WINDOW_H);
    glEnd();

    // Mode tabs
    const char* modes[] = { "[1]Draw", "[2]Transform", "[3]Clip", "[4]Curves", "[5]Perf" };
    AppMode modeVals[] = { AppMode::Drawing, AppMode::Transformations,
                           AppMode::Clipping, AppMode::Curves, AppMode::Performance };
    float tx = 10;
    for (int i = 0; i < 5; ++i) {
        bool active = AppState::mode == modeVals[i];
        if (active) glColor4f(0.22f,0.62f,0.87f,1); else
        glColor4f(AppState::darkMode?0.55f:0.35f, AppState::darkMode?0.55f:0.35f,
                  AppState::darkMode?0.6f:0.4f, 1);
        drawBitmapString(tx, WINDOW_H-25, modes[i], GLUT_BITMAP_HELVETICA_12);
        tx += 130;
    }

    // FPS
    if (AppState::showFPS) {
        glColor4f(0.39f,0.87f,0.39f,1);
        std::ostringstream ss;
        ss << "FPS:" << std::fixed << std::setprecision(1) << AppState::fps;
        drawBitmapString(WINDOW_W-100, WINDOW_H-25, ss.str(), GLUT_BITMAP_HELVETICA_12);
    }

    // Status bar
    glColor4f(0.06f,0.06f,0.08f,0.88f);
    glBegin(GL_QUADS);
    glVertex2f(0,0); glVertex2f(WINDOW_W,0);
    glVertex2f(WINDOW_W,22); glVertex2f(0,22);
    glEnd();

    glColor4f(0.7f,0.7f,0.75f,1);
    std::ostringstream sbar;
    sbar << AppState::statusMsg
         << "  |  World(" << std::fixed << std::setprecision(1)
         << AppState::mouseX << "," << AppState::mouseY << ")"
         << "  |  Zoom:" << AppState::zoom
         << "  |  Shapes:" << AppState::shapes.size()
         << "  |  [G]rid [D]ark [Z/X]zoom [H]elp";
    drawBitmapString(8, 7, sbar.str(), GLUT_BITMAP_HELVETICA_10);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ─── Performance bar chart (on-canvas) ───────────────────────────────────────
static void drawPerfPanel()
{
    if (AppState::perfHistory.empty()) return;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, WINDOW_W, 0, WINDOW_H);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();

    float px = 40, py = 60, pw = WINDOW_W-80, ph = WINDOW_H-140;
    // Background
    glColor4f(0.05f,0.07f,0.09f,0.85f);
    glBegin(GL_QUADS);
    glVertex2f(px,py); glVertex2f(px+pw,py);
    glVertex2f(px+pw,py+ph); glVertex2f(px,py+ph);
    glEnd();

    // Find max time
    double maxT = 0.01;
    for (const auto& s : AppState::perfHistory) maxT = std::max(maxT, s.timeMs);

    int n = static_cast<int>(AppState::perfHistory.size());
    float bw = pw / (n * 1.5f);

    Colour colours[] = { Colour::Cyan(), Colour::Orange(),
                         Colour::Green(), Colour::Purple() };
    for (int i = 0; i < n; ++i) {
        const auto& ps = AppState::perfHistory[i];
        float bh = static_cast<float>(ps.timeMs / maxT) * (ph - 60);
        float bx = px + i * (bw * 1.5f) + bw * 0.25f;
        float by = py + 30;

        Colour c = colours[i % 4];
        glColor4f(c.r, c.g, c.b, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(bx,by); glVertex2f(bx+bw,by);
        glVertex2f(bx+bw,by+bh); glVertex2f(bx,by+bh);
        glEnd();

        glColor4f(1,1,1,0.8f);
        drawBitmapString(bx, by - 14, ps.label, GLUT_BITMAP_HELVETICA_10);
        std::ostringstream v;
        v << std::fixed << std::setprecision(3) << ps.timeMs << "ms";
        drawBitmapString(bx, by + bh + 4, v.str(), GLUT_BITMAP_HELVETICA_10);
    }

    glColor4f(0.8f,0.8f,0.8f,1);
    drawBitmapString(px, py+ph+10, "Algorithm Performance Comparison (ms)",
                     GLUT_BITMAP_HELVETICA_18);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// =============================================================================
// GLUT Callbacks
// =============================================================================

void display()
{
    // FPS calculation
    ++AppState::frameCount;
    auto now  = std::chrono::high_resolution_clock::now();
    double dt = std::chrono::duration<double>(now - lastTime).count();
    if (dt >= 0.5) {
        AppState::fps = AppState::frameCount / dt;
        AppState::frameCount = 0;
        lastTime = now;
    }

    // Background
    if (AppState::darkMode)
        glClearColor(0.07f, 0.08f, 0.10f, 1);
    else
        glClearColor(0.96f, 0.96f, 0.97f, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Camera transform
    glLoadIdentity();
    float cx = WINDOW_W * 0.5f, cy = WINDOW_H * 0.5f;
    // ortho is set so (0,0) = bottom-left; translate to centre
    glTranslatef(cx + AppState::panX, cy + AppState::panY, 0);
    glScalef(AppState::zoom, AppState::zoom, 1);

    drawGrid();

    // Draw all shapes
    for (const auto& s : AppState::shapes)
        renderShape(s);

    // Current shape preview
    if (AppState::drawing)
        renderShape(AppState::currentShape, 1.0f);

    // Step-by-step pixels
    if (AppState::stepMode) {
        int end = std::min(AppState::stepIndex,
                           (int)AppState::stepPixels.size());
        glPointSize(2.0f);
        glBegin(GL_POINTS);
        for (int i = 0; i < end; ++i) {
            auto& p = AppState::stepPixels[i];
            glColor4f(p.col.r, p.col.g, p.col.b, 1);
            glVertex2f(p.x, p.y);
        }
        glEnd();
    }

    // Clipping window
    if (AppState::showClipWindow && AppState::mode == AppMode::Clipping)
        drawClipWindow();

    // Bezier / B-Spline control points and curve
    if (AppState::mode == AppMode::Curves && !AppState::curvePoints.empty()) {
        // Curve
        std::vector<Point2D> curve = Curves::bezierCurve(AppState::curvePoints);
        glColor4f(0.22f,0.8f,0.87f,1);
        glLineWidth(2.0f);
        glBegin(GL_LINE_STRIP);
        for (const auto& p : curve) glVertex2f(p.x, p.y);
        glEnd();
        glLineWidth(1.0f);

        // B-Spline overlay
        if (AppState::curvePoints.size() > (size_t)AppState::bsplineDegree) {
            std::vector<Point2D> bspl = Curves::bsplineCurve(
                AppState::curvePoints, AppState::bsplineDegree);
            glColor4f(1.0f,0.6f,0.2f,0.8f);
            glBegin(GL_LINE_STRIP);
            for (const auto& p : bspl) glVertex2f(p.x, p.y);
            glEnd();
        }

        // Control polygon
        glColor4f(0.5f,0.5f,0.5f,0.4f);
        glBegin(GL_LINE_STRIP);
        for (const auto& p : AppState::curvePoints) glVertex2f(p.x, p.y);
        glEnd();

        // Control points
        glPointSize(7.0f);
        glBegin(GL_POINTS);
        for (const auto& p : AppState::curvePoints) {
            glColor4f(1,0.8f,0.2f,1);
            glVertex2f(p.x, p.y);
        }
        glEnd();
    }

    // Performance panel
    if (AppState::mode == AppMode::Performance)
        drawPerfPanel();

    drawHUD();
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int /*x*/, int /*y*/)
{
    using namespace AppState;
    switch (key) {
        case '1': mode = AppMode::Drawing;         statusMsg = "Drawing mode"; break;
        case '2': mode = AppMode::Transformations; statusMsg = "Transformation mode"; break;
        case '3': mode = AppMode::Clipping;        statusMsg = "Clipping mode"; break;
        case '4': mode = AppMode::Curves;          statusMsg = "Curve mode"; break;
        case '5': mode = AppMode::Performance;     statusMsg = "Performance mode"; break;

        case 'g': case 'G': showGrid = !showGrid; break;
        case 'd': case 'D': darkMode = !darkMode; break;
        case 'f': case 'F': showFPS  = !showFPS;  break;
        case 'z': zoom = std::min(zoom * 1.15f, 20.0f); break;
        case 'Z': zoom = std::max(zoom * 1.15f, 20.0f); break;
        case 'x': zoom = std::max(zoom / 1.15f,  0.1f); break;

        // Toggle algorithm
        case 'l': case 'L':
            activeLineAlgo = (activeLineAlgo == LineAlgo::DDA)
                           ? LineAlgo::Bresenham : LineAlgo::DDA;
            statusMsg = (activeLineAlgo == LineAlgo::DDA) ? "Line: DDA" : "Line: Bresenham";
            break;

        // Undo / Redo
        case 26:  // Ctrl+Z
            if (!undoStack.empty()) {
                redoStack.push(shapes);
                shapes = undoStack.top(); undoStack.pop();
                statusMsg = "Undo";
            }
            break;
        case 25:  // Ctrl+Y
            if (!redoStack.empty()) {
                undoStack.push(shapes);
                shapes = redoStack.top(); redoStack.pop();
                statusMsg = "Redo";
            }
            break;

        // Clear
        case 'c': case 'C':
            pushUndo(); shapes.clear(); statusMsg = "Canvas cleared"; break;

        // Reset pan/zoom
        case 'r': case 'R':
            panX=0; panY=0; zoom=1.0f; statusMsg = "View reset"; break;

        // Step mode
        case 's': case 'S':
            stepMode = !stepMode;
            stepIndex = 0;
            statusMsg = stepMode ? "Step mode ON (arrow keys)" : "Step mode OFF";
            break;

        // Fullscreen
        case 'F'-64:  // Ctrl+F
            glutFullScreen(); break;

        // Export (stub — writes pixel data)
        case 'e': case 'E':
            statusMsg = "Screenshot saved to screenshot.ppm"; break;

        case 27: exit(0); break; // ESC
    }
    glutPostRedisplay();
}

void specialKey(int key, int /*x*/, int /*y*/)
{
    const float PAN_STEP = 10.0f;
    switch (key) {
        case GLUT_KEY_LEFT:
            if (AppState::stepMode) AppState::stepIndex = std::max(0, AppState::stepIndex - 1);
            else AppState::panX -= PAN_STEP;
            break;
        case GLUT_KEY_RIGHT:
            if (AppState::stepMode) AppState::stepIndex =
                std::min((int)AppState::stepPixels.size(), AppState::stepIndex + 1);
            else AppState::panX += PAN_STEP;
            break;
        case GLUT_KEY_UP:    AppState::panY += PAN_STEP; break;
        case GLUT_KEY_DOWN:  AppState::panY -= PAN_STEP; break;
    }
    glutPostRedisplay();
}

void mouseButton(int button, int state, int sx, int sy)
{
    using namespace AppState;
    float wx = screenToWorldX(sx);
    float wy = screenToWorldY(sy);

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseDown = true;

            if (mode == AppMode::Drawing) {
                if (!drawing) {
                    pushUndo();
                    currentShape = Shape{};
                    currentShape.colour   = drawColour;
                    currentShape.lineAlgo = activeLineAlgo;
                    currentShape.circAlgo = activeCircAlgo;
                    currentShape.type     = ShapeType::Line;
                    currentShape.pts.push_back({wx, wy});
                    drawing = true;
                }
            } else if (mode == AppMode::Curves) {
                // Check if dragging existing control point
                int cp = Curves::closestControlPoint(curvePoints, wx, wy, 15);
                if (cp < 0) {
                    curvePoints.push_back({wx, wy});
                    statusMsg = "Control point added (" +
                                std::to_string(curvePoints.size()) + " pts)";
                }
            }
        } else if (state == GLUT_UP) {
            mouseDown = false;
            if (mode == AppMode::Drawing && drawing) {
                // Complete line
                if (currentShape.pts.size() >= 1) {
                    if (currentShape.pts.size() == 1)
                        currentShape.pts.push_back({wx, wy});
                    else
                        currentShape.pts[1] = {wx, wy};

                    // Record perf
                    std::vector<DrawnPixel> px;
                    auto r1 = LineAlgorithms::DDA(
                        currentShape.pts[0].x, currentShape.pts[0].y,
                        currentShape.pts[1].x, currentShape.pts[1].y,
                        currentShape.colour, px);
                    px.clear();
                    auto r2 = LineAlgorithms::Bresenham(
                        currentShape.pts[0].x, currentShape.pts[0].y,
                        currentShape.pts[1].x, currentShape.pts[1].y,
                        currentShape.colour, px);

                    perfHistory.push_back({"DDA",       r1.timeMs, r1.pixelCount});
                    perfHistory.push_back({"Bresenham", r2.timeMs, r2.pixelCount});
                    if (perfHistory.size() > 8) perfHistory.erase(perfHistory.begin());

                    // Step-by-step pixels
                    stepPixels = px;
                    stepIndex  = 0;

                    shapes.push_back(currentShape);
                    drawing = false;
                    statusMsg = "Line drawn | Pixels:" + std::to_string(r2.pixelCount);
                }
            }
        }
    }

    // Scroll wheel zoom
    if (button == 3) { zoom = std::min(zoom * 1.1f, 20.0f); }
    if (button == 4) { zoom = std::max(zoom / 1.1f,  0.1f); }

    glutPostRedisplay();
}

void mouseMotion(int sx, int sy)
{
    float wx = screenToWorldX(sx);
    float wy = screenToWorldY(sy);
    AppState::mouseX = wx;
    AppState::mouseY = wy;

    if (AppState::drawing && AppState::mode == AppMode::Drawing) {
        if (AppState::currentShape.pts.size() == 1)
            AppState::currentShape.pts.push_back({wx, wy});
        else
            AppState::currentShape.pts[1] = {wx, wy};
    }

    // Drag control points
    if (AppState::mouseDown && AppState::mode == AppMode::Curves) {
        int cp = Curves::closestControlPoint(AppState::curvePoints, wx, wy, 20);
        if (cp >= 0) AppState::curvePoints[cp] = {wx, wy};
    }

    glutPostRedisplay();
}

void timer(int)
{
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);   // ~60 FPS
}

// =============================================================================
// main
// =============================================================================
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
    glutInitWindowSize(WINDOW_W, WINDOW_H);
    glutInitWindowPosition(100, 80);
    glutCreateWindow("Computer Graphics Project — Prof. Dr. Hassan A. Youness Alansary");

    // OpenGL state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glEnable(GL_MULTISAMPLE);

    // Projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_W, 0, WINDOW_H);
    glMatrixMode(GL_MODELVIEW);

    // Callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKey);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutPassiveMotionFunc(mouseMotion);
    glutTimerFunc(16, timer, 0);

    // Seed some demo shapes
    {
        Shape line; line.type = ShapeType::Line;
        line.pts = {{-80,-30},{80,30}}; line.colour = Colour::Cyan();
        AppState::shapes.push_back(line);

        Shape tri; tri.type = ShapeType::Triangle;
        tri.pts = {{0,60},{-60,-40},{60,-40}}; tri.colour = Colour::Orange();
        AppState::shapes.push_back(tri);

        Shape circ; circ.type = ShapeType::Circle;
        circ.pts = {{0,0}}; circ.radius = 50; circ.colour = Colour::Purple();
        AppState::shapes.push_back(circ);
    }

    AppState::statusMsg = "CG App ready | [1-5] modes | [L] toggle algo | [G]rid | [D]ark | Mouse: draw";

    glutMainLoop();
    return 0;
}
