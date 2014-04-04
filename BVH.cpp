#include "BVH.h"
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
} cmp;

void BVH::divide(BBox* bbox, int depth)
{
	nBoxes++;
	if (bbox->lastElement - bbox->firstElement <= 3) {
		bbox->isLeaf = true;
		nLeafs++;
	} else {
		bbox->isLeaf = false;
		int axis;		
		BBox* child1 = new BBox();
		BBox* child2 = new BBox();
		float minSplitCost = INFINITY;
		float minSplitPos;
		int minAxis;
		float splitCost;
		child1->firstElement = bbox->firstElement;
		child2->lastElement = bbox->lastElement;	

		// Test 3d split
		for(int i = 0; i < 3; i++){
			axis = i % 3;
			cmp.axis = axis;

			std::sort(m_objects->begin() + bbox->firstElement, m_objects->begin() + bbox->lastElement, cmp);

			int step = std::max((bbox->lastElement- bbox->firstElement) / 100.0f, 1.0f);

			for (int i = bbox->firstElement; i < bbox->lastElement; i += step) {
				child1->lastElement = i;
				child2->firstElement = i + 1;
				child1->calcDimensions(m_objects);
				child2->calcDimensions(m_objects);
				splitCost = bbox->getbboxIsectCost() + (child1->surfaceArea() / bbox->surfaceArea())*child1->getbboxCost() +
					(child2->surfaceArea() / bbox->surfaceArea())*child2->getbboxCost();
				if (splitCost < minSplitCost) {
					minSplitCost = splitCost;
					minSplitPos = i;
					minAxis = axis;
				}
			}
		}
		// Should we even split?
		if(minSplitPos > bbox->getbboxCost() && (bbox->lastElement-bbox->firstElement) < 5){
			bbox->isLeaf = true;
			nLeafs++;
			return;
		}

		// Resort to min split
		cmp.axis = minAxis;
		std::sort(m_objects->begin() + bbox->firstElement, m_objects->begin() + bbox->lastElement, cmp);

		// Left and right 3d bounding boxes. 
		child1->lastElement = minSplitPos;
		child2->firstElement = minSplitPos + 1;
		child1->calcDimensions(m_objects);
		child2->calcDimensions(m_objects);
		child1->parent = bbox;
		child2->parent = bbox;

		divide(child1, depth + 1);
		divide(child2, depth + 1);
		bbox->child1 = child1;
		bbox->child2 = child2;
	}
}

void
BVH::build(Objects * objs)
{
	m_objects = objs;
	root = new BBox();
	root->firstElement = 0;
	root->lastElement = m_objects->size() - 1;
	root->calcDimensions(m_objects);

	divide(root, 0);
	printf("Number of leaf nodes: %i\n", nLeafs);
	printf("Number of non-leaf nodes: %i\n", nBoxes-nLeafs);
}

bool BVH::intersectBVH(BBox* bbox, HitInfo& minHit, const Ray& ray, float tMin, float tMax) const
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
			bool child1Hit = intersectBVH(bbox->child1, minHit, ray, tMin, tMax);
			bool child2Hit = intersectBVH(bbox->child2, minHit, ray, tMin, tMax);
			hit = (child1Hit || child2Hit);
		}
	}
	return hit;
}

bool
BVH::intersect(HitInfo& minHit, const Ray& ray, float tMin, float tMax)
{
	minHit.t = MIRO_TMAX;

	if(time != ray.time){
		time = ray.time;
		for(int i = 0; i < m_objects->size(); i++){
			(*m_objects)[i]->interpolate(time);
		}
	}

	return intersectBVH(root, minHit, ray, tMin, tMax);
}

void 
BVH::draw() {
	root->draw(true,0);
}