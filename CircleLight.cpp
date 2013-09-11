#include "CircleLight.h"

Vector3 CircleLight::generateRandomPosition(){
	Vector3 p; // a point
	Vector3 v;
	 float cosi;
	 do{
		 do{
			 p.x = xfixed ? center.x : radius * (rand() / (float)RAND_MAX);
			 p.y = yfixed ? center.y : radius * (rand() / (float)RAND_MAX);
			 p.z = zfixed ? center.z : radius * (rand() / (float)RAND_MAX);
			 v = p - center;
		 } while (v.length() > radius);
	 } while (dot(n,v) != 0.0); // check if the point is in the plane
	 return p;
}
