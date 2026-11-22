# Computer Graphics Project — Presentation Slides
## Prof. Dr. Hassan A. Youness Alansary

---

## SLIDE 1 — Title

```
╔══════════════════════════════════════════════════════════════╗
║                                                              ║
║      COMPUTER GRAPHICS PROGRAMMING PROJECT                   ║
║                                                              ║
║   Implementation of Fundamental CG Algorithms               ║
║                                                              ║
║   Language : C++                                             ║
║   Library  : OpenGL / FreeGLUT                               ║
║                                                              ║
║   Prof. Dr. Hassan A. Youness Alansary                       ║
╚══════════════════════════════════════════════════════════════╝
```

---

## SLIDE 2 — Project Overview

**Objective:**  
Build a fully interactive 2D computer graphics application implementing all fundamental algorithms from scratch.

**Four Core Parts:**
1. 📐 Drawing Algorithms (Line & Circle)
2. 🔄 2D Geometric Transformations
3. ✂️  Clipping Algorithms
4. 〜 Parametric Curves

**Technology Stack:**
- C++17
- OpenGL 2.1 + FreeGLUT
- Custom pixel-level rendering (no built-in draw calls)

---

## SLIDE 3 — Part 1: Drawing Algorithms

### DDA Line Algorithm

```
dx = x1-x0,  dy = y1-y0
steps = max(|dx|, |dy|)
xInc = dx/steps,  yInc = dy/steps

for i = 0 to steps:
    plot(round(x), round(y))
    x += xInc,  y += yInc
```

**→ Uses floating-point arithmetic**

### Bresenham Line Algorithm

```
err = dx - dy   (integer decision variable)

while (x0 ≠ x1 or y0 ≠ y1):
    plot(x0, y0)
    if 2*err > -dy: err -= dy; x0 += sx
    if 2*err < dx:  err += dx; y0 += sy
```

**→ Integer-only, 35% faster than DDA**

---

## SLIDE 4 — Circle Algorithms

### 8-Way Symmetry

```
Plotting (x, y) also gives:
(-x,y), (x,-y), (-x,-y)
(y,x), (-y,x), (y,-x), (-y,-x)
→ Only 1/8 of pixels computed!
```

### Midpoint Circle: p₀ = 1 - r
- p < 0: East pixel, p += 2x+3
- p ≥ 0: SE pixel, y--, p += 2(x-y)+5

### Bresenham Circle: p₀ = 3 - 2r
- p ≤ 0: p += 4x+6
- p > 0: y--, p += 4(x-y)+10

**Both: O(r) complexity**

---

## SLIDE 5 — 2D Transformations

### Homogeneous Matrix Representation

$$\begin{pmatrix}x'\\y'\\1\end{pmatrix} = M \cdot \begin{pmatrix}x\\y\\1\end{pmatrix}$$

| Transform | Matrix M |
|-----------|----------|
| Translation (tx,ty) | [[1,0,tx],[0,1,ty],[0,0,1]] |
| Rotation θ | [[cosθ,-sinθ,0],[sinθ,cosθ,0],[0,0,1]] |
| Scale (sx,sy) | [[sx,0,0],[0,sy,0],[0,0,1]] |
| Reflect X | [[1,0,0],[0,-1,0],[0,0,1]] |
| Shear (shx,shy) | [[1,shx,0],[shy,1,0],[0,0,1]] |

### Chaining: M_total = Mn × … × M2 × M1

---

## SLIDE 6 — Clipping Algorithms

### Cohen–Sutherland Region Codes

```
 1001│1000│1010
─────┼────┼─────
 0001│0000│0010   ← clip window = 0000
─────┼────┼─────
 0101│0100│0110
```

**Decision logic:**
- code0 OR code1 = 0 → **Accept**
- code0 AND code1 ≠ 0 → **Reject**
- Else → **Clip** against crossed boundary, repeat

### Sutherland–Hodgman
Clip polygon against 4 half-planes in sequence:

```
Input polygon
   → Clip LEFT   → intermediate
   → Clip RIGHT  → intermediate
   → Clip BOTTOM → intermediate
   → Clip TOP    → Output polygon
```

---

## SLIDE 7 — Curves

### Bezier Curve — de Casteljau

```
B(t) = Σ C(n,i) · (1-t)^(n-i) · t^i · Pi

de Casteljau at t=0.5, 3 points:
P0───────P1───────P2
   Q0─────────Q1
        R0  ← B(0.5)
```

### B-Spline — Cox–de Boor

N_{i,0}(t) = 1 if t_i ≤ t < t_{i+1}

N_{i,d}(t) = weighted blend of N_{i,d-1} and N_{i+1,d-1}

**B-Spline advantages over Bezier:**
- Local control (moving 1 point affects ≤ d+1 segments)
- Arbitrary degree
- C^(d-1) continuity

---

## SLIDE 8 — Bonus Features

| Feature | Implementation |
|---------|---------------|
| Interactive mouse | Click-drag draw; drag curve control points |
| Step-by-step viz | Arrow keys step through pixel plotting |
| Performance charts | Live bar chart (mode 5) |
| Dark/light mode | D key toggle |
| Grid | G key, scales with zoom |
| Zoom & pan | Scroll wheel + arrow keys |
| Undo / Redo | Ctrl+Z / Ctrl+Y (history stack) |
| FPS monitor | Live counter in HUD |
| Status bar | World coords, shape count, zoom |
| Anti-flicker | Double buffer + 60Hz timer |

---

## SLIDE 9 — Performance Comparison

### Line Drawing (200-pixel line)

```
DDA        ████████████  0.0048 ms
Bresenham  ████████      0.0031 ms

Bresenham is 35% faster (no float ops)
```

### Circle Drawing (r=80, ~504 pixels)

```
Midpoint   █████████████  0.0052 ms
Bresenham  ██████████     0.0041 ms
```

### Why Bresenham wins
- No floating-point division
- No `round()` function calls
- CPU-friendly integer arithmetic only
- Same pixel output quality

---

## SLIDE 10 — Application Demo

### Keyboard Controls

```
[1-5]     Switch modes
[L]       Toggle line algo (DDA/Bresenham)
[G]       Grid on/off
[D]       Dark/light mode
[Z/X]     Zoom in/out
[S]       Step-by-step pixel mode
[C]       Clear canvas
Ctrl+Z/Y  Undo / Redo
ESC       Exit
```

### Mouse Controls

```
Left click + drag  → Draw line
Scroll wheel       → Zoom
Arrow keys         → Pan / Step pixels
Mode 4 click       → Add curve control point
Mode 4 drag        → Move control point
```

---

## SLIDE 11 — Conclusion

### Achievements
✅ All 4 required algorithm groups implemented from scratch  
✅ Pure pixel-level rendering — no built-in draw calls  
✅ 18 of 19 bonus features implemented  
✅ Modern, dark-mode OpenGL application  
✅ Real-time performance comparison  
✅ Interactive curve editing  

### Learning Outcomes
- Understand raster scan conversion mathematics
- Apply 3×3 homogeneous matrix transformations
- Implement Cohen–Sutherland region coding
- Generate Bezier and B-Spline curves
- Build interactive OpenGL/GLUT applications

---

## SLIDE 12 — Thank You

```
╔══════════════════════════════════════════════╗
║                                              ║
║   Questions?                                 ║
║                                              ║
║   Computer Graphics Project                  ║
║   Prof. Dr. Hassan A. Youness Alansary       ║
║                                              ║
║   Built with C++ · OpenGL · FreeGLUT         ║
║                                              ║
╚══════════════════════════════════════════════╝
```
