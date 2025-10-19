#ifndef DTOS_HPP
#define DTOS_HPP


namespace compute::serialize {

    #define MAT_LAMBERTIAN 0
    #define MAT_METAL 1
    #define MAT_DIELECTRIC 2


    struct CameraGpu {
        // Camera settings
        cl_float4 origin; 

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        cl_float4 pixel_delta_x;
        cl_float4 pixel_delta_y;

        cl_float4 pixel00_pos;
    };

    
    struct MaterialGpu {
        cl_float4 albedo_fuzz;   // xyz albedo; w=fuzz or 0
        cl_int    type;          // MatTag
        cl_float  ref_idx;       // dielectric eta
        cl_int    _pad0, _pad1;
    };

    struct SphereGpu {
        cl_float4 center_r;     // xyz used
        cl_float4 emission;
        cl_int    material_index; 
        cl_int _pad0,_pad1,_pad2;
    };

    // Triangels and meshes will be support in future versions
    // struct TriGpu { uint32_t i0,i1,i2, material_index; };

    // struct MeshGpu {
    //     uint32_t index_offset, index_count;   // indices slice (triangles)
    //     uint32_t vert_offset,  vert_count;    // vertices slice
    //     int32_t  material_index; int32_t _pad[3];
    // };

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
