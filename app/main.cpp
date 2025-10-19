#include "pchray.h"

#include "CLBackend.hpp"


int main() {

    try {

        // Setting up a simple scene and camera for testing
        // Create a scene with some spheres and materials
        Scene scene;

        // Adding spheres to scene
        const size_t obj_num = 5;

        std::vector<Sphere> spheres;
        spheres.reserve(obj_num);

        std::vector<std::shared_ptr<Material>> materials;

        auto mat0 = std::make_shared<Lambertian>( vec3{ 0.6f, 0.5f, 0.2f });
        auto mat1 = std::make_shared<Metal>( vec3{ 0.9f, 0.9f, 0.9f}, 0.2f);
        auto mat2 = std::make_shared<Lambertian>(vec3(0.5f, 0.5f, 0.0f));
        auto mat3 = std::make_shared<Lambertian>(vec3(1.0f, 1.0f, 1.0f));
        auto mat4 = std::make_shared<Lambertian>(vec3(0.7, 0.5f, 0.3f));


        materials.push_back(mat0);
        materials.push_back(mat1);
        materials.push_back(mat2);
        materials.push_back(mat3);
        materials.push_back(mat4);

        Sphere s0(1.0f, vec3{-2.5f, -1.0f, -8.0f }, vec3{ 0.0f, 0.0f, 0.0f }, mat0);
        spheres.push_back(s0);

        Sphere s1(1.0f, vec3{0.0f, -1.0f, -8.0f }, vec3{ 0.0f, 0.0f, 0.0f }, mat1);
        spheres.push_back(s1);

        Sphere s2(1.0f, vec3{2.5f, -1.0f, -8.0f }, vec3{ 0.0f, 0.0f, 0.0f }, mat2);
        spheres.push_back(s2);


        Sphere s3(0.5f, vec3{0.0f, 2.0f, -7.0f }, vec3{ 50.0f, 50.0f, 50.0f }, mat3);
        spheres.push_back(s3);

        Sphere s4(98.0f, vec3{ 0.0f, -100.0f, -8.0f }, vec3{ 0.0f, 0.0f, 0.0f }, mat4);
        spheres.push_back(s4);



        scene.set_spheres_vec(spheres);
        scene.set_materials_vec(materials);
        

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