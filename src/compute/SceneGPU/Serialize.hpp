#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP



namespace compute::serialize {
    struct CameraGPU {
        cl_float origin;


    };

    struct SceneGPU {
        
    };

    struct MaterialGPU {
        
    };

    struct SphereGPU {
        cl_float  radius;
        cl_float3 position;
        cl_float3 color;
        cl_float3 emission;
    };

    
}



#endif // SERIALIZE_HPP