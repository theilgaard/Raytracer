#ifndef BBOX_H_
#define BBOX_H_

#include "Triangle.h"

extern int boxints;
const float BOXCOST = 6.0f;
const float OBJECTCOST = 18.0f;

class BBox {
public:
	BBox();
	Vector3 bounds[2];
	Vector4 bounds4D[2];
	int axis, firstElement, lastElement;
	bool isLeaf, isTimesplit;
	BBox *child1, *child2, *parent;
	bool intersect(const Ray& ray, float tmin, float tmax);
	bool intersect4D(const Ray& ray, float tmin, float tmax);
	void calcDimensions(Objects * objs);
	void calcDimensions4D(Objects * objs, float sT, float eT);
	float surfaceArea();
	float surfaceArea4D();
	float getbboxCost();
	void draw(bool draw);
	void print(std::string s = "");
};

#endif /* BBOX_H_ */