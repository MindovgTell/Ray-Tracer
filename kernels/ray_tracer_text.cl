__constant float EPSILON = 1e-3f; /* required to compensate for limited float precision */
__constant float PI = 3.14159265359f;
__constant int SAMPLES = 100;
__constant int SAMPLES_PER_PIXEL =  50;

#define MAT_LAMBERTIAN 0
#define MAT_METAL 1
#define MAT_DIELECTRIC 2


typedef struct Camera {
    // Camera settings
    float4 origin; // float4(origin.x, origin.y, origin.z, focal_length) the 4th argument is focal_length
   
    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    float4 pixel_delta_x;
    float4 pixel_delta_y;

    float4 pixel00_pos;
} Camera;

typedef struct Material{
	float4 albedo_fuzz; // (float4)(albedo.x, albedo.y, albedo.z, fuzz)
	int    type; 
	float  ref_idx;
	float _pad0,_pad1;
} Material;


typedef struct Sphere{
	float4 center_r; // position + radius (float4)(pos.x, pos.y, pos.z, radius)
	float4 emission;
	int material_index; 
	int _pad0,_pad1,_pad2;
} Sphere;

typedef struct Ray{
	float4 origin;
	float4 direction;
} Ray;




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

// static inline float2 sample_square(const __private float* fseed,
//                                    uint* seed0, uint* seed1)
// {
//     // decorrelate to get jx
//     uint a0 = (*seed0) ^ 0x9E3779B9u;
//     uint a1 = (*seed1) ^ 0x85EBCA6Bu;
//     float jx = get_random(&a0, &a1) - 0.5f;

//     // decorrelate differently to get jy
//     a0 ^= 0xC2B2AE35u; 
//     a1 ^= 0x27D4EB2Fu;
//     float jy = get_random(&a0, &a1) - 0.5f;

//     return (float2)(jx, jy);
// }

inline float3 offset_along_normal(float3 p, float3 n, float3 newdir) {
    float s = (dot(newdir, n) >= 0.0f) ? 1.0f : -1.0f;
    return p + n * (s * EPSILON);
}

static inline Ray create_ray(int x, int y, __global const Camera* cam, float2 jitter)
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
	float3 rayToCenter = (float3)((sphere->center_r.xyz - ray->origin.xyz));
	float b = dot(rayToCenter, (float3)(ray->direction.xyz ));
	float c = dot(rayToCenter, rayToCenter) - (sphere->center_r.w)*(sphere->center_r.w);
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


void lambert_scatter(const Sphere* hitsphere, Ray* ray, const Material* mat, float* t, float* xi1, float* xi2, float3* accum_color, float3* mask ) {
	/* compute the hitpoint using the ray equation */
	float3 hitpoint = ray->origin.xyz + ray->direction.xyz * (*t);
	
	/* compute the surface normal and flip it if necessary to face the incoming ray */
	float3 normal = normalize(hitpoint - hitsphere->center_r.xyz); 
	float3 w = dot(normal, ray->direction.xyz) < 0.0f ? normal : normal * (-1.0f);

	
	float phi = 2.0f * PI * (*xi1);
	float r2  = (*xi2);
	float r2s = sqrt(r2);

	/* create a local orthogonal coordinate frame centered at the hitpoint */

	float3 axis = fabs(w.x) > 0.1f ? (float3)(0.0f, 1.0f, 0.0f) : (float3)(1.0f, 0.0f, 0.0f);
	float3 u = normalize(cross(axis, w));
	float3 v = cross(w, u);

	/* use the coordinte frame and random numbers to compute the next ray direction */
	float3 newdir = normalize(u * cos(phi)*r2s + v*sin(phi)*r2s + w*sqrt(1.0f - r2));

	/* add a very small offset to the hitpoint to prevent self intersection */
	ray->origin = (float4)(hitpoint + w * EPSILON, 0.0f);
	ray->direction = (float4)(newdir, 0.0f);

	(*accum_color) += (*mask) * hitsphere->emission.xyz; // keep if you actually use emission
	(*mask) *= (float3)(mat->albedo_fuzz.x, mat->albedo_fuzz.y, mat->albedo_fuzz.z);
	// cosine-weighted term belongs to lambert:
	(*mask) *= fmax(dot(newdir, w), 0.0f);
}


inline float3 reflect(const float3* direction, const float3* normal) {
	float n = dot((*direction), (*normal));
    return  (*direction) - 2*n*(*normal);
}




