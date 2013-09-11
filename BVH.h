#ifndef CSE168_BVH_H_INCLUDED
#define CSE168_BVH_H_INCLUDED

#include "Miro.h"
#include "Object.h"
#include <string>
#include "BBox.h"


class BVH
{
public:
    void build(Objects * objs);
    bool intersect(HitInfo& result, const Ray& ray,
                   float tMin = 0.0f, float tMax = MIRO_TMAX) const;
	void draw();
    
    int totalnBoxes;
	int nLeafNodes;
protected:
	BBox *root;
	Objects *m_objects;
	void divide(BBox* bbox, int depth);
	bool intersectBVH(BBox* bbox, HitInfo& minHit, const Ray& ray,
					float tMin = 0.0f, float tMax = MIRO_TMAX) const;
};

#endif // CSE168_BVH_H_INCLUDED
