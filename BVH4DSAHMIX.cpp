#include "BVH4DSAHMIX.h"
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
} cmp4dsahmix;

void BVH4DSAHMIX::divide(BBox* bbox, int depth)
{
	nBoxes++;
	if (bbox->lastElement - bbox->firstElement  <= 3 || (bbox->bounds4D[1].w - bbox->bounds4D[0].w) <= (1.0f/(temporalSamples))) {
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
		child1->m_objects = bbox->m_objects;
		child2->m_objects = bbox->m_objects;
		child1->firstElement = bbox->firstElement;
		child2->lastElement = bbox->lastElement;	

		// Test 3d split
		for(int i = 0; i < 3; i++){
			axis = i % 3;
			cmp4dsahmix.axis = axis;

			std::sort(bbox->m_objects->begin() + bbox->firstElement, bbox->m_objects->begin() + bbox->lastElement, cmp4dsahmix);

			int step = std::max((bbox->lastElement- bbox->firstElement) / 100.0f, 1.0f);

			for (int i = bbox->firstElement; i < bbox->lastElement; i += step) {
				child1->lastElement = i;
				child2->firstElement = i + 1;
				child1->calcDimensions4D(bbox->m_objects, bbox->bounds4D[0].w, bbox->bounds4D[1].w);
				child2->calcDimensions4D(bbox->m_objects, bbox->bounds4D[0].w, bbox->bounds4D[1].w);
				splitCost = bbox->getbboxIsectCost() + (child1->surfaceArea4D() / bbox->surfaceArea4D())*child1->getbboxCost() +
					(child2->surfaceArea4D() / bbox->surfaceArea4D())*child2->getbboxCost();
				if (splitCost < minSplitCost) {
					minSplitCost = splitCost;
					minSplitPos = i;
					minAxis = axis;
				}
			}
		}

		// Test time split
		child1->firstElement = child2->firstElement = bbox->firstElement;
		child1->lastElement = child2->lastElement = bbox->lastElement;
		//float tStep = std::max((bbox->bounds4D[1].w - bbox->bounds4D[0].w)/temporalSamples, 0.1f);
		float tStep = (bbox->bounds4D[1].w - bbox->bounds4D[0].w)/5;
		for(float i = bbox->bounds4D[0].w+tStep; i < bbox->bounds4D[1].w; i += tStep){
			child1->calcDimensions4D(bbox->m_objects, bbox->bounds4D[0].w, i);
			child2->calcDimensions4D(bbox->m_objects, i, bbox->bounds4D[1].w);
			splitCost = bbox->getbboxIsectCost() + (child1->surfaceArea4D() / bbox->surfaceArea4D())*child1->getbboxCost() +
				(child2->surfaceArea4D() / bbox->surfaceArea4D())*child2->getbboxCost();
			if (splitCost < minSplitCost) {
				bbox->isTimesplit = true;
				minSplitCost = splitCost;
				minSplitPos = i;
				minAxis = 3;
			}
		}

		// Should we even split?
		if(minSplitCost > bbox->getbboxCost() && (bbox->lastElement-bbox->firstElement) < 5){
			bbox->isLeaf = true;
			nLeafs++;
			return;
		}

		// Set child box vars according to case.
		if(minAxis != 3){
			cmp4dsahmix.axis = minAxis;
			child1->m_objects = bbox->m_objects;
			child2->m_objects = bbox->m_objects;
			std::sort(bbox->m_objects->begin() + bbox->firstElement, bbox->m_objects->begin() + bbox->lastElement, cmp4dsahmix);
			child1->firstElement = bbox->firstElement;
			child2->lastElement = bbox->lastElement;	

			// Left and right 3d bounding boxes. 
			child1->lastElement = minSplitPos;
			child2->firstElement = minSplitPos + 1;
			child1->calcDimensions4D(bbox->m_objects, bbox->bounds4D[0].w, bbox->bounds4D[1].w);
			child2->calcDimensions4D(bbox->m_objects, bbox->bounds4D[0].w, bbox->bounds4D[1].w);
		}else{
			tSplits++;
			//printf("MinSplitPos: %f\n", minSplitPos);
			child1->m_objects = new Objects();
			child2->m_objects = new Objects();
			//float timeSplit = (bbox->bounds4D[0].w + bbox->bounds4D[1].w)/2.0f;
			child1->calcDimensions4D(bbox->m_objects, bbox->bounds4D[0].w, minSplitPos);
			child2->calcDimensions4D(bbox->m_objects, minSplitPos, bbox->bounds4D[1].w);
			// Copy over the primitives for the timesplit children.
			for (int i = child1->firstElement; i <= child1->lastElement; ++i) {
				child1->m_objects->push_back((*bbox->m_objects)[i]);
			}
			for (int i = child2->firstElement; i <= child2->lastElement; ++i) {
				child2->m_objects->push_back((*bbox->m_objects)[i]);
			}
			dupPointers += (bbox->lastElement - bbox->firstElement)*2;
			// Make sure the first and last element now point to the start and end of their lists.
			child1->firstElement = child2->firstElement = 0;
			child1->lastElement = child2->lastElement = child1->m_objects->size()-1;

			// Save timesplit position for analysis.
			tssplitsFile << (bbox->bounds4D[1].w+bbox->bounds4D[0].w)/2 << ",";
			tssplitsFile << minSplitPos << "\n";
		}
		child1->parent = bbox;
		child2->parent = bbox;
		divide(child1, depth + 1);
		divide(child2, depth + 1);
		bbox->child1 = child1;
		bbox->child2 = child2;
	}
}

bool checkBbsahmix(BBox *bbox){
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
		bool e1 = checkBbsahmix(bbox->child1);
		bool e2 = checkBbsahmix(bbox->child2);
		isValid = (e1 && e2);
	}
	return isValid;
}

void
	BVH4DSAHMIX::build(Objects * objs)
{
	tssplitsFile.open("C:\\timeSplitFile.txt", std::ios::trunc);
	m_objects = objs;
	root = new BBox();
	root->parent = 0;
	root->m_objects = objs;
	root->firstElement = 0;
	root->lastElement = m_objects->size() - 1;
	root->calcDimensions4D(m_objects, 0.0f, 1.0f);

	divide(root, 0);

	tssplitsFile.close();
	printf("[-]  Total Bboxes: %d\n", nBoxes);
	printf("[-]  Total Leafs: %d\n", nLeafs);
	printf("[-]  Total Time splits: %d\n", tSplits);
	printf("[-]  Number of duplicate pointers: %d\n", dupPointers);
	if(true){ // Check 4D BVH for errors?
		printf("[-]   Checking for errors...\n");
		if(checkBbsahmix(root))
			printf("[-]   Success, 4D BVH is valid!\n");
		else
			printf("[-]   ERROR! 4D BVH Contains invalid nodes!\n");
	}
}

bool BVH4DSAHMIX::intersectBVH4D(BBox* bbox, HitInfo& minHit, const Ray& ray, float tMin, float tMax) const
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
	BVH4DSAHMIX::intersect(HitInfo& minHit, const Ray& ray, float tMin, float tMax)
{
	minHit.t = MIRO_TMAX;
	return intersectBVH4D(root, minHit, ray, tMin, tMax);
}

void 
	BVH4DSAHMIX::draw() {
		// Does not make sense in 4D
		root->draw4D(true,0);
}