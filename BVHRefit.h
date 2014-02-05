#ifndef CSE168_BVHRefit_H_INCLUDED
#define CSE168_BVHRefit_H_INCLUDED

#include "Miro.h"
#include "Object.h"
#include <string>
#include "BBox.h"
#include "AccStructure.h"


class BVHRefit : public AccStructure
{
public:
	BVHRefit() { nBoxes = 0; nLeafs = 0, time = -1.f; }
    virtual void build(Objects * objs);
    virtual bool intersect(HitInfo& result, const Ray& ray,
                   float tMin = 0.0f, float tMax = MIRO_TMAX);
	virtual void draw();

	void refit(BBox* bbox);
    
	float time;
    int nBoxes;
	int nLeafs;
protected:
	BBox *root;
	Objects *m_objects;
	void divide(BBox* bbox, int depth);
	bool intersectBVHRefit(BBox* bbox, HitInfo& minHit, const Ray& ray,
					float tMin = 0.0f, float tMax = MIRO_TMAX) const;
};

#endif // CSE168_BVHRefit_H_INCLUDED
