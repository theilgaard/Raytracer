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
	int axis, firstElement, lastElement;
	bool isLeaf;
	BBox *child1, *child2, *parent;
	bool intersect(const Ray& ray, float tmin, float tmax);
	void calcDimensions(Objects * objs);
	float surfaceArea();
	float getbboxCost();
	void draw(bool draw);
	void print(std::string s = "");
};

#endif /* BBOX_H_ */