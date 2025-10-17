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




    // hittable_list world;

    // auto ground_material = std::make_shared<Lambertian>(color(0.5, 0.5, 0.5));
    // world.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    // for (int a = -11; a < 11; a++) {
    //     for (int b = -11; b < 11; b++) {
    //         auto choose_mat = random_double();
    //         point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

    //         if ((center - point3(4, 0.2, 0)).length() > 0.9) {
    //             std::shared_ptr<Material> sphere_material;

    //             if (choose_mat < 0.8) {
    //                 // diffuse
    //                 auto albedo = color::random() * color::random();
    //                 sphere_material = std::make_shared<Lambertian>(albedo);
    //                 world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
    //             } else if (choose_mat < 0.95) {
    //                 // metal
    //                 auto albedo = color::random(0.5, 1);
    //                 auto fuzz = random_double(0, 0.5);
    //                 sphere_material = std::make_shared<Metal>(albedo, fuzz);
    //                 world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
    //             } else {
    //                 // glass
    //                 sphere_material = std::make_shared<Dielectric>(1.5);
    //                 world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
    //             }
    //         }
    //     }
    // }

    // auto material1 = std::make_shared<Dielectric>(1.5);
    // world.add(std::make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    // auto material2 = std::make_shared<Lambertian>(color(0.4, 0.2, 0.1));
    // world.add(std::make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    // auto material3 = std::make_shared<Metal>(color(0.7, 0.6, 0.5), 0.0);
    // world.add(std::make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    // camera cam;

    // cam.aspect_ratio      = 16.0 / 9.0;
    // cam.image_width       = 400;
    // cam.samples_per_pixel = 200;
    // cam.max_depth         = 20;

    // cam.vfov     = 20;
    // cam.lookfrom = point3(13,2,3);
    // cam.lookat   = point3(0,0,0);
    // cam.vup      = vec3(0,1,0);

    // cam.defocus_angle = 0.6;
    // cam.focus_dist    = 10.0;

    // cam.render(world);