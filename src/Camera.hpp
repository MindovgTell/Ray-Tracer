#ifndef CAMERA_HPP
#define CAMERA_HPP

class Camera {



private:
    // Previously public
    double aspect_ratio      = 1.0;  // Ratio of image width over height
    int    image_width       = 100;  // Rendered image width in pixel count
    int    samples_per_pixel = 10;   // Count of random samples for each pixel
    int    max_depth         = 10;   // Maximum number of ray bounces into scene

    double vfov     = 90.0;              // Vertical view angle (field of view)
    point3 lookfrom = point3(0,0,0);   // Point camera is looking from
    point3 lookat   = point3(0,0,-1);  // Point camera is looking at
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




    inline vec3 unit_vector(const vec3& v) {
        return {v.x / v.length(), v.y / v.length(), v.z / v.length()};
    }


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

        pixel_samples_scale = 1.0 / samples_per_pixel;

        origin = lookfrom;

        // Determine viewport dimensions.
        auto focal_length = (lookfrom - lookat).length();
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta/2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (double(image_width)/image_height);

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewport_x = u;
        viewport_x *= viewport_width;    // Vector across viewport horizontal edge
        vec3 viewport_y = -v;  // Vector down viewport vertical edge
        viewport_y *= viewport_height;

        // Calculate the horizontal and vertical delta vectors to the next pixel.
        pixel_delta_x = viewport_x;
        pixel_delta_x /= image_width;
        pixel_delta_y = viewport_y;
        pixel_delta_y /= image_height;

        // Calculate the location of the upper left pixel.
        auto fw = w;
        fw *= focus_dist;
        auto vx2 = viewport_x;
        vx2 /= 2;
        auto vy2 = viewport_y;
        vy2 /= 2;
        auto viewport_upper_left = origin - fw - vx2 - vy2;
        auto pixel_delta = (pixel_delta_x + pixel_delta_y);
        pixel_delta *= 0.5;
        pixel00_loc = viewport_upper_left + pixel_delta;
    }

};


#endif // CAMERA_HPP