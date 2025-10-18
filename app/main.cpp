#include "pchray.h"

#include "CLBackend.hpp"


int main() {

    try {

        // Setting up a simple scene and camera for testing
        // Create a scene with some spheres and materials
        Scene scene;

        // Adding spheres to scene
        const size_t obj_num = 5;

        std::vector<Sphere> spheres(obj_num);

        for (int i = 0; i != 2; i++) {
            spheres[i].set_radius(1.0f);
            spheres[i].set_center_pos(vec3{-2.5f + i*2.5f, -1.0f, -6.0f }); // much closer
            spheres[i].set_color(vec3{ 0.9f - 0.3f*i, 0.2f + 0.3f*i, 0.2f });
            spheres[i].set_emission(vec3{ 0.0f, 0.0f, 0.0f }); 
        }

        spheres[3].set_radius(0.5f);
        spheres[3].set_center_pos(vec3{ 0.0f, 2.0f, -5.0f }); // much closer
        spheres[3].set_color(vec3{ 0.5f, 0.5f, 1.0f });
        spheres[3].set_emission(vec3{ 50.0f, 50.0f, 50.0f }); 

        spheres[4].set_radius(98.0f);
        spheres[4].set_center_pos(vec3{ 0.0f, -100.0f, -6.0f }); // much closer
        spheres[4].set_color(vec3{ 0.3f, 0.3f, 0.9f });
        spheres[4].set_emission(vec3{ 0.0f, 0.0f, 0.0f }); 


        scene.set_spheres_vec(spheres);


        // Create a camera
        Camera cam(1440, (16.0/ 9.0));
        cam.set_samples_per_pixel(20);
        cam.set_max_depth(50);

        cam.initialize();
    

        // Configure the backend
        compute::Config config;
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