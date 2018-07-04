#ifndef MATERIALH
#define MATERIALH 

#include <random>

struct hit_record;

#include "ray.h"
#include "hitable.h"


// polynomial approximation for glass' reflectivity depending on angle
float schlick(float cosine, float ref_idx) {
    float r0 = (1-ref_idx) / (1+ref_idx);
    r0 = r0*r0;
    return r0 + (1-r0)*pow((1 - cosine),5);
}

bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted) {
    vec3 uv = unit_vector(v);
    float dt = dot(uv, n);
    float discriminant = 1.0 - ni_over_nt*ni_over_nt*(1-dt*dt);
    if (discriminant > 0) {
        refracted = ni_over_nt*(uv - n*dt) - n*sqrt(discriminant);
        return true;
    }
		return false;
}


vec3 reflect(const vec3& v, const vec3& n) {
	return v - 2*dot(v,n)*n;
}

// [0.0, 1.0)
std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(0.0, 1.0);
float my_rand() {
        return distribution(generator);
}

vec3 rand_in_unit_sphere() {
  vec3 p;
        do {
                // random point in [0,1]
                p = vec3(my_rand(), my_rand(), my_rand());
                // adjust to [-1, 1]
                p = 2.0*p - vec3(1,1,1);

                // this is now random point in unit square.
        } while(p.squared_length() >= 1.0);

        // now its guaranteed to point inside sphere.
        return p;
}

class material  {
    public:
        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const = 0;
};

// randomly diffuse the ray in a random direction
class lambertian : public material {
    public:
        lambertian(const vec3& a) : albedo(a) {}
        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const  {
					// material is diffuse, need to bounce light a random direction away.
					// Use random point on unit_sphere to pick direction (Lambertian diffusion).
					// point of concet + 1 unit off using normal vector + random direction
					vec3 target = rec.p + rec.normal + rand_in_unit_sphere();
					// this new scattered ray will have "attenuated" less energy, will bounce
				  // off this pixel, and may hit the scene somewhere else. Lets say it hits
					// a red sphere, that red ray will then bounce back to the camera. So
					// this current original pixel should be slightly red according to the camera.
					scattered = ray(rec.p, target-rec.p);
					attenuation = albedo;
					return true;
        }

        vec3 albedo;
};

// reflect the ray at complementary angle
class metal : public material {
    public:
        metal(const vec3& a, float f) : albedo(a) {
					if (f < 1) fuzz = f; else fuzz = 1;
				}
        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const  {
            vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
            scattered = ray(rec.p, reflected + fuzz*rand_in_unit_sphere());
            attenuation = albedo;
            return (dot(scattered.direction(), rec.normal) > 0);
        }
        vec3 albedo;
				// the sphere ain't perfectly smooth so "fuzz" where the reflected ray ends up by this radius
        float fuzz;
};

// incoming rays are split into reflected and refracted (transmitted)
// randomly choose between reflected or refraction for each interaction
class dielectric : public material { 
    public:
        dielectric(float ri) : ref_idx(ri) {}
        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const  {
             vec3 outward_normal;
             vec3 reflected = reflect(r_in.direction(), rec.normal);
             float ni_over_nt;
						 // attenuation is always 1, the glass surface absorbs nothing.
             attenuation = vec3(1.0, 1.0, 1.0); 
             vec3 refracted;
             float reflect_prob;
             float cosine;
             if (dot(r_in.direction(), rec.normal) > 0) {
                  outward_normal = -rec.normal;
                  ni_over_nt = ref_idx;
                  cosine = ref_idx * dot(r_in.direction(), rec.normal) / r_in.direction().length();
                  //cosine = dot(r_in.direction(), rec.normal) / r_in.direction().length();
                  //cosine = sqrt(1 - ref_idx*ref_idx*(1-cosine*cosine));
             } else {
                  outward_normal = rec.normal;
                  ni_over_nt = 1.0 / ref_idx;
                  cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
             }
             if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted)) 
                reflect_prob = schlick(cosine, ref_idx);
             else {
                reflect_prob = 1.0;
						 }
             if (my_rand() < reflect_prob) 
                scattered = ray(rec.p, reflected);
             else {
                scattered = ray(rec.p, refracted);
						 }
             return true;
        }

				// refractive index. air = 1, glass = 1.3-1.7, diamond = 2.4
        float ref_idx;
};

#endif
