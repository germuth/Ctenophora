#include <iostream>

#include "float.h"

#include "camera.h"
#include "material.h"
#include "sphere.h"
#include "hitable_list.h"
#include "ray.h"
#include "vec3.h"

// TODO
// 1. run program N times, with different seeds, average the results
// together one by one as they come in
float kMAXFLOAT = 99999999;
vec3 kRED(1.0, 0.0, 0.0);

// returns color of scene
vec3 color(const ray& r, hitable* world, int depth) {
	hit_record rec;
	if (world->hit(r, 0.001, kMAXFLOAT, rec)) {
	  ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
			return attenuation * color(scattered, world, depth+1);
	  } else {
			return vec3(0,0,0);
	  }
	} else {
		// paint a blue -> white background

					// get direction of camera -> scene ray
					vec3 unit_direction = unit_vector(r.direction());
					// y direction will be [-1, 1], turn into [0, 1]
					float alpha = 0.5 * (unit_direction.y() + 1.0);

					// linear interpolate using t inbetween two colors
					vec3 white(1.0, 1.0, 1.0);
					vec3 blue(0.5, 0.7, 1.0);
					return (1.0 - alpha) * white + alpha * blue;
	}
}

hitable *random_scene() {
	int n = 500;
	hitable **list = new hitable*[n+1];
	list[0] =  new sphere(vec3(0,-1000,0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
	int i = 1;
	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			float choose_mat = my_rand();
			vec3 center(a+0.9*my_rand(),0.2,b+0.9*my_rand()); 
			if ((center-vec3(4,0.2,0)).length() > 0.9) { 
				if (choose_mat < 0.8) {  // diffuse
					list[i++] = new sphere(center, 0.2, new lambertian(vec3(my_rand()*my_rand(), my_rand()*my_rand(), my_rand()*my_rand())));
				}
				else if (choose_mat < 0.95) { // metal
					list[i++] = new sphere(center, 0.2,
							new metal(vec3(0.5*(1 + my_rand()), 0.5*(1 + my_rand()), 0.5*(1 + my_rand())),  0.5*my_rand()));
				}
				else {  // glass
					list[i++] = new sphere(center, 0.2, new dielectric(1.5));
				}
			}
		}
	}

	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

	return new hitable_list(list,i);
}

int main() {
	int nx = 1200;
	int ny = 800;
	int ns = 10;
	std::cout << "P3\n" << nx << " " << ny << "\n255\n";

	vec3 lookfrom(13,2,3);
	vec3 lookat(0,0,0);
	float dist_to_focus = 10.0;
	float aperature = 0.0;
	camera cam(lookfrom, lookat, vec3(0,1,0), 20, float(nx)/float(ny), aperature, dist_to_focus);
	
	hitable *world = random_scene();
/*
	int nx = 200;
	int ny = 100;
	// P3 means ascii, then nx columns (width), ny rows (height) 
	std::cout << "P3\n" << nx << " " << ny << "\n255\n";

	int ns = 100;

	// camera is at the origin
	vec3 lookfrom(3,3,2);
	vec3 lookat(0,0,-1);
	float dist_to_focus = (lookfrom-lookat).length();
	float aperature = 0.0;
	camera cam(lookfrom, lookat, vec3(0,1,0), 20, float(nx)/float(ny), aperature, dist_to_focus);
	// x axis ->
	// y axis ^^
	// z axis towards camera

	int num_objects = 5;
	hitable *list[num_objects];
	// center sphere
	list[0] = new sphere(vec3(0,0,-1), 0.5, new lambertian(vec3(0.1, 0.2,0.5)));
	// world sphere that makes ground
	list[1] = new sphere(vec3(0,-100.5,-1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
	// some metal spheres
	list[2] = new sphere(vec3(1,0,-1), 0.5, new metal(vec3(0.8,0.6,0.2), 0.3));
	list[3] = new sphere(vec3(-1,0,-1), 0.5, new dielectric(1.5));
	// negative radius sphere will flip the normals, and create a hollow glass sphere.
	list[4] = new sphere(vec3(-1,0,-1), -0.45, new dielectric(1.5));

	hitable *world = new hitable_list(list, num_objects);
*/
/*
	int num_objects = 2;
	hitable *list[num_objects];
	float R = cos(M_PI/4);
	list[0] = new sphere(vec3(-R,0,-1), R, new lambertian(vec3(0, 0, 1)));
	list[1] = new sphere(vec3( R,0,-1), R, new lambertian(vec3(1, 0, 0)));
	*/

	// PPM format writes top left to bottom right, in rows
	for (int j = ny-1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {
			// color for this pixel
			vec3 col(0,0,0);

			for (int s = 0; s < ns; s++) {
				// percent on x and y axis through the scene from bottom left corner
				float percent_right = float(i + my_rand()) / float(nx);
				float percent_up = float(j + my_rand()) / float(ny);

				// create ray from camera to each pixel
				ray r = cam.get_ray(percent_right, percent_up);

				// this does nothing?
				// vec3 p = r.point_at_parameter(2.0);

				col += color(r, world, 0);
			}

			// average each sample
			col /= float(ns);
			// adjust colors for gamma
			col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));

			int ir = int(255.99 * col[0]);
			int ig = int(255.99 * col[1]);
			int ib = int(255.99 * col[2]);
			std::cout << ir << " " << ig << " " << ib << "\n";
		}
	}
}