void metal_scatter(const Sphere* hitsphere, Ray* ray, const Material* mat, float* t,
                   float3* accum_color, float3* mask, const float3* _unused) {

    float3 hitpoint = ray->origin.xyz + ray->direction.xyz * (*t);
    float3 n        = normalize(hitpoint - hitsphere->center_r.xyz);
    float3 w        = dot(n, ray->direction.xyz) < 0.0f ? n : -n;

    float3 dir = ray->direction.xyz;
    float3 reflected = reflect(&dir, &w);

    // jitter WITHOUT normalization; scale by fuzz (already in mat->albedo_fuzz.w)
    uint a = as_uint(hitpoint.x) ^ 0xC2B2AE35u;
    uint b = as_uint(hitpoint.y) ^ 0x27D4EB2Fu;
    float3 jitter = (float3)(
        get_random(&a,&b) - 0.5f,
        get_random(&a,&b) - 0.5f,
        get_random(&a,&b) - 0.5f
    );

    float3 newdir = normalize(reflected + mat->albedo_fuzz.w * jitter);

    float3 neworig = offset_along_normal(hitpoint, w, newdir);
    ray->origin    = (float4)(neworig, 0.0f);
    ray->direction = (float4)(newdir, 0.0f);

    (*accum_color) += (*mask) * hitsphere->emission.xyz;
    (*mask) *= (float3)(mat->albedo_fuzz.x, mat->albedo_fuzz.y, mat->albedo_fuzz.z);
}




// Calculates reflectance for Dielectric materials using Schlick's approximation

inline float reflectance(float cosine, float ri) {
    float r0 = (1.0f - ri) / (1.0f + ri);
    r0 = r0 * r0;
    return r0 + (1.0f - r0) * pow(1.0f - cosine, 5.0f);
}

inline float3 refract_dir(const float3 in, const float3 n, float eta) {
    float cosi = clamp(-dot(in, n), 0.0f, 1.0f);
    float sint2 = max(0.0f, 1.0f - cosi*cosi);
    float k = 1.0f - eta*eta * sint2;
    if (k < 0.0f) return (float3)(0,0,0); // TIR flag (caller should reflect)
    return normalize(eta*in + (eta*cosi - sqrt(k))*n);
}

// void dielectric_scatter(const Sphere* hitsphere, Ray* ray, const Material* mat,
//                         float* t, float3* accum_color, float3* mask, float* xi)
// {
//     float3 hit = ray->origin.xyz + ray->direction.xyz * (*t);
//     float3 n   = normalize(hit - hitsphere->center_r.xyz);
//     bool front = dot(ray->direction.xyz, n) < 0.0f;
//     float3 w   = front ? n : -n;

//     float eta = front ? (1.0f / mat->ref_idx) : mat->ref_idx;

//     float cos_theta = clamp(-dot(normalize(ray->direction.xyz), w), 0.0f, 1.0f);
//     // Schlick's approximation expects the refractive index (n), not the ratio n1/n2.
// 	float R = reflectance(cos_theta, mat->ref_idx);
//     float3 in = normalize(ray->direction.xyz);

//     float3 dir;
//     if ((*xi) < R) {
//         dir = reflect(&in, &w);
//     } else {
//         float3 tdir = refract_dir(in, w, eta);
//         if (tdir.x == 0.0f && tdir.y == 0.0f && tdir.z == 0.0f) {
//             dir = reflect(&in, &w); // TIR fallback
//         } else {
//             dir = tdir;
//         }
//     }



//     // Typically, dielectric has no absorption: keep mask as-is,
//     // or apply slight attenuation if you want (beer’s law).
//     ray->origin    = (float4)(hit + w * EPSILON, 0.0f);
//     ray->direction = (float4)(normalize(dir), 0.0f);
// }

void dielectric_scatter(const Sphere* hitsphere, Ray* ray, const Material* mat,
                        float* t, float3* accum_color, float3* mask, float* xi)
{
    float3 hit = ray->origin.xyz + ray->direction.xyz * (*t);
    float3 n   = normalize(hit - hitsphere->center_r.xyz);
    bool front = dot(ray->direction.xyz, n) < 0.0f;
    float3 w   = front ? n : -n;

    float eta = front ? (1.0f / mat->ref_idx) : mat->ref_idx;
    float3 in = normalize(ray->direction.xyz);

    // cosine between incoming ray and surface normal (clamped)
    float cos_theta = clamp(-dot(in, w), 0.0f, 1.0f);

    // When the ray is exiting the medium we should use the transmitted-angle cosine
    // for Schlick's approximation to get the correct reflect probability.
    float R;
    if (!front) {
        // compute the cosine of the transmitted angle (safe-guard sqrt)
        float tmp = 1.0f - eta*eta * (1.0f - cos_theta * cos_theta);
        float cos_trans = tmp > 0.0f ? sqrt(tmp) : 0.0f;
        R = reflectance(cos_trans, mat->ref_idx);
    } else {
        R = reflectance(cos_theta, mat->ref_idx);
    }

    // Decide reflection vs refraction (handle TIR in refract_dir)
    float3 dir;
    float3 tdir = refract_dir(in, w, eta);
    bool tir = (tdir.x == 0.0f && tdir.y == 0.0f && tdir.z == 0.0f);
    if (tir || (*xi) < R) {
        dir = reflect(&in, &w);
    } else {
        dir = tdir;
    }

    // Offset along correct side to avoid self-intersection (follows outgoing direction)
    float3 neworig = offset_along_normal(hit, w, dir);
    ray->origin    = (float4)(neworig, 0.0f);
    ray->direction = (float4)(normalize(dir), 0.0f);

    // dielectric typically doesn't attenuate (no change to mask)
    (*accum_color) += (*mask) * hitsphere->emission.xyz;
}





