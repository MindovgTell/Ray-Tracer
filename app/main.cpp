#include "pchray.h"

#include "CLBackend.hpp"

#include <chrono>


void setup_scene(Scene& scene);

int main() {

    try {

        // Setting up a simple scene and camera for testing
        // Create a scene with some spheres and materials
        Scene scene;

        setup_scene(scene);
        

        // Create a camera
        Camera cam(1440, (16.0 / 9.0));
        cam.set_look_at(point3(0.7,1.3,3));
        cam.set_look_from(point3(0,2,6));
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
        auto start = std::chrono::high_resolution_clock::now();
        backend->render(cam, scene);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Rendering completed in " << elapsed.count() << " seconds.\n";
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



void setup_scene(Scene& scene) {
    // Add spheres, materials, and other objects to the scene
    const size_t obj_num = 155;

    std::vector<Sphere> spheres;
    spheres.reserve(obj_num);

    std::vector<std::shared_ptr<Material>> materials;
    materials.reserve(obj_num);

    auto mat1 = std::make_shared<Metal>( vec3{ 0.7f, 0.9f, 0.9f}, 0.0f);
    auto mat2 = std::make_shared<Lambertian>(vec3(0.9f, 0.3f, 0.2f));
    auto mat3 = std::make_shared<Metal>( vec3{ 0.8f, 0.6f, 0.2f}, 0.3f);

    float ref = 1.05f;

    auto mat41 = std::make_shared<Dielectric>(ref);
    auto mat42 = std::make_shared<Dielectric>(1.0/ref);

    Sphere MetalSph(1.0f, point3(4,1.0,0.0f),  vec3{ 0.0f, 0.0f, 0.0f }, mat1);
    spheres.push_back(MetalSph);
    materials.push_back(mat1);

    Sphere light_source(0.5, point3(4.0,5.0,-3.0f),  vec3{ 50.0f, 50.0f, 50.0f }, mat2);
    spheres.push_back(light_source);
    materials.push_back(mat2);

    Sphere LamSph(1, point3(2,1.0,-1.0f),  vec3{ 0.0f, 0.0f, 0.0f }, mat2);
    spheres.push_back(LamSph);
    materials.push_back(mat2);

    Sphere GlassSph1(1.0, point3(0, 1.0, 0.0),  vec3{ 0.0f, 0.0f, 0.0f}, mat41);
    spheres.push_back(GlassSph1);
    materials.push_back(mat41);

    Sphere GlassSph2(0.95, point3(0.0, 1.0, 0.0),  vec3{ 0.0f, 0.0f, 0.0f}, mat42);
    spheres.push_back(GlassSph2);
    materials.push_back(mat42);

    for (int a = -6; a < 6; a++) {
            for (int b = -6; b < 6; b++) {
                auto choose_mat = random_double();
                point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());
                // Skip spheres in the specified region
                if (center.x >= -1.3 && center.x <= 3.3 && center.z >= -1.0 && center.z <= 1.0) {
                    continue;
                }

                if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                    std::shared_ptr<Material> sphere_material;

                    if (choose_mat < 0.65) {
                        // diffuse
                        auto albedo = glm::vec3{random_double(), random_double(), random_double()};
                        sphere_material = std::make_shared<Lambertian>(albedo);
                        spheres.push_back(Sphere(0.3, center, vec3{ 0.0f, 0.0f, 0.0f }, sphere_material));
                        materials.push_back(sphere_material);
                    } else if (choose_mat < 0.85) {
                        // metal
                        auto albedo = glm::vec3{random_double(), random_double(), random_double()};
                        auto fuzz = random_double(0, 0.5);
                        sphere_material = std::make_shared<Metal>(albedo, fuzz);
                        spheres.push_back(Sphere( 0.3, center,vec3{ 0.0f, 0.0f, 0.0f }, sphere_material));
                        materials.push_back(sphere_material);
                    } else {
                    // glass
                        sphere_material = std::make_shared<Dielectric>(1.5);
                        spheres.push_back(Sphere( 0.4, center,vec3{ 0.0f, 0.0f, 0.0f }, sphere_material));
                        materials.push_back(sphere_material);}
                }
            }
    }



    auto mat5 = std::make_shared<Lambertian>(vec3(0.5, 0.5f, 0.5f));
    Sphere Ground(1000, point3(0,-1000,0),  vec3{ 0.0f, 0.0f, 0.0f }, mat5);
    spheres.push_back(Ground);



    scene.set_spheres_vec(spheres);
    scene.set_materials_vec(materials);
}