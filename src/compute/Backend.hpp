#ifndef BACKEND_HPP
#define BACKEND_HPP

namespace compute {

    enum class BackendType {
        OpenCL,
        Metal,
        Vulkan,
        CUDA
    };

    struct Config {
        int width = 800;
        int height = 600;
        int samples_per_pixel = 100;
        int max_depth = 50;

        struct OpenCl {
        int platform_index = 0;
        int device_index   = 0;
        std::string build_options; // e.g. "-cl-std=CL1.2 -cl-fast-relaxed-math"
        } cl;
    };


    class Backend {
    public: 
        virtual ~Backend() = default;
        virtual BackendType type() const = 0;

        virtual void initialize(const Config& config) = 0;
        virtual void render(const Camera& cam, const Scene& scene) = 0;
        // virtual void shutdown() = 0;

    };

    std::unique_ptr<Backend> CreateBackend(BackendType type);
}


#endif // BACKEND_HPP