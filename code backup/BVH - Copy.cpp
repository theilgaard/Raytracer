#include "BVH.h"
#include "Ray.h"
#include "Console.h"
#include <cstdlib>
#include <algorithm>


struct Split{
	float cost;
	BBox *leftBox;
	BBox *rightBox;
	int nchildrenRight;
	int nchildrenLeft;
	std::vector<BBox*>::iterator split;
};

bool compareBoxesX(BBox* i, BBox* j){
	float centeri = (i->maxx - i->minx) / 2.0 + i->minx;
	float centerj = (j->maxx - j->minx) / 2.0 + j->minx;
	return (centeri < centerj);
}

bool compareBoxesY(BBox* i, BBox* j){
	float centeri = (i->maxy - i->miny) / 2.0 + i->miny;
	float centerj = (j->maxy - j->miny) / 2.0 + j->miny;
	return (centeri < centerj);
}

bool compareBoxesZ(BBox* i, BBox* j){
	float centeri = (i->maxz - i->minz) / 2.0 + i->minz;
	float centerj = (j->maxz - j->minz) / 2.0 + j->minz;
	return (centeri < centerj);
}

void
BVH::divide(BBox* root, std::vector<BBox*> *boxes, std::vector<BBox*>::iterator start, std::vector<BBox*>::iterator end,
								int nboxes, int prevAxis, bool wasSplit, std::string s, bool xtried, bool ytried, bool ztried)
{
	setDimensions(boxes, root, start, end);
	if(nboxes == 1){
		root->print(s);
		//printf((s + "nBoxes: %u\n").c_str(), nboxes);
		//fflush(stdout);
		nLeafNodes++;
		BBox box = **start;
		root->triangle = box.triangle;
		root->minx = box.minx;
		root->miny = box.miny;
		root->minz = box.minz;
		root->maxx = box.maxx;
		root->maxy = box.maxy;
		root->maxz = box.maxz;
		root->nchildren = 0;
		return;
	}
	std::string tab= " >";
	if(nboxes < 5){
		root->print(s);
		//printf((s + "Limit reached nBoxes: %u\n").c_str(), nboxes);
		//fflush(stdout);
		root->children = new BBox*[nboxes];
		root->nchildren = nboxes;
		totalnBoxes += nboxes;
		nLeafNodes++;
		int i = 0;
		std::vector<BBox*>::iterator p2;
		for(p2 = start; p2 != end; p2++){
			root->children[i] = *p2;
			i++;
		}
		return;
	}
	int axis = 1;
	if(wasSplit){
		float axisSize = root->maxx - root->minx;
		if((root->maxy - root->miny) > axisSize){
			axisSize = root->maxy - root->miny;
			axis = 2;
		}
		if((root->maxz - root->minz) > axisSize){
			axisSize = root->maxz - root->minz;
			axis = 3;
		}
	}
	else{
		axis = prevAxis + 1;
		if(axis == 4){
			axis = 1;
		}
	}
	if(xtried && ytried && ztried){
		//printf((s + "All tried: %u\n").c_str(), nboxes);
		//fflush(stdout);
		//split in the middle
		root->children = new BBox*[2];
		root->nchildren = 2;
		totalnBoxes += 2;
		BBox *leftBox = new BBox();
		BBox *rightBox = new BBox();
		root->children[0] = leftBox;
		root->children[1] = rightBox;
		int splitPos = nboxes / 2;
		int nchildrenLeft = distance(start, start + splitPos);
		divide(leftBox, boxes, start, start + splitPos, nchildrenLeft, axis, true, s + tab, false, false, false);
		divide(rightBox, boxes, start + splitPos, end, nboxes - nchildrenLeft, axis, true, s + tab, false, false, false);
		return;
	}
	float rmin, rmax;
	switch (axis){
		case 1:
			std::sort(start, end, compareBoxesX);
			rmin = root->minx;
			rmax = root->maxx;
			break;
		case 2:
			std::sort(start, end, compareBoxesY);
			rmin = root->miny;
			rmax = root->maxy;
			break;
		default:
			std::sort(start, end, compareBoxesZ);
			rmin = root->minz;
			rmax = root->maxz;
			break;
	}
	const float Ctri = 3;
	const float Cbox = 1;
	//float costNoSplit = Cbox + boxes->size() * Ctri;
	bool costSet = false;
	Split minCost;
	//minCost.cost = costNoSplit;
	minCost.cost = -1;
	minCost.rightBox = 0;
	minCost.leftBox = 0;
	minCost.nchildrenLeft = -1;
	minCost.nchildrenRight = -1;
	float nplanes = 10;
	float delta = (rmax - rmin) / (nplanes + 1);
	float splitAxisPos = rmin + delta;
	bool tooSmall = false;
	if((rmax - rmin) < 0.001 ){
		tooSmall = true;
	}
	else{
		while(splitAxisPos < rmax){
			BBox *leftBox = new BBox();
			BBox *rightBox = new BBox();
			int nchildrenLeft = 0;
			std::vector<BBox*>::iterator p;
			for(p = start; p != end; p++){
				BBox *box = *p;
				float min = axis == 1 ? box->minx : (axis == 2 ? box->miny : box->minz);
				float max = axis == 1 ? box->maxx : (axis == 2 ? box->maxy : box->maxz);
				float center = (max - min) / 2.0 + min;
				if(center > splitAxisPos){
					break;
				}
				nchildrenLeft++;
			}
			//p is now pointing to the first box on the right side
			int nchildrenRight = nboxes - nchildrenLeft;
			setDimensions(boxes, leftBox, start, p);
			setDimensions(boxes, rightBox, p, end);
			leftBox->nchildren = nchildrenLeft;
			rightBox->nchildren = nchildrenRight;
			float A1 = leftBox->surfaceArea();
			float A2 = rightBox->surfaceArea();
			float Ac = root->surfaceArea();
			float N1 = nchildrenLeft;
			float N2 = nchildrenRight;
			float cost = 2 * Cbox + (A1 / Ac) * N1 * Ctri + (A2 / Ac) * N2 * Ctri;
			if(cost < minCost.cost || !costSet){
				delete minCost.leftBox;
				delete minCost.rightBox;
				minCost.cost = cost;
				minCost.leftBox = leftBox;
				minCost.rightBox = rightBox;
				minCost.nchildrenLeft = nchildrenLeft;
				minCost.nchildrenRight = nchildrenRight;
				minCost.split = p;
				costSet = true;
			}
			else{
				delete leftBox;
				delete rightBox;
			}
			splitAxisPos += delta;
		}
	}
	//minCost now contains information about the "best" splitting
	if(minCost.nchildrenLeft == 0 || minCost.nchildrenRight == 0 || tooSmall){
		//a split has not been done
		delete minCost.leftBox;
		delete minCost.rightBox;
		switch (axis){
				case 1:
					divide(root, boxes, start, end, nboxes, axis, false, s, true, ytried, ztried);
					break;
				case 2:
					divide(root, boxes, start, end, nboxes, axis, false, s, xtried, true, ztried);
					break;
				default:
					divide(root, boxes, start, end, nboxes, axis, false, s, xtried, ytried, true);
					break;
		}
	}
	else{
		//root->print(s);
		//printf((s + "nBoxes: %u\n").c_str(), nboxes);
		//fflush(stdout);
		root->children = new BBox*[2];
		root->nchildren = 2;
		totalnBoxes += 2;
		root->children[0] = minCost.leftBox;
		root->children[1] = minCost.rightBox;
		divide(minCost.leftBox, boxes, start, minCost.split, minCost.nchildrenLeft, axis, true, s + tab, false, false, false);
		divide(minCost.rightBox, boxes, minCost.split, end, minCost.nchildrenRight, axis, true, s + tab, false, false, false);
	}

}

