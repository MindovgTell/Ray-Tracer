#ifndef CLUTILS_HPP
#define CLUTILS_HPP


namespace compute::clutils {

    inline cl_float get_random() {
        static std::uniform_real_distribution<double> distribution(0.0, 1.0);
        static std::mt19937 generator;
        return static_cast<cl_float>(distribution(generator));
    }

    inline cl_float get_random(float min, float max) {
        return min + (max-min)*random_double();
    }

    inline std::vector<cl::Platform> get_platforms() {

        /*
        *   Function for getting all available OpenCL platforms.
        *   Throws runtime_error if no platforms found.
        *   Returns vector of cl::Platform objects.
        */

        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        if (platforms.empty()) {
            std::cerr << "No OpenCL platforms\n"; 
            throw std::runtime_error("No OpenCL platforms found.");
        }
        return platforms;
    }

    inline std::vector<cl::Device> get_devices(const cl::Platform& platform, cl_device_type type = CL_DEVICE_TYPE_ALL) {

        /*
        *   Function for getting all available OpenCL devices of a given type from a platform.
        *   Throws runtime_error if no devices found.
        *   Returns vector of cl::Device objects.
        */

        std::vector<cl::Device> devices;
        platform.getDevices(type, &devices);
        if (devices.empty()) {
            std::cerr << "No OpenCL devices\n"; 
            throw std::runtime_error("No OpenCL devices found for the specified type.");
        }
        return devices;
    }

    

    inline void print_platforms_info(const std::vector<cl::Platform>& platforms) {
    std::cout << "Platforms: " << platforms.size() << "\n";
        for (size_t i = 0; i < platforms.size(); ++i) {
            const auto& p = platforms[i];
            std::cout << "[" << i << "] "
                      << p.getInfo<CL_PLATFORM_NAME>()
                      << " | " << p.getInfo<CL_PLATFORM_VENDOR>() << "\n";

            std::vector<cl::Device> devices;
            p.getDevices(CL_DEVICE_TYPE_ALL, &devices);
            if (devices.empty()) {
                std::cout << "  (no available device)\n";
                continue;
            }

            for (size_t j = 0; j < devices.size(); ++j) {
                const auto& d = devices[j];
                auto name        = d.getInfo<CL_DEVICE_NAME>();
                auto ver         = d.getInfo<CL_DEVICE_VERSION>();
                auto type        = d.getInfo<CL_DEVICE_TYPE>();
                auto units       = d.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
                auto extensions  = d.getInfo<CL_DEVICE_EXTENSIONS>();

                const char* type_str =
                    (type & CL_DEVICE_TYPE_GPU) ? "GPU" :
                    (type & CL_DEVICE_TYPE_CPU) ? "CPU" :
                    (type & CL_DEVICE_TYPE_ACCELERATOR) ? "ACCEL" : "OTHER";

                std::cout << "  (" << j << ") " << name
                          << "  [" << type_str << "]  CU=" << units
                          << "  {" << ver << "}\n"
                          << "  Device extensions:   \n" << extensions << "\n";
            }
        }
    }


    inline std::string device_type_to_str(cl_device_type type) {
        if (type & CL_DEVICE_TYPE_GPU) return "GPU";
        if (type & CL_DEVICE_TYPE_CPU) return "CPU";
        if (type & CL_DEVICE_TYPE_ACCELERATOR) return "ACCELERATOR";
        return "UNKNOWN";
    }

    inline void print_device_info(const cl::Device& device) {
        std::cout << "Device: " << device.getInfo<CL_DEVICE_NAME>() << "\n";
        std::cout << "  Type: " << device_type_to_str(device.getInfo<CL_DEVICE_TYPE>()) << "\n";
        std::cout << "  Vendor: " << device.getInfo<CL_DEVICE_VENDOR>() << "\n";
        std::cout << "  Version: " << device.getInfo<CL_DEVICE_VERSION>() << "\n";
        std::cout << "  Driver version: " << device.getInfo<CL_DRIVER_VERSION>() << "\n";
        std::cout << "  Max compute units: " << device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << "\n";
        std::cout << "  Max work group size: " << device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << "\n";
        std::cout << "  Global memory size: " << device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / (1024 * 1024) << " MB\n";
        std::cout << "  Local memory size: " << device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() / 1024 << " KB\n";
        std::cout << "  Max memory allocation size: " << device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() / (1024 * 1024) << " MB\n";
    }

    inline std::string build_log(const cl::Program& program, const cl::Device& device) {
        return program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
    }

    inline cl::Program BuildProgram( cl::Context& context ,
                              const cl::Device& device, 
                              const std::vector<const std::string>& kernel_sources, 
                              const std::string build_options = "") {
        cl::Program::Sources sources;
        for (const auto& src : kernel_sources) {
            sources.push_back({ src.c_str(), src.length() });
        }
        cl::Program program(context, sources);
        cl_int err = program.build({device}, build_options.c_str());
        if (err != CL_SUCCESS) {
            std::cerr << "Build error:\n" << build_log(program, device) << "\n";
            throw std::runtime_error("No OpenCL platforms");
        }
        return program;
    }



    inline std::filesystem::path find_directory(std::string dir_name = "kernels") {
        const std::filesystem::path start = std::filesystem::current_path();

        // Walk up: start, parent, grandparent, ... until root
        for (std::filesystem::path cur = start; !cur.empty(); cur = cur.parent_path()) {
            std::filesystem::path candidate = cur / dir_name;
            if (std::filesystem::exists(candidate) && std::filesystem::is_directory(candidate)) {
                return std::filesystem::canonical(candidate);
            }
        }
        throw std::runtime_error(
            "Couldn't find '" + dir_name + "' directory starting from: " + start.string());
    }

    inline std::string read_file_to_string(const std::filesystem::path& p) {
        std::ifstream ifs(p, std::ios::binary);
        if (!ifs) throw std::runtime_error("Failed to open kernel file: " + p.string());
        std::ostringstream ss;
        ss << ifs.rdbuf();
        return ss.str();
    }

    // Gather all *.cl files from a directory into a vector<string> (sorted deterministically)
    inline std::vector<std::string> read_kernel_sources_from_dir(const std::filesystem::path& dir) {
        if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
            throw std::runtime_error("Kernel directory not found: " + dir.string());
        }

        std::vector<std::filesystem::path> files;
        for (auto& e : std::filesystem::directory_iterator(dir)) {
            if (!e.is_regular_file()) continue;
            auto ext = e.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".cl") files.push_back(e.path());
        }

        if (files.empty()) {
            throw std::runtime_error("No .cl files found in: " + dir.string());
        }

        std::sort(files.begin(), files.end()); // stable, deterministic order (by filename)

        std::vector<std::string> sources;
        sources.reserve(files.size());
        for (const auto& f : files) {
            sources.push_back(read_file_to_string(f));
        }
        return sources;
    }

}

#endif // CLUTILS_HPP