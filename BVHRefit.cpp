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
			cmpRefit.axis = axis;

			std::sort(m_objects->begin() + bbox->firstElement, m_objects->begin() + bbox->lastElement, cmpRefit);

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
		if(minSplitPos > bbox->getbboxCost()){
			bbox->isLeaf = true;
			nLeafs++;
			return;
		}

		// Resort to min split
		cmpRefit.axis = minAxis;
		std::sort(m_objects->begin() + bbox->firstElement, m_objects->begin() + bbox->lastElement, cmpRefit);

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
BVHRefit::build(Objects * objs)
{
	m_objects = objs;
	root = new BBox();
	root->firstElement = 0;
	root->lastElement = m_objects->size() - 1;
	root->calcDimensions(m_objects);

	divide(root, 0);

	printf("[-]  Total Bboxes: %d\n", nBoxes);
	printf("[-]  Total Leafs: %d\n", nLeafs);
}

void
BVHRefit::refit(BBox* bbox){
	if(bbox->isLeaf){
		bbox->calcDimensions(m_objects);
	}else{
		refit(bbox->child1);
		refit(bbox->child2);
		bbox->bounds[0].x = std::min(bbox->child1->bounds[0].x, bbox->child2->bounds[0].x);  // bounds[0] == max
		bbox->bounds[0].y = std::min(bbox->child1->bounds[0].y, bbox->child2->bounds[0].y);  // bounds[0] == max
		bbox->bounds[0].z = std::min(bbox->child1->bounds[0].z, bbox->child2->bounds[0].z);  // bounds[0] == max
		bbox->bounds[1].x = std::max(bbox->child1->bounds[1].x, bbox->child2->bounds[1].x);  // bounds[1] == min
		bbox->bounds[1].y = std::max(bbox->child1->bounds[1].y, bbox->child2->bounds[1].y);  // bounds[1] == min
		bbox->bounds[1].z = std::max(bbox->child1->bounds[1].z, bbox->child2->bounds[1].z);  // bounds[1] == min
	}
}

bool 
BVHRefit::intersectBVHRefit(BBox* bbox, HitInfo& minHit, const Ray& ray, float tMin, float tMax) const
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

	// Refit the BVH to current timestep.
	if(time != ray.time){
		time = ray.time;
		for(int i = 0; i < m_objects->size(); i++){
			(*m_objects)[i]->interpolate(time);
		}
		refit(root);
	}

	return intersectBVHRefit(root, minHit, ray, tMin, tMax);
}

void 
BVHRefit::draw() {
	root->draw(true);
}