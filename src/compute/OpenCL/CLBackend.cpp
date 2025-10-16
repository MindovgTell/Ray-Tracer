#include "pchray.h"

#include "CLBackend.hpp"
#include "CLUtils.hpp"

namespace compute {

    void CLBackend::initialize(const Config& config) {
        try {
            config_ = config;
            selectPlatform(config_.cl.platform_index); // Select the first platform
            printPlatformInfo();

            selectDevice(config_.cl.device_index, CL_DEVICE_TYPE_GPU); // Select the first GPU device
            printDeviceInfo();

            context_ = cl::Context(device_);
            queue_ = cl::CommandQueue(context_, device_);

            std::string kernel_dir = clutils::findDirectory("kernels");
            std::vector<std::string> src = clutils::readKernelSourcesFromDir(kernel_dir);

            for (const auto& k : src) {
                if (k.empty()) {
                    throw std::runtime_error("Failed to read kernel: " + k);
                }
            }

            buildProgram(src, config_.cl.build_options);

        } catch (const cl::Error& e) {
            std::cerr << "OpenCL Error: " << e.what() << " : " << e.err() << "\n";
            throw;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            throw;
        }
    }

    void CLBackend::render(const Camera& cam, const Scene& scene) {
        const size_t W = static_cast<size_t>(config_.width);
        const size_t H = static_cast<size_t>(config_.height);
        const size_t N = W * H;

        if (W <= 0 || H <= 0) return;
        if (N > (std::numeric_limits<size_t>::max() / sizeof(cl_uchar4)))
            throw std::runtime_error("Image too large");

        const size_t bytes = N * sizeof(cl_uchar4);

        const size_t obj_num = 5;
        serialize::SphereGPU cpu_spheres[obj_num];
        for (size_t i = 0; i < obj_num; ++i) {
            cpu_spheres[i].radius   = 1.0f;
            cpu_spheres[i].position = { -2.5f + i*2.5f, -1.0f, -6.0f }; // much closer
            cpu_spheres[i].color    = { 0.9f - 0.3f*i, 0.2f + 0.3f*i, 0.2f };
            cpu_spheres[i].emission = { 0.0f, 0.0f, 0.0f }; 
        }

        cpu_spheres[3].radius   = 0.5f;
        cpu_spheres[3].position = { 0.0f, 2.0f, -5.0f }; // much closer
        cpu_spheres[3].color    = { 0.5f, 0.5f, 1.0f };
        cpu_spheres[3].emission = { 50.0f, 50.0f, 50.0f }; 

        cpu_spheres[4].radius   = 98.0f;
        cpu_spheres[4].position = { 0.0f, -100.0f, -6.0f }; // much closer
        cpu_spheres[4].color    = { 0.3f, 0.3f, 0.9f };
        cpu_spheres[4].emission = { 0.0f, 0.0f, 0.0f }; 

        // Serialize scene objects to GPU format
        cl::Buffer cl_output(context_, CL_MEM_WRITE_ONLY, N * sizeof(cl_uchar4));
        cl::Buffer cl_spheres(context_, CL_MEM_READ_ONLY, obj_num * sizeof(serialize::SphereGPU));
        queue_.enqueueWriteBuffer(cl_spheres, CL_TRUE, 0, obj_num * sizeof(serialize::SphereGPU), cpu_spheres);

        // get real rundom number
        cl_float randomseed = clutils::get_random();

        // Kernel: __kernel void render(const int height, const int width, __global uchar4* C)
        kernel_ = cl::Kernel(program_, "render");
        kernel_.setArg(0, static_cast<int>(W));
        kernel_.setArg(1, static_cast<int>(H));
        kernel_.setArg(2, cl_spheres);
        kernel_.setArg(3, static_cast<int>(obj_num));
        kernel_.setArg(4, randomseed);
        kernel_.setArg(5, cl_output);

        // One work-item per pixel (x = 0..W-1, y = 0..H-1)
        cl::NDRange global(W, H);
        queue_.enqueueNDRangeKernel(kernel_, cl::NullRange, global, cl::NullRange);
        queue_.finish();

        // Read back
        std::vector<cl_uchar4> output(N);
        queue_.enqueueReadBuffer(cl_output, CL_TRUE, 0, bytes, output.data());

        // Save / use output (example loop)
        saveImage("rednerer2.ppm", output, W, H);

    }

    void CLBackend::selectPlatform(int platform_index) {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        if (platforms.empty()) {
            throw std::runtime_error("No OpenCL platforms found.");
        }
        if (platform_index < 0 || platform_index >= static_cast<int>(platforms.size())) {
            throw std::runtime_error("Invalid platform index.");
        }
        platform_ = platforms[platform_index];
    }

