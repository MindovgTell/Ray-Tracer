# Ray Tracer (OpenCL)  
_A small GPU path tracer inspired by **Ray Tracing in One Weekend**._

<p align="center">
  <img src=https://github.com/MindovgTell/Ray-Tracer/blob/master/images/rednerer3.png alt="Hero render" width="80%">
</p>

## Highlights
- GPU-accelerated rendering with OpenCL 1.2 (vendor-agnostic).
- Progressive path tracing with anti-aliasing and sky lighting.
- Lambertian materials, multiple spheres, ground plane; emissive support.
- Optional OpenCL↔OpenGL interop for real-time preview (WIP).
- Simple, extensible codebase (C++ host + OpenCL kernels).

---

## Gallery
> Drop your images here (PNG/JPG). Keep a few noisy → clean comparisons to show convergence.

<p align="center">
  <img src="https://github.com/MindovgTell/Ray-Tracer/blob/master/images/renderer2.png" alt="Spheres noisy" width="48%">
</p>
<!-- 
<p align="center">
  <img src="docs/aa_compare.png" alt="AA comparison" width="80%">
</p> -->

---

## Quick Start

### Requirements
- **C++17**, **CMake ≥ 3.16**
- **OpenCL 1.2+** runtime (NVIDIA/AMD/Intel/etc.)
- (Optional) **OpenGL** for interactive preview

### Build
```bash
git clone https://github.com/MindovgTell/RayTracer.git
cd RayTracer/scripts
./build.sh