/* the path tracing function */
/* computes a path (starting from the camera) with a defined number of bounces, accumulates light/color at each bounce */
/* each ray hitting a surface will be reflected in a random direction (by randomly sampling the hemisphere above the hitpoint) */
/* small optimisation: diffuse ray directions are calculated using cosine weighted importance sampling */

float3 trace( __global const Sphere* spheres, 
			  __global const Material* materials, 
			  const Ray* camray,  
			  const int sphere_count, 
			  const int material_count, 
			  unsigned int* seed0,
			  unsigned int* seed1 ) 
{
    Ray ray = *camray;

	float3 accum_color = (float3)(0.0f, 0.0f, 0.0f);
	float3 mask = (float3)(1.0f, 1.0f, 1.0f);

	for (int bounces = 0; bounces < 10; bounces++){

		float t;   /* distance to intersection */
		int hitsphere_id = 0; /* index of intersected sphere */

		/* if ray misses scene, return background colour */
		if (!intersect_scene(spheres, &ray, &t, &hitsphere_id, sphere_count))
		{
            float3 d = normalize((float3)(ray.direction.xyz));
            float tbg = 0.5f*(d.y + 1.0f);
            float3 sky = mix((float3)(0.0f,0.0f,1.0f), (float3)(0.8f,0.8f,1.0f), tbg);
            return accum_color + mask * sky;
        }

		/* else, we've got a hit! Fetch the closest hit sphere */
		Sphere hitsphere = spheres[hitsphere_id]; /* version with local copy of sphere */
		int mat_idx = (int)hitsphere.material_index;

		Material material = materials[mat_idx];

		switch(material.type) {
			case MAT_LAMBERTIAN : {
				/* compute two random numbers to pick a random point on the hemisphere above the hitpoint*/
				uint salt0 = (uint)*seed0 ^ (uint)(bounces*2+0) * 0x9E3779B9u;
				uint salt1 = (uint)*seed1 ^ (uint)(bounces*2+1) * 0x85EBCA6Bu;

				float xi1 = get_random(&salt0, &salt1); // in [0,1)
				float xi2 = get_random(&salt0, &salt1); // in [0,1)

				lambert_scatter(&hitsphere, &ray, &material, &t, &xi1, &xi2, &accum_color, &mask);
				/* perform cosine-weighted importance sampling for diffuse surfaces*/
				// mask *= dot(newdir, normal_facing); 
				break;
			}
			case MAT_METAL : {
				uint salt0 = (uint)*seed0 ^ 0xC2B2AE35u;
				uint salt1 = (uint)*seed1 ^ 0x27D4EB2Fu;
				float3 jitter = normalize((float3)(
					get_random(&salt0,&salt1) - 0.5f,
					get_random(&salt0,&salt1) - 0.5f,
					get_random(&salt0,&salt1) - 0.5f));
				metal_scatter(&hitsphere, &ray, &material, &t, &accum_color, &mask, &jitter);

				break;
			}

			case MAT_DIELECTRIC : {
				uint salt0 = (uint)*seed0 ^ (uint)(bounces*2+0) * 0x9E3779B9u;
				uint salt1 = (uint)*seed1 ^ (uint)(bounces*2+1) * 0x85EBCA6Bu;

				float xi1 = get_random(&salt0, &salt1); // in [0,1)
				dielectric_scatter(&hitsphere, &ray, &material, &t, &accum_color, &mask, &xi1);
				break;
			}

			// default : {
			// 	return (float3)(1.0f, 0.0f, 1.0f);
			// }
		}
	}

	return accum_color;
}

__kernel void render(int width, int height, 
					 __global const Camera* camera,
                     __global const Sphere* spheres, const int sphere_count,
					 __global const Material* materials, const int material_count,
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
            Ray camray = create_ray(x, y, camera, jitter);
            sum += trace(spheres, materials, &camray, sphere_count, material_count, &seed0, &seed1);
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

