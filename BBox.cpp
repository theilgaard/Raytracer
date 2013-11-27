//#include "windows.h"

#include "BBox.h"
#include "Ray.h"


BBox::BBox(){

}

bool
BBox::intersect(const Ray& r, float tMin, float tMax)
{
	boxints++;
	float tmin, tmax, tymin, tymax, tzmin, tzmax;
	tmin = (bounds[r.sign[0]].x - r.o.x) * r.inv.x;
	tmax = (bounds[1 - r.sign[0]].x - r.o.x) * r.inv.x;
	tymin = (bounds[r.sign[1]].y - r.o.y) * r.inv.y;
	tymax = (bounds[1 - r.sign[1]].y - r.o.y) * r.inv.y;
	if ((tmin > tymax) || (tymin > tmax))
		return false;
	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;
	tzmin = (bounds[r.sign[2]].z - r.o.z) * r.inv.z;
	tzmax = (bounds[1 - r.sign[2]].z - r.o.z) * r.inv.z;
	if ((tmin > tzmax) || (tzmin > tmax))
		return false;
	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;
	//if (tmin > tMin) tMin = tmin;
	//if (tmax < tMax) tMax = tmax;
	return true;
}

void BBox::calcDimensions(Objects * objs){
	bounds[0] = Vector3(INFINITY, INFINITY, INFINITY);
	bounds[1] = Vector3(-INFINITY, -INFINITY, -INFINITY);
	for (int i = firstElement; i <= lastElement; i++) {
		bounds[0].x = bounds[0].x < (*objs)[i]->min.x ? bounds[0].x : (*objs)[i]->min.x;
		bounds[0].y = bounds[0].y < (*objs)[i]->min.y ? bounds[0].y : (*objs)[i]->min.y;
		bounds[0].z = bounds[0].z < (*objs)[i]->min.z ? bounds[0].z : (*objs)[i]->min.z;
		bounds[1].x = bounds[1].x > (*objs)[i]->max.x ? bounds[1].x : (*objs)[i]->max.x;
		bounds[1].y = bounds[1].y > (*objs)[i]->max.y ? bounds[1].y : (*objs)[i]->max.y;
		bounds[1].z = bounds[1].z > (*objs)[i]->max.z ? bounds[1].z : (*objs)[i]->max.z;
	}
}

float BBox::surfaceArea(){
	return 2 * ((bounds[1].x - bounds[0].x)*(bounds[1].y - bounds[0].y)
			 + (bounds[1].y - bounds[0].y)*(bounds[1].z - bounds[0].z)
			 + (bounds[1].x - bounds[0].x)*(bounds[1].z - bounds[0].z));
}

float BBox::getbboxCost() {
	return OBJECTCOST*(lastElement - firstElement);
}

void BBox::draw(bool draw){
	if(draw){
		glBegin(GL_QUADS);
			glColor3f(1.0,0,0);
			glVertex3f(bounds[0].x, bounds[0].y, bounds[0].z);
			glVertex3f(bounds[1].x, bounds[0].y, bounds[0].z);
			glVertex3f(bounds[1].x, bounds[1].y, bounds[0].z);
			glVertex3f(bounds[0].x, bounds[1].y, bounds[0].z);

			glVertex3f(bounds[0].x, bounds[0].y, bounds[1].z);
			glVertex3f(bounds[1].x, bounds[0].y, bounds[1].z);
			glVertex3f(bounds[1].x, bounds[1].y, bounds[1].z);
			glVertex3f(bounds[0].x, bounds[1].y, bounds[1].z);

			glVertex3f(bounds[0].x, bounds[0].y, bounds[0].z);
			glVertex3f(bounds[0].x, bounds[0].y, bounds[1].z);
			glVertex3f(bounds[0].x, bounds[1].y, bounds[1].z);
			glVertex3f(bounds[0].x, bounds[1].y, bounds[0].z);

			glVertex3f(bounds[1].x, bounds[0].y, bounds[0].z);
			glVertex3f(bounds[1].x, bounds[0].y, bounds[1].z);
			glVertex3f(bounds[1].x, bounds[1].y, bounds[1].z);
			glVertex3f(bounds[1].x, bounds[1].y, bounds[0].z);
		glEnd();
	}
	if (!(this->isLeaf)){
		child1->draw(draw);
		child2->draw(draw);
	}
}

void BBox::print(std::string s){
	printf((s + "Box: min: %f %f %f max: %f %f %f\n").c_str(), bounds[0].x, bounds[0].y, bounds[0].z, bounds[1].x, bounds[1].y, bounds[1].z);
	fflush(stdout);
}
