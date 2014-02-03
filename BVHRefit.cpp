#include "BVHRefit.h"
#include "Ray.h"
#include "Console.h"
#include <cstdlib>
#include <algorithm>


struct objectCmp {
	int axis;
	bool operator() (Object* obj1, Object* obj2)
	{
		return obj1->centroid[axis] < obj2->centroid[axis];
	}
} cmpRefit;

void BVHRefit::divide(BBox* bbox, int depth)
{
	nBoxes++;
	if (bbox->lastElement - bbox->firstElement <= 3) {
		bbox->isLeaf = true;
		nLeafs++;
	} else {
		bbox->isLeaf = false;
		int axis = depth % 3;
		cmpRefit.axis = axis;
		std::sort(m_objects->begin() + bbox->firstElement, m_objects->begin() + bbox->lastElement, cmpRefit);
		BBox* child1 = new BBox();
		BBox* child2 = new BBox();
		child1->firstElement = bbox->firstElement;
		child2->lastElement = bbox->lastElement;
		float minSplitCost = INFINITY;
		float minSplitPos;

		int step = std::max((bbox->lastElement- bbox->firstElement) / 100.0f, 1.0f);
		//std::cout << "Step: " << step << std::endl;

		for (int i = bbox->firstElement; i < bbox->lastElement; i += step) {
			child1->lastElement = i;
			child2->firstElement = i + 1;
			child1->calcDimensions(m_objects);
			child2->calcDimensions(m_objects);
			float splitCost = (child1->surfaceArea() / bbox->surfaceArea())*child1->getbboxCost() +
				(child2->surfaceArea() / bbox->surfaceArea())*child2->getbboxCost();
			if (splitCost < minSplitCost) {
				minSplitCost = splitCost;
				minSplitPos = i;
			}
		}
		child1->lastElement = minSplitPos;
		child2->firstElement = minSplitPos + 1;
		child1->calcDimensions(m_objects);
		child2->calcDimensions(m_objects);
		divide(child1, depth + 1);
		divide(child2, depth + 1);
		bbox->child1 = child1;
		bbox->child2 = child2;
	}
}

void
BVHRefit::build(Objects * objs)
{
	m_objects = objs;
	root = new BBox();
	root->firstElement = 0;
	root->lastElement = m_objects->size() - 1;
	root->calcDimensions(m_objects);

	divide(root, 0);
}

bool BVHRefit::intersectBVHRefit(BBox* bbox, HitInfo& minHit, const Ray& ray, float tMin, float tMax) const
{
	bool hit = false;
	HitInfo tempMinHit;
	if (bbox->intersect(ray, tMin, tMax)) {
		if (bbox->isLeaf) {
			for (int i = bbox->firstElement; i <= bbox->lastElement; ++i) {
				if ((*m_objects)[i]->intersect(tempMinHit, ray, tMin, tMax)) {
					hit = true;
					if (tempMinHit.t < minHit.t)
						minHit = tempMinHit;
				}
			}
		}
		else {
			bool child1Hit = intersectBVHRefit(bbox->child1, minHit, ray, tMin, tMax);
			bool child2Hit = intersectBVHRefit(bbox->child2, minHit, ray, tMin, tMax);
			hit = (child1Hit || child2Hit);
		}
	}
	return hit;
}

bool
BVHRefit::intersect(HitInfo& minHit, const Ray& ray, float tMin, float tMax)
{
	minHit.t = MIRO_TMAX;
	return intersectBVHRefit(root, minHit, ray, tMin, tMax);
}

void 
BVHRefit::draw() {
	root->draw(true);
}