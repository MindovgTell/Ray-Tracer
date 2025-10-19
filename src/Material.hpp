#ifndef MATERIAL_HPP
#define MATERIAL_HPP



enum class MatTag : int32_t { Lambert=0, Metal=1, Dielectric=2 };

struct Material {
    virtual ~Material() = default;
    virtual MatTag tag() const = 0;
};

struct Lambertian : Material {

    Lambertian(vec3 alb) : albedo(alb) {}

    glm::vec3 albedo{0.0f, 0.0f, 0.0f};
    MatTag tag() const override { return MatTag::Lambert; }
};

struct Metal : Material {

    Metal(vec3 alb, float f) : albedo(alb), fuzz(f) {}

    glm::vec3 albedo{}; float fuzz{0};
    MatTag tag() const override { return MatTag::Metal; }
};

struct Dielectric : Material {

    Dielectric(float r_i) : ref_idx(r_i) {}

    float ref_idx{1.5f};
    MatTag tag() const override { return MatTag::Dielectric; }
};

#endif  // MATERIAL_HPP