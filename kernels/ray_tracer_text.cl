__constant float EPSILON = 0.00003f; /* required to compensate for limited float precision */
__constant float PI = 3.14159265359f;
__constant int SAMPLES = 200;
__constant int SAMPLES_PER_PIXEL =  20;


typedef struct Camera {
    // Camera settings
    float focal_length;
    float view_port_height;
    float view_port_width;
    float3 center;

    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    float3 viewport_x;
    float3 viewport_y;

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    float3 pixel_delta_x;
    float3 pixel_delta_y;

    // Calculate the position of the upper left pixel.
    float3 viewport_upper_left;
    float3 pixel00_pos;
} Camera;

typedef struct Ray{
	float3 origin;
	float3 direction;
} Ray;

typedef struct Sphere{
	float  radius;
	float3 position;
	float3 color;
	float3 emission;
} Sphere;



static float get_random(unsigned int *seed0, unsigned int *seed1) {

	/* hash the seeds using bitwise AND operations and bitshifts */
	*seed0 = 36969 * ((*seed0) & 65535) + ((*seed0) >> 16);  
	*seed1 = 18000 * ((*seed1) & 65535) + ((*seed1) >> 16);

	unsigned int ires = ((*seed0) << 16) + (*seed1);

	/* use union struct to convert int to float */
	union {
		float f;
		unsigned int ui;
	} res;

	res.ui = (ires & 0x007fffff) | 0x40000000;  /* bitwise AND, bitwise OR */
	return (res.f - 2.0f) / 2.0f;
}


static float3 get_random_unit_vector(){

    return (float3)(0.0f, 0.0f, 0.0f);
}

static inline float2 sample_square(const __private float* fseed,
                                   uint* seed0, uint* seed1) // <— NEW
{
    float jx = get_random(seed0,
               (*seed1)*0x27d4eb2du ^ ((*seed1)*2u+0u)*0x165667b1u ^ (*seed0)*0x9E3779B9u) - 0.5f;
    float jy = get_random(seed1,
               (*seed0)*0x9E3779B9u ^ ((*seed0)*2u+1u)*0x85EBCA6Bu ^ (*seed1)*0x27d4eb2du) - 0.5f;
    return (float2)(jx, jy);
}

static inline Ray create_ray(int x, int y, const Camera* cam, float2 jitter)
{
    Ray r; 
    r.origin = cam->center;
    float3 pixel_sample =
        cam->pixel00_pos +
        ((float)x + 0.5f + jitter.x) * cam->pixel_delta_x +
        ((float)y + 0.5f + jitter.y) * cam->pixel_delta_y;

    r.direction = normalize(pixel_sample - r.origin);  // IMPORTANT
    return r;
}

				/* (__global Sphere* sphere, const Ray* ray) */
float intersect_sphere(const Sphere* sphere, const Ray* ray) /* version using local copy of sphere */
{
	float3 rayToCenter = sphere->position - ray->origin;
	float b = dot(rayToCenter, ray->direction);
	float c = dot(rayToCenter, rayToCenter) - sphere->radius*sphere->radius;
	float disc = b * b - c;

	if (disc < 0.0f) return 0.0f;
	else disc = sqrt(disc);

	if ((b - disc) > EPSILON) return b - disc;
	if ((b + disc) > EPSILON) return b + disc;

	return 0.0f;
}

bool intersect_scene(__constant Sphere* spheres, const Ray* ray, float* t, int* sphere_id, const int sphere_count)
{
	/* initialise t to a very large number, 
	so t will be guaranteed to be smaller
	when a hit with the scene occurs */

	float inf = 1e20f;
	*t = inf;

	/* check if the ray intersects each sphere in the scene */
	for (int i = 0; i < sphere_count; i++)  {
		
		Sphere sphere = spheres[i]; /* create local copy of sphere */
		
		/* float hitdistance = intersect_sphere(&spheres[i], ray); */
		float hitdistance = intersect_sphere(&sphere, ray);
		/* keep track of the closest intersection and hitobject found so far */
		if (hitdistance != 0.0f && hitdistance < *t) {
			*t = hitdistance;
			*sphere_id = i;
		}
	}
	return *t < inf; /* true when ray interesects the scene */
}


/* the path tracing function */
/* computes a path (starting from the camera) with a defined number of bounces, accumulates light/color at each bounce */
/* each ray hitting a surface will be reflected in a random direction (by randomly sampling the hemisphere above the hitpoint) */
/* small optimisation: diffuse ray directions are calculated using cosine weighted importance sampling */

