#ifndef RAYH
#define RAYH

#include "vec3.h"

class ray
{
    public:
        ray() {}
        ray(const vec3& a, const vec3& b) { A = a; B = b; }  
        vec3 origin() const       { return A; }
        vec3 direction() const    { return B; }

				// return a new vec that points from A to B, with magnitude t
        vec3 point_at_parameter(float t) const { return A + t*B; }

				// origin of the vector, where it points from.
        vec3 A;
				// unit vector where the vector points.
				// not actually a unit vector because the code is maybe simpler/faster 
				// without it??
        vec3 B;
};

#endif 
