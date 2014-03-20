#ifndef ACCSTRUCTURE_H_
#define ACCSTRUCTURE_H_


#include "BBox.h"

class AccStructure {
protected:
	Objects *m_objects; // Primitives
public:
	virtual void build(Objects * objs) = 0;
	virtual bool intersect(HitInfo& minHit, const Ray& ray, float tMin, float tMax) = 0;

	virtual void draw() = 0;
};

#endif