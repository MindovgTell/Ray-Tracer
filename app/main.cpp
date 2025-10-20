#include "pchray.h"

#include "CLBackend.hpp"


int main() {

    try {

        // Setting up a simple scene and camera for testing
        // Create a scene with some spheres and materials
        Scene scene;

        // Adding spheres to scene
        const size_t obj_num = 100;

        std::vector<Sphere> spheres;
        spheres.reserve(obj_num);

        std::vector<std::shared_ptr<Material>> materials;

        auto mat0 = std::make_shared<Lambertian>( vec3{ 0.6f, 0.5f, 0.2f });
        auto mat1 = std::make_shared<Metal>( vec3{ 0.9f, 0.9f, 0.9f}, 0.0f);
        auto mat2 = std::make_shared<Lambertian>(vec3(0.9f, 0.3f, 0.2f));

        Sphere s5(0.5, point3(-4.0,5.0,-3.0f),  vec3{ 50.0f, 50.0f, 50.0f }, mat2);
        spheres.push_back(s5);
        materials.push_back(mat2);


        Sphere s6(1.0f, point3(0,1.0,-3.0f),  vec3{ 0.0f, 0.0f, 0.0f }, mat1);
        spheres.push_back(s6);
        materials.push_back(mat1);

        for (int a = -5; a < 5; a++) {
                for (int b = -5; b < 5; b++) {
                    auto choose_mat = random_double();
                    point3 center(a + 1.5*random_double(), 0.2, -5.0 + b + 0.9*random_double());

                    if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                        std::shared_ptr<Material> sphere_material;

                        if (choose_mat < 0.3) {
                            // diffuse
                            auto albedo = glm::vec3{random_double(), random_double(), random_double()};
                            sphere_material = std::make_shared<Lambertian>(albedo);
                            spheres.push_back(Sphere(0.3, center, vec3{ 0.0f, 0.0f, 0.0f }, sphere_material));
                            materials.push_back(sphere_material);
                        } else if (choose_mat < 0.95) {
                            // metal
                            auto albedo = glm::vec3{random_double(), random_double(), random_double()};
                            auto fuzz = random_double(0, 0.5);
                            sphere_material = std::make_shared<Metal>(albedo, fuzz);
                            spheres.push_back(Sphere( 0.2, center,vec3{ 0.0f, 0.0f, 0.0f }, sphere_material));
                            materials.push_back(sphere_material);
                        } else {
                            continue;
                            // glass
                            // sphere_material = make_shared<dielectric>(1.5);
                        }
                    }
                }
        }

        auto mat4 = std::make_shared<Lambertian>(vec3(0.5, 0.5f, 0.5f));
        Sphere s4(1000, point3(0,-1000,0),  vec3{ 0.0f, 0.0f, 0.0f }, mat4);
        spheres.push_back(s4);



        scene.set_spheres_vec(spheres);
        scene.set_materials_vec(materials);
        

        // Create a camera
        Camera cam(1920, (16.0/ 9.0));
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