#ifndef CSE168_BVH4D_H_INCLUDED
#define CSE168_BVH4D_H_INCLUDED

#include "Miro.h"
#include "Object.h"
#include <string>
#include "BBox.h"
#include "AccStructure.h"


class BVH4D : public AccStructure
{
public:
	BVH4D(int tempSamples) : temporalSamples(tempSamples) { nBoxes = 0; nLeafs = 0; }
    virtual void build(Objects * objs);
    virtual bool intersect(HitInfo& result, const Ray& ray,
                   float tMin = 0.0f, float tMax = MIRO_TMAX);
	virtual void draw();
protected:
	int nBoxes;
	int nLeafs;
	int temporalSamples;
	BBox *root;
	Objects *m_objects;
	void divide(BBox* bbox, int depth);
	bool intersectBVH4D(BBox* bbox, HitInfo& minHit, const Ray& ray,
					float tMin = 0.0f, float tMax = MIRO_TMAX) const;
};

#endif // CSE168_BVH4D_H_INCLUDED
