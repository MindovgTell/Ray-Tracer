#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP



namespace compute::serialize {
    inline cl_float3 to_cl_float3(const glm::vec3& v) {
        // cl_float3 is a struct { cl_float x, y, z; } (often padded to 16 bytes)
        cl_float3 r;
        r.x = v.x; r.y = v.y; r.z = v.z;
        return r;
    }

    struct GpuCamera {
        cl_float origin;


    };

    struct GpuScene {
        
    };

    struct GpuMaterial {
        
    };

    struct GpuSphere {
        cl_float  radius;
        cl_float3 position;
        cl_float3 color;
        cl_float3 emission;
    };



    inline GpuSphere serialize_sphere(const Sphere& s) {
        GpuSphere g{};
        g.center          = to_cl_float3(s.get_center());
        g.radius          = s.get_radius();
        g.color           = to_cl_float3(s.get_color());
        g.emission        = to_cl_float3(s.get_emission());
        // g.material_index  = s.material_index;
        return g;
    }

    inline std::vector<GpuSphere> serialize_spheres(const std::vector<Sphere>& in) {
        std::vector<GpuSphere> out;
        out.reserve(in.size());
        for (const auto& s : in) out.push_back(serialize_sphere(s));
        return out;
    }
    
}



#endif // SERIALIZE_HPP