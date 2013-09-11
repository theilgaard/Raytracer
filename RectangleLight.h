#ifndef RECTANGLELIGHT_H_
#define RECTANGLELIGHT_H_

#include "AreaLight.h"

class RectangleLight : public AreaLight
{
public:
	Vector3 generateRandomPosition();
	float xmin, xmax, ymin, ymax, zmin, zmax;
	void setDimensions(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);
};



#endif /* RECTANGLELIGHT_H_ */
