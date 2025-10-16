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
        cl::Buffer cameraBuffer_;
        cl::Buffer sceneBuffer_;
        cl::Buffer outputBuffer_;

        // Helper functions for initialization
        void selectPlatform(int platform_index);
        void selectDevice(int device_index, cl_device_type type);
        void buildProgram(const std::vector<std::string>& kernel_sources, const std::string& build_options);


        // Platform and device info
        void printPlatformInfo();
        void printDeviceInfo();

        // Image saving utility
        void saveImage(const std::string& filename, const std::vector<cl_uchar4>& image, int width, int height);

        std::string build_log();

    public:
        BackendType type() const override { return BackendType::OpenCL; }
    };

}

#endif // CLBACKEND_HPP