#include "pchray.h"

#include "CLBackend.hpp"
#include "CLUtils.hpp"

namespace compute {

    void CLBackend::initialize(const Config& config) {
        try {
            config_ = config;
            select_platform(config_.cl.platform_index); // Select the first platform
            print_platform_info();

            select_device(config_.cl.device_index, CL_DEVICE_TYPE_GPU); // Select the first GPU device
            print_device_info();

            context_ = cl::Context(device_);
            queue_ = cl::CommandQueue(context_, device_);

            std::string kernel_dir = clutils::find_directory("kernels");
            std::vector<std::string> src = clutils::read_kernel_sources_from_dir(kernel_dir);

            for (const auto& k : src) {
                if (k.empty()) {
                    throw std::runtime_error("Failed to read kernel: " + k);
                }
            }

            build_program(src, config_.cl.build_options);

        } catch (const cl::Error& e) {
            std::cerr << "OpenCL Error: " << e.what() << " : " << e.err() << "\n";
            throw;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            throw;
        }
    }

    void CLBackend::render(const Camera& cam, const Scene& scene) {
        const size_t W = static_cast<size_t>(cam.get_image_width());
        const size_t H = static_cast<size_t>(cam.get_image_height());
        const size_t N = W * H;

        if (W <= 0 || H <= 0) return;
        if (N > (std::numeric_limits<size_t>::max() / sizeof(cl_uchar4)))
            throw std::runtime_error("Image too large");

        serialize::PackedScene pscene = serialize::pack_scene(scene, cam);
        upload_scene(context_, queue_, pscene, gpu_scene_);
        ensure_output(context_, gpu_scene_, W, H);

        // get real rundom number
        cl_float randomseed = clutils::get_random();

        cl_int s_count = scene.get_spheres_count();
        cl_int m_count = scene.get_materials_count();

        // Kernel: __kernel void render(const int height, const int width, __global uchar4* C)
        kernel_ = cl::Kernel(program_, "render");
        kernel_.setArg(0, W);
        kernel_.setArg(1, H);
        kernel_.setArg(2, gpu_scene_.camera);
        kernel_.setArg(3, gpu_scene_.spheres); 
        kernel_.setArg(4, s_count);
        kernel_.setArg(5, gpu_scene_.materials);
        kernel_.setArg(6, m_count);
        kernel_.setArg(7, randomseed);
        kernel_.setArg(8, gpu_scene_.out_rgb);

        // One work-item per pixel (x = 0..W-1, y = 0..H-1)
        cl::NDRange global(W, H);
        queue_.enqueueNDRangeKernel(kernel_, cl::NullRange, global, cl::NullRange);
        queue_.finish();

        // Read back
        std::vector<cl_uchar4> output(N);
        queue_.enqueueReadBuffer(gpu_scene_.out_rgb, CL_TRUE, 0, N*sizeof(cl_uchar4), output.data());

        // Save / use output (example loop)
        save_image("rednerer4.ppm", output, W, H);

    }

    void CLBackend::select_platform(int platform_index) {
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

    void CLBackend::select_device(int device_index, cl_device_type type = CL_DEVICE_TYPE_ALL) {
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

    void CLBackend::build_program(
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




    void CLBackend::print_platform_info() {
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

    void CLBackend::print_device_info() {
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


    void CLBackend::save_image(const std::string& filename, const std::vector<cl_uchar4>& image, int width, int height) {
        auto image_dir = clutils::find_directory("images");
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