void
BVH::setDimensions(std::vector<BBox*> *boxes, BBox *current, std::vector<BBox*>::iterator start, std::vector<BBox*>::iterator end){
	if(start == end){
		return;
	}
	BBox *box = *start;
	current->minx = box->minx;
	current->miny = box->miny;
	current->minz = box->minz;
	current->maxx = box->maxx;
	current->maxy = box->maxy;
	current->maxz = box->maxz;
	std::vector<BBox*>::iterator p;
	for(p = start; p != end; p++){
		current->minx = std::min((*p)->minx, current->minx);
		current->miny = std::min((*p)->miny, current->miny);
		current->minz = std::min((*p)->minz, current->minz);
		current->maxx = std::max((*p)->maxx, current->maxx);
		current->maxy = std::max((*p)->maxy, current->maxy);
		current->maxz = std::max((*p)->maxz, current->maxz);
	}
}


void
BVH::build(std::vector<Triangle*> * objs)
{
    // construct the bounding volume hierarchy
    std::vector<BBox*> *boxes = new std::vector<BBox*>(objs->size());
    printf("Size: %i\n", (int)objs->size());
    fflush(stdout);
    root = new BBox();
    //root->nchildren = objs->size();
    for (size_t i = 0; i < objs->size(); ++i){
    		Triangle *tri = (*objs)[i];
    		float *dim; //[minx, miny, minz, maxx, maxy, maxz]
    		tri->getDimensions(dim);
    		BBox *box = new BBox();
    		box->minx = dim[0];
    		box->miny = dim[1];
    		box->minz = dim[2];
    		box->maxx = dim[3];
    		box->maxy = dim[4];
    		box->maxz = dim[5];
    		box->triangle = tri;
    		boxes->at(i) = box;
    		//box->print();
    		delete[] dim;
    		/*
    		root->minx = std::min(box->minx, root->minx);
    		root->miny = std::min(box->miny, root->miny);
    		root->minz = std::min(box->minz, root->minz);
    		root->maxx = std::max(box->maxx, root->maxx);
    		root->maxy = std::max(box->maxy, root->maxy);
    		root->maxz = std::max(box->maxz, root->maxz);
    		*/
    }
    printf("\n");
    fflush(stdout);
    std::vector<BBox*>::iterator start = boxes->begin();
    std::vector<BBox*>::iterator end = boxes->end();
    totalnBoxes++;
    divide(root, boxes, start, end, objs->size(), 0, true, "", false, false, false);
}

bool
BVH::intersect(HitInfo& minHit, const Ray& ray, float tMin, float tMax) const
{
    // Here you would need to traverse the BVH to perform ray-intersection
    // acceleration. For now we just intersect every object.

    bool hit = false;
    HitInfo tempMinHit;
    minHit.t = MIRO_TMAX;
    
    if(root->intersect(tempMinHit, ray, tMin, tMax)){
		if (tempMinHit.t < minHit.t){
			hit = true;
			minHit = tempMinHit;
		}
    }
    return hit;
}

