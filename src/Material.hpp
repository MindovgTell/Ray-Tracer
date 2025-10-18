#ifndef MATERIAL_HPP
#define MATERIAL_HPP



enum class MatTag : int32_t { Lambert=0, Metal=1, Dielectric=2 };

struct mat_gpu {
    vec3 albedo;
    float fuzz;   // xyz albedo; w=fuzz or 0
    float     ref_idx;       // dielectric eta
    int32_t   tag;           // MatTag
};

struct Material {
  virtual ~Material() = default;
  virtual MatTag tag() const = 0;
  virtual mat_gpu toDto() const = 0;
};

struct Lambertian : Material {
  glm::vec3 albedo{};
  MatTag tag() const override { return MatTag::Lambert; }
  mat_gpu toDto() const override {
    mat_gpu g{};
    g.albedo = vec3{albedo.x, albedo.y, albedo.z};
    g.fuzz = 0.0f;
    g.ref_idx = 0.0f;
    g.tag = (int32_t)MatTag::Lambert;
    return g;
  }
};

struct Metal : Material {
  glm::vec3 albedo{}; float fuzz{0};
  MatTag tag() const override { return MatTag::Metal; }
  mat_gpu toDto() const override {
    mat_gpu g{};
    g.albedo = vec3{albedo.x, albedo.y, albedo.z};
    g.fuzz = fuzz;
    g.ref_idx = 0.0f;
    g.tag = (int32_t)MatTag::Metal;
    return g;
  }
};

struct Dielectric : Material {
  float ref_idx{1.5f};
  MatTag tag() const override { return MatTag::Dielectric; }
  mat_gpu toDto() const override {
    mat_gpu g{};
    g.albedo = vec3{1,1,1}; // not used
    g.fuzz = 0.0f;
    g.ref_idx = ref_idx;
    g.tag = (int32_t)MatTag::Dielectric;
    return g;
  }
};

#endif  // MATERIAL_HPP