float3 trace(__constant Sphere* spheres, const Ray* camray, const int sphere_count, unsigned int* seed0, unsigned int* seed1){
    Ray ray = *camray;

	float3 accum_color = (float3)(0.0f, 0.0f, 0.0f);
	float3 mask = (float3)(1.0f, 1.0f, 1.0f);

	for (int bounces = 0; bounces < 25; bounces++){

		float t;   /* distance to intersection */
		int hitsphere_id = 0; /* index of intersected sphere */

		/* if ray misses scene, return background colour */
		if (!intersect_scene(spheres, &ray, &t, &hitsphere_id, sphere_count))
		{
            float3 d = normalize(ray.direction);
            float tbg = 0.5f*(d.y + 1.0f);
            float3 sky = mix((float3)(0.2f,0.2f,0.2f), (float3)(0.0f,0.0f,0.0f), tbg);
            return accum_color + mask * sky;
        }

		/* else, we've got a hit! Fetch the closest hit sphere */
		Sphere hitsphere = spheres[hitsphere_id]; /* version with local copy of sphere */

		/* compute the hitpoint using the ray equation */
		float3 hitpoint = ray.origin + ray.direction * t;
		
		/* compute the surface normal and flip it if necessary to face the incoming ray */
		float3 normal = normalize(hitpoint - hitsphere.position); 
		float3 normal_facing = dot(normal, ray.direction) < 0.0f ? normal : normal * (-1.0f);

		/* compute two random numbers to pick a random point on the hemisphere above the hitpoint*/
		uint salt0 = (uint)*seed0 ^ (uint)(bounces*2+0) * 0x9E3779B9u;
        uint salt1 = (uint)*seed1 ^ (uint)(bounces*2+1) * 0x85EBCA6Bu;

        float xi1 = get_random(&salt0, &salt1);      // in [0,1)
        float xi2 = get_random(&salt0, &salt1);     // in [0,1)

        float phi = 2.0f * PI * xi1;
        float r2  = xi2;
        float r2s = sqrt(r2);

		/* create a local orthogonal coordinate frame centered at the hitpoint */
		float3 w = normal_facing;
		float3 axis = fabs(w.x) > 0.1f ? (float3)(0.0f, 1.0f, 0.0f) : (float3)(1.0f, 0.0f, 0.0f);
		float3 u = normalize(cross(axis, w));
		float3 v = cross(w, u);

		/* use the coordinte frame and random numbers to compute the next ray direction */
		float3 newdir = normalize(u * cos(phi)*r2s + v*sin(phi)*r2s + w*sqrt(1.0f - r2));

		/* add a very small offset to the hitpoint to prevent self intersection */
		ray.origin = hitpoint + normal_facing * EPSILON;
		ray.direction = newdir;

		/* add the colour and light contributions to the accumulated colour */
		accum_color += mask * hitsphere.emission; 

		/* the mask colour picks up surface colours at each bounce */
		mask *= hitsphere.color; 
		
		/* perform cosine-weighted importance sampling for diffuse surfaces*/
		mask *= dot(newdir, normal_facing); 
	}

	return accum_color;
}

__kernel void render(int width, int height,
                     __constant Sphere* spheres, int obj_num,
                     float random_seed, __global uchar4* output)
{
    int x = get_global_id(0), y = get_global_id(1);
    if (x >= width || y >= height) return;
    int idx = y*width + x;
    uint seed0 = x;
    uint seed1 = y;

    // Camera (same as yours) ...
    Camera camera;
    camera.focal_length     = 1.0f;
    camera.view_port_height = 1.0f;
    camera.view_port_width  = camera.view_port_height * ((float)width/(float)height);
    camera.center = (float3)(0,0,0);
    camera.viewport_x = (float3)(camera.view_port_width, 0, 0);
    camera.viewport_y = (float3)(0, -camera.view_port_height, 0);
    camera.pixel_delta_x = camera.viewport_x / (float)width;
    camera.pixel_delta_y = camera.viewport_y / (float)height;
    camera.viewport_upper_left = camera.center - (float3)(0,0,camera.focal_length)
                               - camera.viewport_x*0.5f - camera.viewport_y*0.5f;
    camera.pixel00_pos = camera.viewport_upper_left; // we'll add 0.5 in create_ray

    float3 sum = (float3)(0);
    for(int j = 0; j != SAMPLES; j++) {
        for (int s = 0; s < SAMPLES_PER_PIXEL; ++s) {
            float2 jitter = sample_square(&random_seed, &seed0, &seed1);
            Ray camray = create_ray(x, y, &camera, jitter);
            sum += trace(spheres, &camray, obj_num, &seed0, &seed1);
        }
    }

    // 1) Average
    float3 avg = sum / ((float)SAMPLES_PER_PIXEL * (float)SAMPLES);

    // 2) Tonemap (Reinhard) then 3) gamma
    float3 mapped = avg ;/// (1.0f + avg);
    mapped = (float3)(pow(mapped.x, 1.0f/2.2f),
                      pow(mapped.y, 1.0f/2.2f),
                      pow(mapped.z, 1.0f/2.2f));

    output[idx] = (uchar4)(
        (uchar)(clamp(mapped.x, 0.0f, 1.0f) * 255.0f),
        (uchar)(clamp(mapped.y, 0.0f, 1.0f) * 255.0f),
        (uchar)(clamp(mapped.z, 0.0f, 1.0f) * 255.0f),
        (uchar)255);
}

