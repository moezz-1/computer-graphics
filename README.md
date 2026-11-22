# Computer Graphics Project
### Prof. Dr. Hassan A. Youness Alansary

> **Implementation of Fundamental Computer Graphics Algorithms**  
> Interactive 2D graphics application built with C++ and OpenGL/GLUT.

---

## 📁 Project Structure

```
CGProject/
├── main.cpp                        # Application entry point, GLUT callbacks, rendering
├── CMakeLists.txt                  # CMake build configuration
├── include/
│   └── CGApp.h                    # Master header — all types, global state
├── src/
│   ├── algorithms/
│   │   ├── LineAlgorithms.h/.cpp  # DDA + Bresenham line algorithms
│   │   └── CircleAlgorithms.h/.cpp# Midpoint + Bresenham circle algorithms
│   ├── transformations/
│   │   └── Transformations.h/.cpp # All 2D transforms via 3×3 homogeneous matrices
│   ├── clipping/
│   │   └── Clipping.h/.cpp        # Cohen–Sutherland + Sutherland–Hodgman
│   └── curves/
│       └── Curves.h/.cpp          # Bezier (de Casteljau) + B-Spline (Cox–de Boor)
├── assets/                         # Icons, fonts, sample files
├── screenshots/                    # Application screenshots
├── report/                         # PDF report
└── slides/                         # Presentation slides
```

---

## 🛠️ Build Instructions

### Prerequisites

| Platform | Requirement |
|----------|-------------|
| Linux    | `sudo apt install freeglut3-dev libgl1-mesa-dev cmake g++` |
| macOS    | Xcode Command Line Tools (GLUT included) |
| Windows  | Visual Studio + freeglut NuGet package |

### Linux / macOS

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./CGProject
```

### Windows (Visual Studio)

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
Release\CGProject.exe
```

### One-liner (Linux)

```bash
g++ -std=c++17 main.cpp src/**/*.cpp -I. -lGL -lGLU -lglut -o CGProject && ./CGProject
```

---

## 🎮 Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `1` | Drawing mode |
| `2` | Transformation mode |
| `3` | Clipping mode |
| `4` | Curve mode |
| `5` | Performance comparison |
| `L` | Toggle line algorithm (DDA ↔ Bresenham) |
| `G` | Toggle grid |
| `D` | Toggle dark/light mode |
| `Z` / `X` | Zoom in / out |
| `R` | Reset view |
| `S` | Toggle step-by-step mode |
| `C` | Clear canvas |
| `Ctrl+Z` | Undo |
| `Ctrl+Y` | Redo |
| `←→↑↓` | Pan canvas (or step through pixels in step mode) |
| `ESC` | Quit |

---

## 🖱️ Mouse Controls

| Action | Effect |
|--------|--------|
| Click + drag | Draw line (in Drawing mode) |
| Click | Add control point (in Curve mode) |
| Drag near point | Move control point (in Curve mode) |
| Scroll wheel | Zoom in/out |

---

## 📐 Algorithms Implemented

### Part 1: Drawing Algorithms

#### DDA (Digital Differential Analyzer)
- Computes increments `dx/steps` and `dy/steps`
- Plots pixels by accumulating fractional coordinates
- Uses floating-point arithmetic
- **Complexity:** O(max(|Δx|, |Δy|))

#### Bresenham Line Algorithm
- Pure integer arithmetic (no division, no float)
- Decision variable `p = 2|Δy| - |Δx|` updated each step
- Faster than DDA; no rounding errors
- **Complexity:** O(max(|Δx|, |Δy|))

#### Midpoint Circle Algorithm
- Decision variable `p = 1 - r`
- Exploits 8-way symmetry — only 1/8 of circle computed
- **Complexity:** O(r)

#### Bresenham Circle Algorithm
- Decision variable `p = 3 - 2r`
- Integer-only, 8-way symmetry
- **Complexity:** O(r)

### Part 2: 2D Transformations (3×3 Homogeneous Matrices)

| Transform | Matrix |
|-----------|--------|
| Translation | `[[1,0,tx],[0,1,ty],[0,0,1]]` |
| Rotation θ | `[[cosθ,-sinθ,0],[sinθ,cosθ,0],[0,0,1]]` |
| Scale | `[[sx,0,0],[0,sy,0],[0,0,1]]` |
| Reflection X | `[[1,0,0],[0,-1,0],[0,0,1]]` |
| Shear | `[[1,shx,0],[shy,1,0],[0,0,1]]` |

Transformations can be **chained** by matrix multiplication.

### Part 3: Clipping

#### Cohen–Sutherland
- 4-bit region code per endpoint (LEFT, RIGHT, BOTTOM, TOP)
- Trivial accept/reject → clip against boundary for mixed cases
- Iterates until fully inside or fully outside

#### Sutherland–Hodgman
- Clips polygon against each of 4 half-planes in sequence
- Outputs intermediate polygons at each clipping stage
- Handles concave polygons correctly

### Part 4: Curves

#### Bezier (de Casteljau)
- Recursive linear interpolation at parameter t
- `B(t) = Σ C(n,i) · (1-t)^(n-i) · t^i · Pi`
- Numerically stable, supports subdivision

#### B-Spline (Cox–de Boor)
- `C(t) = Σ Pi · N_{i,d}(t)` via Cox–de Boor recurrence
- Clamped uniform knot vector
- Degree-adjustable (default cubic d=3)

---

## 🎓 Viva Questions & Answers

**Q: What is the main advantage of Bresenham over DDA?**  
A: Bresenham uses only integer arithmetic, making it faster and free of floating-point rounding errors.

**Q: Why does the circle algorithm use 8-way symmetry?**  
A: A circle is symmetric about both axes and both diagonals, so computing one octant gives all 8 points for free, reducing computation by 87.5%.

**Q: What are region codes in Cohen–Sutherland?**  
A: 4-bit binary codes encoding which side(s) of the clip rectangle an endpoint lies outside. Trivial rejection occurs when both codes share a set bit.

**Q: What is de Casteljau's algorithm?**  
A: A recursive interpolation scheme: given control points, repeatedly lerp between consecutive points until one point remains — that point is B(t).

**Q: Why use homogeneous coordinates for transformations?**  
A: They unify translation (additive) with rotation/scale (multiplicative) into a single matrix multiply, enabling efficient transformation chaining.

---

## 📊 Expected Performance

| Algorithm | Typical Time (100px line) | Pixels |
|-----------|---------------------------|--------|
| DDA | ~0.005 ms | ~100 |
| Bresenham line | ~0.003 ms | ~100 |
| Midpoint circle r=50 | ~0.004 ms | ~400 |
| Bresenham circle r=50 | ~0.003 ms | ~400 |

---

## 📝 Report

See `report/CG_Project_Report.md` for the full professional report.

## 📊 Slides

See `slides/CG_Project_Slides.md` for presentation slides.


How to run project.

use this:


mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .

.\CGProject.exe


