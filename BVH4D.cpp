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
	if (bbox->lastElement - bbox->firstElement  <= 3 || (bbox->bounds4D[1].w - bbox->bounds4D[0].w) <= (1.0f/(temporalSamples))) {
		bbox->isLeaf = true;
		nLeafs++;
	} else {
		bbox->isLeaf = false;
		int axis;		
		if(temporalSamples == 1){ // Static or motion-blured scene?
			axis = depth % 3;
		}else{
			axis = depth % 4;
		}
		BBox* child1 = new BBox();
		BBox* child2 = new BBox();
		if(axis != 3){
			cmp4d.axis = axis;
			child1->m_objects = bbox->m_objects;
			child2->m_objects = bbox->m_objects;
			std::sort(bbox->m_objects->begin() + bbox->firstElement, bbox->m_objects->begin() + bbox->lastElement, cmp4d);
			child1->firstElement = bbox->firstElement;
			child2->lastElement = bbox->lastElement;
			float minSplitCost = INFINITY;
			float minSplitPos;

			int step = std::max((bbox->lastElement- bbox->firstElement) / 100.0f, 1.0f);
			//std::cout << "Step: " << step << std::endl;

			for (int i = bbox->firstElement; i < bbox->lastElement; i += step) {
				child1->lastElement = i;
				child2->firstElement = i + 1;
				child1->calcDimensions4D(bbox->m_objects, bbox->bounds4D[0].w, bbox->bounds4D[1].w);
				child2->calcDimensions4D(bbox->m_objects, bbox->bounds4D[0].w, bbox->bounds4D[1].w);
				float splitCost = bbox->getbboxIsectCost() + (child1->surfaceArea4D() / bbox->surfaceArea4D())*child1->getbboxCost() +
					(child2->surfaceArea4D() / bbox->surfaceArea4D())*child2->getbboxCost();
				if (splitCost < minSplitCost) {
					minSplitCost = splitCost;
					minSplitPos = i;
				}
			}
			// Should we even split?
			if(minSplitPos > bbox->getbboxCost() && (bbox->lastElement-bbox->firstElement < 5)){
				bbox->isLeaf = true;
				nLeafs++;
				return;
			}
			child1->lastElement = minSplitPos;
			child2->firstElement = minSplitPos + 1;
			child1->calcDimensions4D(bbox->m_objects, bbox->bounds4D[0].w, bbox->bounds4D[1].w);
			child2->calcDimensions4D(bbox->m_objects, bbox->bounds4D[0].w, bbox->bounds4D[1].w);

		}else{
			// Timesplit!
			//	for(int d = 0; d < depth; d++){
			//		printf(".");
			//	}
			//	printf("Time: [%.4f ; %.4f] Objs: %d\n", bbox->bounds4D[0].w, bbox->bounds4D[1].w, bbox->lastElement - bbox->firstElement);
			tSplits++;
			bbox->isTimesplit = true;
			child1->firstElement = child2->firstElement = bbox->firstElement;
			child1->lastElement = child2->lastElement = bbox->lastElement;
			float timeSplit = (bbox->bounds4D[0].w + bbox->bounds4D[1].w)/2.0f;
			child1->calcDimensions4D(bbox->m_objects, bbox->bounds4D[0].w, timeSplit);
			child2->calcDimensions4D(bbox->m_objects, timeSplit, bbox->bounds4D[1].w);
			for (int i = child1->firstElement; i <= child1->lastElement; ++i) {
				child1->m_objects->push_back((*bbox->m_objects)[i]);
			}
			for (int i = child2->firstElement; i <= child2->lastElement; ++i) {
				child2->m_objects->push_back((*bbox->m_objects)[i]);
			}
			dupPointers += (bbox->lastElement - bbox->firstElement)*2;
			child1->firstElement = child2->firstElement = 0;
			child1->lastElement = child2->lastElement = child1->m_objects->size()-1;

		}
		child1->parent = bbox;
		child2->parent = bbox;
		divide(child1, depth + 1);
		divide(child2, depth + 1);
		bbox->child1 = child1;
		bbox->child2 = child2;
	}
}

bool checkBb(BBox *bbox){
	bool isValid = true;
	if(bbox->parent != 0){ // Skip root
		if(bbox->bounds4D[0] < bbox->parent->bounds4D[0]){ // Min check
			printf("[!] Min invalid");
			printf("Px: %.8f, Py: %.8f, Pz: %.8f, Pw: %.8f |\n   \tCx: %.8f, Cy: %.8f, Cz: %.8f, Cw: %.8f\n", bbox->parent->bounds4D[0].x,bbox->parent->bounds4D[0].y,bbox->parent->bounds4D[0].z,bbox->parent->bounds4D[0].w,
				bbox->bounds4D[0].x,bbox->bounds4D[0].y,bbox->bounds4D[0].z,bbox->bounds4D[0].w);
			isValid = false;
		}
		if(bbox->bounds4D[1] > bbox->parent->bounds4D[1]){ // Max check
			printf("[!] Max invalid");
			printf("Px: %.8f, Py: %.8f, Pz: %.8f, Pw: %.8f |\n   \tCx: %.8f, Cy: %.8f, Cz: %.8f, Cw: %.8f\n", bbox->parent->bounds4D[1].x,bbox->parent->bounds4D[1].y,bbox->parent->bounds4D[1].z,bbox->parent->bounds4D[1].w,
				bbox->bounds4D[1].x,bbox->bounds4D[1].y,bbox->bounds4D[1].z,bbox->bounds4D[1].w);
			printf("Timesplit parent: %d\n", bbox->parent->isTimesplit? 1 : 0);
			isValid = false;
		}
	}

	if(bbox->isLeaf){
		return isValid;
	}else{
		bool e1 = checkBb(bbox->child1);
		bool e2 = checkBb(bbox->child2);
		isValid = (e1 && e2);
	}
	return isValid;
}

void
	BVH4D::build(Objects * objs)
{
	m_objects = objs;
	root = new BBox();
	root->parent = 0;
	root->m_objects = objs;
	root->firstElement = 0;
	root->lastElement = m_objects->size() - 1;
	root->calcDimensions4D(m_objects, 0.0f, 1.0f);

	divide(root, 0);

	printf("[-]  Total Bboxes: %d\n", nBoxes);
	printf("[-]  Total Leafs: %d\n", nLeafs);
	printf("[-]  Total Time splits: %d\n", tSplits);
	printf("[-]  Number of duplicate pointers: %d\n", dupPointers);
	if(true){ // Check 4D BVH for errors?
		printf("[-]   Checking for errors...\n");
		if(checkBb(root))
			printf("[-]   Success, 4D BVH is valid!\n");
		else
			printf("[-]   ERROR! 4D BVH Contains invalid nodes!\n");
	}
}

bool BVH4D::intersectBVH4D(BBox* bbox, HitInfo& minHit, const Ray& ray, float tMin, float tMax) const
{
	bool hit = false;
	HitInfo tempMinHit;
	if (bbox->intersect4D(ray, tMin, tMax)) {
		if (bbox->isLeaf) {
			for (int i = bbox->firstElement; i <= bbox->lastElement; ++i) {
				(*bbox->m_objects)[i]->interpolate(ray.time);
				if ((*bbox->m_objects)[i]->intersect(tempMinHit, ray, tMin, tMax)) {
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
		// Does not make sense in 4D
		root->draw4D(true,0);
}