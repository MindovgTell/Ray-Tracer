#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP

#include <unordered_map>
#include "DTOs.hpp"



namespace compute::serialize {

    inline cl_float3 to_f3(const glm::vec3& v) { return {v.x, v.y, v.z}; }
    // inline cl_float3 to_f3(glm::vec3 v) { return {v.x, v.y, v.z}; }
    inline cl_float4 to_f4(const glm::vec3& v, float w = 0.f) { return {v.x, v.y, v.z, w}; }
    // inline cl_float4 to_f4(glm::vec3 v, float w = 0.f) { return {v.x, v.y, v.z, w}; }

    inline SphereGpu to_gpu(const Sphere& s) {
        SphereGpu g{};
        g.center_r          = to_f4(s.get_center_pos(), s.get_radius());
        g.emission        = to_f4(s.get_emission());

        return g;
    }

    inline std::vector<SphereGpu> to_gpu(std::vector<Sphere>& in) {
        std::vector<SphereGpu> out;
        out.reserve(in.size());
        for (const auto& s : in) out.push_back(to_gpu(s));
        return out;
    }


    inline CameraGpu to_gpu(const Camera& c) {
        CameraGpu g{};
        g.origin  = to_f4(c.get_origin());
        g.pixel_delta_x = to_f4(c.get_pixel_delta_x());
        g.pixel_delta_y = to_f4(c.get_pixel_delta_y());
        g.pixel00_pos   = to_f4(c.get_pixel00_location());
        return g;
    }

    // Main packer: builds a PackedScene from a host Scene + Camera
    inline PackedScene pack_scene(const Scene& src, const Camera& cam)
    {
        PackedScene out{};
        out.camera = to_gpu(cam);

        // Materials
        out.materials.reserve(src.materials.size());

        std::unordered_map<const Material*, int> mat_index; 

        auto add_material = [&](const std::shared_ptr<Material>& mptr) -> int {
        auto* key = mptr.get();
        if (auto it = mat_index.find(key); it != mat_index.end())
            return it->second;
        MaterialGpu mg{}; // zero-init padding too
        if (auto* lam = dynamic_cast<Lambertian*>(key); lam != nullptr) {
            mg.type      = MAT_LAMBERTIAN;
            mg.albedo_fuzz    = to_f4(lam->albedo, 0.0f);
            mg.ref_idx        = 1.0f;
        } else if (auto* met = dynamic_cast<Metal*>(key); met != nullptr) {
            mg.type      = MAT_METAL;
            mg.albedo_fuzz    = to_f4(met->albedo, met->fuzz);
            mg.ref_idx       = 1.0f;
        } else if (auto* die = dynamic_cast<Dielectric*>(key); die != nullptr) {
            mg.type      = MAT_DIELECTRIC;
            mg.albedo_fuzz   = to_f4({1,1,1}, 0.0f); // unused; keep sane default
            mg.ref_idx   = die->ref_idx;
        } else {
            throw std::runtime_error("Unknown material subtype");
        }

        int idx = (int)out.materials.size();
        mat_index.emplace(key, idx);
        out.materials.push_back(mg);
        return idx;

        };

        for (auto& m : src.materials) add_material(m);
        // Spheres
        out.spheres.reserve(src.spheres.size());
        for (auto& s : src.spheres) {
            SphereGpu gs{};
            gs.center_r        = to_f4(s.get_center_pos(), s.get_radius());
            gs.emission        = to_f4(s.get_emission());
            gs.material_index  = add_material(s.get_material_ptr()); // reuses index if already added
            out.spheres.push_back(gs);
        }

        // // Meshes: flatten into SoA + concatenated triangle list
        // out.positions4.clear(); out.normals4.clear(); out.uvs4.clear();
        // out.triangles.clear();  out.meshes.clear();
        // for (const auto& m : src.meshes) {
        //     MeshGpu mg{};
        //     mg.vert_offset  = (uint32_t)out.positions4.size();
        //     mg.vert_count   = (uint32_t)m.positions.size();
        //     mg.index_offset = (uint32_t)out.triangles.size();
        //     mg.index_count  = (uint32_t)m.indices.size();
        //     mg.material_index = m.material_index;
        //     // positions
        //     out.positions4.reserve(out.positions4.size() + m.positions.size());
        //     for (auto& p : m.positions) out.positions4.push_back(to_f4(p));
        //     // normals (optional)
        //     if (!m.normals.empty()) {
        //         out.normals4.reserve(out.normals4.size() + m.normals.size());
        //         for (auto& n : m.normals) out.normals4.push_back(to_f4(n));
        //     }
        //     // uvs (optional)
        //     if (!m.uvs.empty()) {
        //         out.uvs4.reserve(out.uvs4.size() + m.uvs.size());
        //         for (auto& t : m.uvs) out.uvs4.push_back(cl_float4{t.x, t.y, 0.f, 0.f});
        //     }
        //     // triangles
        //     for (size_t i = 0; i + 2 < m.indices.size(); i += 3) {
        //         TriGpu tri{};
        //         tri.i0 = (uint32_t)(m.indices[i+0] + mg.vert_offset);
        //         tri.i1 = (uint32_t)(m.indices[i+1] + mg.vert_offset);
        //         tri.i2 = (uint32_t)(m.indices[i+2] + mg.vert_offset);
        //         tri.material_index = m.material_index;
        //         out.triangles.push_back(tri);
        //     }
        //     out.meshes.push_back(mg);
        // }

        return out;
    }



    
}



#endif // SERIALIZE_HPP