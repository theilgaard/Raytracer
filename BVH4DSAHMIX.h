#ifndef CSE168_BVH4DSAHMIX_H_INCLUDED
#define CSE168_BVH4DSAHMIX_H_INCLUDED

#include "Miro.h"
#include "Object.h"
#include <string>
#include "BBox.h"
#include "AccStructure.h"
#include <iostream>
#include <fstream>

class BVH4DSAHMIX : public AccStructure
{
public:
	BVH4DSAHMIX(int tempSamples) : temporalSamples(tempSamples) { nBoxes = 0; nLeafs = 0; tSplits = 0, dupPointers = 0;}
    virtual void build(Objects * objs);
    virtual bool intersect(HitInfo& result, const Ray& ray,
                   float tMin = 0.0f, float tMax = MIRO_TMAX);
	virtual void draw();
protected:
	int dupPointers;
	int nBoxes;
	int nLeafs;
	int tSplits;
	int temporalSamples;
	BBox *root;
	Objects *m_objects;
	void divide(BBox* bbox, int depth);
	bool intersectBVH4D(BBox* bbox, HitInfo& minHit, const Ray& ray,
					float tMin = 0.0f, float tMax = MIRO_TMAX) const;
	std::ofstream tssplitsFile;
};

#endif // CSE168_BVH4D_H_INCLUDED
