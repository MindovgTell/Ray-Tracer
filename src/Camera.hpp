#ifndef CAMERA_HPP
#define CAMERA_HPP

class Camera {

public: 
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

    double get_defocus_angle() const { return defocus_angle; }
    double get_focus_distance() const { return focus_dist; }

    double get_pixel_samples_scale() const { return pixel_samples_scale; }
    const point3 get_center_point() const { return center; }
    const point3 get_pixel00_location() const { return pixel00_loc; }
    const vec3   get_pixel_delta_u() const { return pixel_delta_u; }
    const vec3   get_pixel_delta_v() const { return pixel_delta_v; }
    const vec3   get_u_vec() const { return u; }
    const vec3   get_v_vec() const { return v; }
    const vec3   get_w_vec() const { return w; }
    const vec3   get_defocus_disk_u() const { return defocus_disk_u; }
    const vec3   get_defocus_disk_v() const { return defocus_disk_v; }

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
    void set_look_from(const point3& p) {
        lookfrom = p;
    }
    void set_look_at(const point3& p) {
        lookat = p;
    }
    void set_vup(const vec3& up) {
        vup = up;
    }

    void set_defocus_angle(double a) {
        defocus_angle = a;
    }
    void set_focus_distance(double fd) {
        focus_dist = fd;
    }

private:
    double aspect_ratio      = 16.0 / 9.0;  // Ratio of image width over height
    int    image_width       = 100;  // Rendered image width in pixel count
    int    samples_per_pixel = 10;
    int    max_depth         = 10;   // Maximum ray bounce depth

    double vfov = 90;  // Vertical view angle (field of view)
    point3 lookfrom = glm::vec3(0,0,0);   // Point camera is looking from
    point3 lookat   = glm::vec3(0,0,-1);  // Point camera is looking at
    vec3 vup      = glm::vec3(0,1,0);   // Camera-relative "up" direction

    double defocus_angle = 0;  // Variation angle of rays through each pixel
    double focus_dist = 10;    // Distance from camera lookfrom point to plane of perfect focus



    int    image_height;   // Rendered image height
    double pixel_samples_scale;  // Color scale factor for a sum of pixel samples
    point3 center;         // Camera center
    point3 pixel00_loc;    // Location of pixel 0, 0
    vec3   pixel_delta_u;  // Offset to pixel to the right
    vec3   pixel_delta_v;  // Offset to pixel below
    vec3   u, v, w;              // Camera frame basis vectors
    vec3   defocus_disk_u;       // Defocus disk horizontal radius
    vec3   defocus_disk_v;       // Defocus disk vertical radius


    void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        center = lookfrom;

        pixel_samples_scale = 1.0 / samples_per_pixel;

        // Determine viewport dimensions.

        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta/2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (double(image_width)/image_height);

    }
};


#endif // CAMERA_HPP