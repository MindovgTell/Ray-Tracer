#include "pchray.h"

#include "CLBackend.hpp"


int main() {

    try {

        // Setting up a simple scene and camera for testing
        // Create a scene with some spheres and materials
        Scene scene;

        // Create a camera
        Camera cam;
        cam.aspect_ratio      = 16.0 / 9.0;
        cam.image_width       = 1440;
        cam.samples_per_pixel = 20;
        cam.max_depth         = 50;

        cam.vfov     = 20;
        cam.lookfrom = glm::vec3(13,2,3);
        cam.lookat   = glm::vec3(0,0,0);
        cam.vup      = glm::vec3(0,1,0);

    

        // Configure the backend
        compute::Config config;
        config.width = 1440;
        config.height = 1080;
        config.samples_per_pixel = 100;
        config.max_depth = 50;
        config.cl.platform_index = 0;
        config.cl.device_index = 0;
        config.cl.build_options = "-cl-std=CL1.2 -cl-fast-relaxed-math";
        
        // Create and initialize the backend
        std::unique_ptr<compute::Backend> backend = compute::CreateBackend(compute::BackendType::OpenCL);
        backend->initialize(config);
        // Render the scene using the backend
        backend->render(cam, scene);
        // backend->shutdown();
        

    }
    catch(const cl::Error& e) {
        std::cerr << "OpenCL error: " << e.what() << " (" << e.err() << ")\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}