#include "AreaLight.h"

class CircleLight : public AreaLight
{
public:
	Vector3 generateRandomPosition();
	float radius;
};
