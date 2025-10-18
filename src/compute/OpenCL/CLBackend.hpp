#ifndef CLBACKEND_HPP
#define CLBACKEND_HPP


#define CL_HPP_TARGET_OPENCL_VERSION  120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120


#define CL_HPP_ENABLE_EXCEPTIONS


#ifdef __APPLE__
#define CL_SILENCE_DEPRECATION
#endif

#include <CL/opencl.hpp> 
#include <fstream>
#include <sstream>
#include "CLUtils.hpp"
#include "Backend.hpp"
#include "Serialize.hpp"



namespace compute {

    struct GpuSceneBuffers {
    // device buffers (owned, grown on demand)
    cl::Buffer spheres, materials, camera;
    cl::Buffer out_rgb;

    // sizes cached for ensure()
    size_t spheres_bytes = 0, materials_bytes = 0,
           camera_bytes = 0, out_rgb_bytes = 0;
    };

    inline void ensure(cl::Context& ctx, cl::Buffer& b, size_t needBytes, cl_mem_flags flags, size_t& cachedSize) {
        if (needBytes == 0) return;
        if (!b() || cachedSize < needBytes) {
            b = cl::Buffer(ctx, flags, needBytes);
            cachedSize = needBytes;
        }
    }

    inline void upload_scene(cl::Context& ctx, cl::CommandQueue& q,
                         const PackedScene& ps, GpuSceneBuffers& gpu)
    {
        // Ensure buffers
        ensure(ctx, gpu.spheres,    ps.spheres.size()*sizeof(serialize::SphereGpu),     CL_MEM_READ_ONLY, gpu.spheres_bytes);
        ensure(ctx, gpu.materials,  ps.materials.size()*sizeof(serialize::MaterialGpu), CL_MEM_READ_ONLY, gpu.materials_bytes);
        ensure(ctx, gpu.camera,     sizeof(serialize::CameraGpu),                       CL_MEM_READ_ONLY, gpu.camera_bytes);

        // Upload
        if (!ps.spheres.empty())    q.enqueueWriteBuffer(gpu.spheres,    CL_TRUE, 0, ps.spheres.size()*sizeof(serialize::SphereGpu),     ps.spheres.data());
        if (!ps.materials.empty())  q.enqueueWriteBuffer(gpu.materials,  CL_TRUE, 0, ps.materials.size()*sizeof(serialize::MaterialGpu), ps.materials.data());

        q.enqueueWriteBuffer(gpu.camera, CL_TRUE, 0, sizeof(serialize::CameraGpu), &ps.camera);
    }

    inline void ensure_output(cl::Context& ctx, GpuSceneBuffers& gpu, int W, int H) {
        const size_t bytes = size_t(W) * size_t(H) * sizeof(cl_uchar4);
        ensure(ctx, gpu.out_rgb, bytes, CL_MEM_WRITE_ONLY, gpu.out_rgb_bytes);
    }



    class CLBackend final : public Backend {
    public: 
        
        void initialize(const Config& config) override;
        void render(const Camera& cam, const Scene& scene) override;
        // void shutdown() override;

    private:
        // Configuration parameters
        Config config_;

        // OpenCL components
        cl::Platform platform_;
        cl::Device device_;
        cl::Context context_;
        cl::CommandQueue queue_;
        cl::Program program_;

        // OpenCL kernel
        cl::Kernel kernel_;

        // Buffers
        GpuSceneBuffers gpu_scene_;

        // Helper functions for initialization
        void select_platform(int platform_index);
        void select_device(int device_index, cl_device_type type);
        void build_program(const std::vector<std::string>& kernel_sources, const std::string& build_options);


        // Platform and device info
        void print_platform_info();
        void print_device_info();

        // Image saving utility
        void save_image(const std::string& filename, const std::vector<cl_uchar4>& image, int width, int height);

        std::string build_log();

    public:
        BackendType type() const override { return BackendType::OpenCL; }
    };

}

#endif // CLBACKEND_HPP