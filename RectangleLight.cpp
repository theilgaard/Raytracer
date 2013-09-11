#include "RectangleLight.h"

Vector3 RectangleLight::generateRandomPosition(){
	Vector3 p; // a point
	 p.x = xfixed ? center.x : (xmax - xmin) * (rand() / (float)RAND_MAX) + xmin;
	 p.y = yfixed ? center.y : (ymax - ymin) * (rand() / (float)RAND_MAX) + ymin;
	 p.z = zfixed ? center.z : (zmax - zmin) * (rand() / (float)RAND_MAX) + zmin;
	 return p;
}

void RectangleLight::setDimensions(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax)
{
}
