#ifndef DTOS_HPP
#define DTOS_HPP

#include <unordered_map>

namespace compute::serialize {
    
    struct MaterialGpu {
        cl_float4 albedo_fuzz;   // xyz albedo; w=fuzz or 0
        float     ref_idx;       // dielectric eta
        int32_t   tag;           // MatTag
        int32_t   _pad0, _pad1;
    };

    struct SphereGpu {
        cl_float4 center_r;     // xyz used
        cl_float4 color;
        cl_float4 emission;
        cl_int    material_index; cl_int _padS0,_padS1,_padS2;
    };

    // Triangels and meshes will be support in future versions
    // struct TriGpu { uint32_t i0,i1,i2, material_index; };

    // struct MeshGpu {
    //     uint32_t index_offset, index_count;   // indices slice (triangles)
    //     uint32_t vert_offset,  vert_count;    // vertices slice
    //     int32_t  material_index; int32_t _pad[3];
    // };

    struct CameraGpu {
        // Camera settings
        cl_float4 origin_focal; // float4(origin.x, origin.y, origin.z, focal_length) the 4th argument is focal_length

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        cl_float4 pixel_delta_x;
        cl_float4 pixel_delta_y;

        cl_float4 pixel00_pos;
    };


    struct MaterialBake {
        std::vector<MaterialGpu> materials_gpu;
        std::unordered_map<const Material*, int> indexOf; // pointer → gpu index
    };

    inline MaterialBake bakeMaterials(const std::vector<std::shared_ptr<Material>>& mats) {
        MaterialBake out;
        out.materials_gpu.reserve(mats.size());
        out.indexOf.reserve(mats.size());
        for (const auto& m : mats) {
            if (!m) { out.materials_gpu.push_back(MaterialGpu{}); continue; }
            out.indexOf[m.get()] = (int)out.materials_gpu.size();
            out.materials_gpu.push_back(m->toDto());
        }
        return out;
    }

    // A fully flattened scene ready to upload
    struct PackedScene {
        CameraGpu camera;

        // // Mesh SoA
        // std::vector<cl_float4> positions4;  // xyz used
        // std::vector<cl_float4> normals4;    // optional (may be empty)
        // std::vector<cl_float4> uvs4;        // xy used

        // std::vector<TriGpu>    triangles;
        // std::vector<MeshGpu>   meshes;

        // Spheres & materials
        std::vector<SphereGpu>   spheres;
        std::vector<MaterialGpu> materials;
    };
}


#endif //DTOS_HPP
