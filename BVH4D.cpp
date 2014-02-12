#include "BVH4D.h"
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
} cmp4d;

void BVH4D::divide(BBox* bbox, int depth)
{
	nBoxes++;
	if (bbox->lastElement - bbox->firstElement <= 3) {
		bbox->isLeaf = true;
		nLeafs++;
	} else {
		bbox->isLeaf = false;
		int axis = depth % 4;
		BBox* child1 = new BBox();
		BBox* child2 = new BBox();
		if(axis != 3){
			cmp4d.axis = axis;
			std::sort(m_objects->begin() + bbox->firstElement, m_objects->begin() + bbox->lastElement, cmp4d);
			child1->firstElement = bbox->firstElement;
			child2->lastElement = bbox->lastElement;
			float minSplitCost = INFINITY;
			float minSplitPos;

			int step = std::max((bbox->lastElement- bbox->firstElement) / 100.0f, 1.0f);
			//std::cout << "Step: " << step << std::endl;

			for (int i = bbox->firstElement; i < bbox->lastElement; i += step) {
				child1->lastElement = i;
				child2->firstElement = i + 1;
				child1->calcDimensions4D(m_objects, bbox->bounds4D[0].w, bbox->bounds4D[1].w);
				child2->calcDimensions4D(m_objects, bbox->bounds4D[0].w, bbox->bounds4D[1].w);
				float splitCost = (child1->surfaceArea4D() / bbox->surfaceArea4D())*child1->getbboxCost() +
					(child2->surfaceArea4D() / bbox->surfaceArea4D())*child2->getbboxCost();
				if (splitCost < minSplitCost) {
					minSplitCost = splitCost;
					minSplitPos = i;
				}
			}
			child1->lastElement = minSplitPos;
			child2->firstElement = minSplitPos + 1;
			child1->calcDimensions4D(m_objects, bbox->bounds4D[0].w, bbox->bounds4D[1].w);
			child2->calcDimensions4D(m_objects, bbox->bounds4D[0].w, bbox->bounds4D[1].w);
		}else{
			// Timesplit!
			for(int d = 0; d < depth; d++){
				printf(".");
			}
			printf("Time: [%.3f ; %.3f] Objs: %d\n", bbox->bounds4D[0].w, bbox->bounds4D[1].w, bbox->lastElement - bbox->firstElement);
			child1->firstElement = child2->firstElement = bbox->firstElement;
			child1->lastElement = child2->lastElement = bbox->lastElement;
			child1->calcDimensions4D(m_objects, bbox->bounds4D[0].w, (bbox->bounds4D[0].w+bbox->bounds4D[1].w)/2.0f);
			child2->calcDimensions4D(m_objects, (bbox->bounds4D[0].w+bbox->bounds4D[1].w)/2.0f, bbox->bounds4D[1].w);
		}
		divide(child1, depth + 1);
		divide(child2, depth + 1);
		bbox->child1 = child1;
		bbox->child2 = child2;
	}
}

void
BVH4D::build(Objects * objs)
{
	m_objects = objs;
	root = new BBox();
	root->firstElement = 0;
	root->lastElement = m_objects->size() - 1;
	root->calcDimensions4D(m_objects, 0.0f, 1.0f);

	divide(root, 0);
}

bool BVH4D::intersectBVH4D(BBox* bbox, HitInfo& minHit, const Ray& ray, float tMin, float tMax) const
{
	bool hit = false;
	HitInfo tempMinHit;
	if (bbox->intersect4D(ray, tMin, tMax)) {
		if (bbox->isLeaf) {
			for (int i = bbox->firstElement; i <= bbox->lastElement; ++i) {
				(*m_objects)[i]->interpolate(ray.time);
				if ((*m_objects)[i]->intersect(tempMinHit, ray, tMin, tMax)) {
					hit = true;
					if (tempMinHit.t < minHit.t)
						minHit = tempMinHit;
				}
			}
		}
		else {
			bool child1Hit = intersectBVH4D(bbox->child1, minHit, ray, tMin, tMax);
			bool child2Hit = intersectBVH4D(bbox->child2, minHit, ray, tMin, tMax);
			hit = (child1Hit || child2Hit);
		}
	}
	return hit;
}

bool
BVH4D::intersect(HitInfo& minHit, const Ray& ray, float tMin, float tMax)
{
	minHit.t = MIRO_TMAX;
	return intersectBVH4D(root, minHit, ray, tMin, tMax);
}

void 
BVH4D::draw() {
	root->draw(true);
}