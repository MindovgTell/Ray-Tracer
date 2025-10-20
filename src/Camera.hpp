#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "Utils.hpp"

class Camera {
private:
    // Previously public
    double aspect_ratio      = 1.0;  // Ratio of image width over height
    int    image_width       = 100;  // Rendered image width in pixel count
    int    samples_per_pixel = 10;   // Count of random samples for each pixel
    int    max_depth         = 10;   // Maximum number of ray bounces into scene

    double vfov     = 90.0;              // Vertical view angle (field of view)
    point3 lookfrom = point3(0,1,4);   // Point camera is looking from
    point3 lookat   = point3(0,1,1);  // Point camera is looking at
    vec3   vup      = vec3(0,1,0);     // Camera-relative "up" direction

    double focus_dist = 1.0;    // Distance from camera lookfrom point to plane of perfect focus

    // Previously private
    int    image_height;         // Rendered image height
    double pixel_samples_scale;  // Color scale factor for a sum of pixel samples
    point3 origin;               // Camera center
    point3 pixel00_loc;          // Location of pixel 0, 0
    vec3   pixel_delta_x;        // Offset to pixel to the right
    vec3   pixel_delta_y;        // Offset to pixel below
    vec3   u, v, w;              // Camera frame basis vectors

public: 

    Camera(int width, double aspect_r) : image_width(width), aspect_ratio(aspect_r)  {
        initialize();
    }

    // Getter functions
    double get_aspect_ratio() const { return aspect_ratio; }
    int    get_image_width()  const { return image_width; }
    int    get_image_height() const { return image_height; }
    int    get_samples_per_pixel() const { return samples_per_pixel; }
    int    get_max_depth()    const { return max_depth; }

    double get_vertical_fov() const { return vfov; }
    const point3 get_look_from() const { return lookfrom; }
    const point3 get_look_at()   const { return lookat; }
    const vec3   get_vup_vec()   const { return vup; }

    double get_focal_length() const { return focus_dist; }

    double get_pixel_samples_scale() const { return pixel_samples_scale; }
    const point3 get_origin() const { return origin; }
    const point3 get_pixel00_location() const { return pixel00_loc; }
    const vec3   get_pixel_delta_x() const { return pixel_delta_x; }
    const vec3   get_pixel_delta_y() const { return pixel_delta_y; }
    const vec3   get_u_vec() const { return u; }
    const vec3   get_v_vec() const { return v; }
    const vec3   get_w_vec() const { return w; }

    // Setter functions (do not call initialize() here; use recompute() when needed)
    void set_aspect_ratio(double a) {
        aspect_ratio = a;
    }
    void set_image_width(int w) {
        image_width = (w > 0) ? w : 1;
    }
    void set_samples_per_pixel(int s) {
        samples_per_pixel = (s > 0) ? s : 1;
    }
    void set_max_depth(int d) {
        max_depth = d;
    }

    void set_vertical_fov(double f) {
        vfov = f;
    }
    void set_look_from(point3 p) {
        lookfrom = p;
    }
    void set_look_at(point3 p) {
        lookat = p;
    }
    void set_vup(vec3 up) {
        vup = up;
    }

    void set_focus_dist(double fd) {
        focus_dist = fd;
    }

    void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        origin = lookfrom;

        pixel_samples_scale = 1.0 / samples_per_pixel;

        // Determine viewport dimensions.

        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta/2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (double(image_width)/image_height);

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        
        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewport_x = viewport_width * u;    // Vector across viewport horizontal edge
        vec3 viewport_y = viewport_height * -v;  // Vector down viewport vertical edge


        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_x = viewport_x / image_width;
        pixel_delta_y = viewport_y / image_height;

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left = origin - (focus_dist * w) - viewport_x/2 - viewport_y/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_x + pixel_delta_y);

        // // Calculate the camera defocus disk basis vectors.
        // auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        // defocus_disk_u = u * defocus_radius;
        // defocus_disk_v = v * defocus_radius;
    }

};


#endif // CAMERA_HPP