    void CLBackend::selectDevice(int device_index, cl_device_type type = CL_DEVICE_TYPE_ALL) {
        if (!platform_()) {
            throw std::runtime_error("Platform not selected.");
        }
        std::vector<cl::Device> devices;
        platform_.getDevices(type, &devices);
        if (devices.empty()) {
            throw std::runtime_error("No OpenCL devices found for the selected platform.");
        }
        if (device_index < 0 || device_index >= static_cast<int>(devices.size())) {
            throw std::runtime_error("Invalid device index.");
        }
        device_ = devices[device_index];
    }

    std::string CLBackend::build_log() {
        return program_.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device_);
    }

    void CLBackend::buildProgram(
                              const std::vector<std::string>& kernel_sources, 
                              const std::string& build_options = "") {
        cl::Program::Sources sources;
        for (const auto& src : kernel_sources) {
            sources.push_back({ src.c_str(), src.length() });
        }
        cl::Program program(context_, sources);
        cl_int err = program.build({device_}, build_options.c_str());
        if (err != CL_SUCCESS) {
            std::cerr << "Build log:\n" << build_log() << "\n";
            throw std::runtime_error("No OpenCL platforms");
        }
        program_ = program;
    }




    void CLBackend::printPlatformInfo() {
        if (!platform_()) {
            throw std::runtime_error("Invalid platform.");
        }
        std::cout << "\n//=========== OpenCL Platform Info ===========\n";
        std::cout << "|| Platform: " << platform_.getInfo<CL_PLATFORM_NAME>() << "\n";
        std::cout << "|| Vendor: " << platform_.getInfo<CL_PLATFORM_VENDOR>() << "\n";
        std::cout << "|| Version: " << platform_.getInfo<CL_PLATFORM_VERSION>() << "\n";
        std::cout << "|| Profile: " << platform_.getInfo<CL_PLATFORM_PROFILE>() << "\n";
        std::cout << "|| Extensions:\n " << platform_.getInfo<CL_PLATFORM_EXTENSIONS>() << "\n";
        std::cout << "//============================================\n" << std::endl;
    }

    void CLBackend::printDeviceInfo() {
        if (!device_()) {
            throw std::runtime_error("Invalid device.");
        }
        std::cout << "//=========== OpenCL Device Info =============\n";
        std::cout << "|| Device: " << device_.getInfo<CL_DEVICE_NAME>() << "\n";
        std::cout << "|| Type: " << (device_.getInfo<CL_DEVICE_TYPE>() & CL_DEVICE_TYPE_GPU ? "GPU" :
                                      device_.getInfo<CL_DEVICE_TYPE>() & CL_DEVICE_TYPE_CPU ? "CPU" :
                                      device_.getInfo<CL_DEVICE_TYPE>() & CL_DEVICE_TYPE_ACCELERATOR ? "ACCELERATOR" : "OTHER") << "\n";
        std::cout << "|| Vendor: " << device_.getInfo<CL_DEVICE_VENDOR>() << "\n";
        std::cout << "|| Version: " << device_.getInfo<CL_DEVICE_VERSION>() << "\n";
        std::cout << "|| Driver version: " << device_.getInfo<CL_DRIVER_VERSION>() << "\n";
        std::cout << "|| Max compute units: " << device_.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << "\n";
        std::cout << "|| Max work group size: " << device_.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << "\n";
        std::cout << "|| Global memory size: " << device_.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / (1024 * 1024) << " MB\n";
        std::cout << "|| Local memory size: " << device_.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() / 1024 << " KB\n";
        std::cout << "|| Max memory allocation size: " << device_.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() / (1024 * 1024) << " MB\n";
        std::cout << "//============================================\n" << std::endl;
    }


    void CLBackend::saveImage(const std::string& filename, const std::vector<cl_uchar4>& image, int width, int height) {
        auto image_dir = clutils::findDirectory("images");
        std::filesystem::path filepath = image_dir / filename;
        std::ofstream ofs(filepath);
        if (!ofs) {
            throw std::runtime_error("Failed to open file for writing: " + filepath.string());
        }

        ofs << "P6\n" << width << " " << height << "\n255\n";

        // write RGB bytes
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const cl_uchar4& p = image[y * width + x];
                unsigned char rgb[3] = { p.s[0], p.s[1], p.s[2] };
                ofs.write(reinterpret_cast<char*>(rgb), 3);
            }
        }
        ofs.close();
        std::cout << "Image saved to " << filepath << "\n";
    }
}