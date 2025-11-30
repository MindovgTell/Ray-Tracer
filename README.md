# Ray Tracer (OpenCL)  
_A small GPU path tracer inspired by **Ray Tracing in One Weekend**._

<p align="center">
  <img src=https://github.com/MindovgTell/Ray-Tracer/blob/master/images/rednerer4.png alt="Hero render" width="80%">
</p>

## Highlights
- GPU-accelerated rendering with OpenCL 1.2 (vendor-agnostic).
- Progressive path tracing with anti-aliasing and sky lighting.
- Lambertian, metal, dielectric materials. Multiple spheres, ground plane; emissive support.
- Simple, extensible codebase (C++ host + OpenCL kernels).

---

## Gallery
<!-- > Drop your images here (PNG/JPG). Keep a few noisy → clean comparisons to show convergence. -->

<p align="center">
  <img src="https://github.com/MindovgTell/Ray-Tracer/blob/master/images/renderer2.png" alt="Spheres1" width="48%">
</p>
<p align="center">
  <img src="https://github.com/MindovgTell/Ray-Tracer/blob/master/images/rednerer3.png" alt="Spheres2" width="48%">
</p>
<!-- 
<p align="center">
  <img src="docs/aa_compare.png" alt="AA comparison" width="80%">
</p> -->

---

## Quick Start

### Requirements
- **C++17**, **CMake ≥ 3.16**
- **OpenCL 1.2+**

### Build
```bash
git clone https://github.com/MindovgTell/RayTracer.git
cd RayTracer/scripts
./build.sh
