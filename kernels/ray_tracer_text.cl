__constant float EPSILON = 0.00003f; /* required to compensate for limited float precision */
__constant float PI = 3.14159265359f;
__constant int SAMPLES = 200;
__constant int SAMPLES_PER_PIXEL =  20;


typedef struct Camera {
    // Camera settings
    float4 origin; // float4(origin.x, origin.y, origin.z, focal_length) the 4th argument is focal_length
   
    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    float4 pixel_delta_x;
    float4 pixel_delta_y;

    float4 pixel00_pos;
} Camera;

typedef struct Material{
	int    tag; int _pad0,_pad1,_pad2;
	float4 albedo;
	float  fuzz, ref_idx; float _p2a,_p2b;
} Material;

typedef struct Ray{
	float4 origin;
	float4 direction;
} Ray;

typedef struct Sphere{
	float4 position; // position + radius (float4)(pos.x, pos.y, pos.z, radius)
	float4 color;
	float4 emission;
	int material_index;
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
    r.origin = cam->origin;
    float4 pixel_sample =
        cam->pixel00_pos +
        ((float)x + 0.5f + jitter.x) * cam->pixel_delta_x +
        ((float)y + 0.5f + jitter.y) * cam->pixel_delta_y;

    r.direction = normalize(pixel_sample - r.origin);  // IMPORTANT
    return r;
}

				/* (__global Sphere* sphere, const Ray* ray) */
float intersect_sphere(const Sphere* sphere, const Ray* ray) /* version using local copy of sphere */
{
	float3 rayToCenter = (float3)((sphere->position.xyz - ray->origin.xyz));
	float b = dot(rayToCenter, (float3)(ray->direction.xyz ));
	float c = dot(rayToCenter, rayToCenter) - (sphere->position.w)*(sphere->position.w);
	float disc = b * b - c;

	if (disc < 0.0f) return 0.0f;
	else disc = sqrt(disc);

	if ((b - disc) > EPSILON) return b - disc;
	if ((b + disc) > EPSILON) return b + disc;

	return 0.0f;
}

bool intersect_scene(__global const Sphere* spheres, const Ray* ray, float* t, int* sphere_id, const int sphere_count)
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

float3 trace(__global const Sphere* spheres, const Ray* camray, const int sphere_count, unsigned int* seed0, unsigned int* seed1){
    Ray ray = *camray;

	float3 accum_color = (float3)(0.0f, 0.0f, 0.0f);
	float3 mask = (float3)(1.0f, 1.0f, 1.0f);

	for (int bounces = 0; bounces < 25; bounces++){

		float t;   /* distance to intersection */
		int hitsphere_id = 0; /* index of intersected sphere */

		/* if ray misses scene, return background colour */
		if (!intersect_scene(spheres, &ray, &t, &hitsphere_id, sphere_count))
		{
            float3 d = normalize((float3)(ray.direction.xyz));
            float tbg = 0.5f*(d.y + 1.0f);
            float3 sky = mix((float3)(0.2f,0.2f,0.2f), (float3)(0.0f,0.0f,0.0f), tbg);
            return accum_color + mask * sky;
        }

		/* else, we've got a hit! Fetch the closest hit sphere */
		Sphere hitsphere = spheres[hitsphere_id]; /* version with local copy of sphere */

		/* compute the hitpoint using the ray equation */
		float3 hitpoint = ray.origin.xyz + ray.direction.xyz * t;
		
		/* compute the surface normal and flip it if necessary to face the incoming ray */
		float3 normal = normalize(hitpoint - hitsphere.position.xyz); 
		float3 normal_facing = dot(normal, ray.direction.xyz) < 0.0f ? normal : normal * (-1.0f);

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
		ray.origin = (float4)(hitpoint + normal_facing * EPSILON, 0.0f);
		ray.direction = (float4)(newdir, 0.0f);

		/* add the colour and light contributions to the accumulated colour */
		accum_color += mask * hitsphere.emission.xyz; 

		/* the mask colour picks up surface colours at each bounce */
		mask *= hitsphere.color.xyz; 
		
		/* perform cosine-weighted importance sampling for diffuse surfaces*/
		mask *= dot(newdir, normal_facing); 
	}

	return accum_color;
}

__kernel void render(int width, int height, 
					 __global const Camera* camera,
                     __global const Sphere* spheres, int sphere_count,
					 __global const Material* materials, int material_count,
                     float random_seed, __global uchar4* output)
{
    int x = get_global_id(0), y = get_global_id(1);
    if (x >= width || y >= height) return;
    int idx = y*width + x;
    uint seed0 = x;
    uint seed1 = y;

    float3 sum = (float3)(0);
    for(int j = 0; j != SAMPLES; j++) {
        for (int s = 0; s < SAMPLES_PER_PIXEL; ++s) {
            float2 jitter = sample_square(&random_seed, &seed0, &seed1);
            Ray camray = create_ray(x, y, &camera, jitter);
            sum += trace(spheres, &camray, sphere_count, &seed0, &seed1);
        }
    }

    float3 avg = sum / ((float)SAMPLES_PER_PIXEL * (float)SAMPLES);

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

