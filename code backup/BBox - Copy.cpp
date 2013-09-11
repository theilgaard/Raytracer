//#include "windows.h"

#include "BBox.h"
#include "Ray.h"

BBox::BBox() :
	triangle(0), children(0), nchildren(0), minx(0.0f), miny(0.0f), minz(0.0f), maxx(0.0f), maxy(0.0f), maxz(0.0f)
{

}

bool
BBox::intersect(HitInfo& minHit, const Ray& r, float tMin, float tMax)
{
	boxints++;
	bool inside = false;
	bool hit = false;
	float tmin = tMin, tmax = tMax;
	// Always hit if origin is inside box
	if (r.o.x < maxx && r.o.y < maxy && r.o.z < maxz && r.o.x > minx && r.o.y > miny && r.o.z > minz){
		inside = true;
	}else{
		float txmin = (minx - r.o.x) / r.d.x;
		float txmax = (maxx - r.o.x) / r.d.x;
		float tymin = (miny - r.o.y) / r.d.y;
		float tymax = (maxy - r.o.y) / r.d.y;
		float tzmin = (minz - r.o.z) / r.d.z;
		float tzmax = (maxz - r.o.z) / r.d.z;

		tmin = std::max(std::max(std::min(txmin, txmax), std::min(tymin, tymax)), std::min(tzmin, tzmax));
		tmax = std::min(std::min(std::max(txmin, txmax), std::max(tymin, tymax)), std::max(tzmin, tzmax));
	}
	if( inside || ((tmin <= tmax) && !(tmax < 0)) ){
		//ray intersects the box
		HitInfo tempMinHit;
		minHit.t = MIRO_TMAX;
		if(triangle != 0){ //leaf
			if (triangle->intersect(tempMinHit, r, tmin, tmax)){
				hit = true;
				if (tempMinHit.t < minHit.t){
					minHit = tempMinHit;
				}
//				printf("%p\n", minHit.material);
//				fflush(stdout);
			}
		}else{
			//put this on stack instead!!!!!!!!!!!!!!!!!!!!!!!!!

			for(size_t j = 0; j < nchildren; j++){
				if(children[j] == 0){
					continue;
				}
				else if (children[j]->intersect(tempMinHit, r, tmin, tmax)){
					hit = true;
					if (tempMinHit.t < minHit.t){
						minHit = tempMinHit;
					}
				}
			}
		}
	}
	return hit;
}

void BBox::calcDimensions(){
	if(children == 0){
		return;
	}
	for (size_t i = 0; i < 2; ++i){
		minx = std::min(children[i]->minx, minx);
		miny = std::min(children[i]->miny, miny);
		minz = std::min(children[i]->minz, minz);
		maxx = std::max(children[i]->maxx, maxx);
		maxy = std::max(children[i]->maxy, maxy);
		maxz = std::max(children[i]->maxz, maxz);
	}
}

float BBox::surfaceArea(){
	float x = (maxx - minx);
	float y = (maxy - miny);
	float z = (maxz - minz);
	return x * y * 2 + z * y * 2 + x * z * 2;
}

void BBox::draw(bool draw){
	if(draw){
		glBegin(GL_QUADS);
			glColor3f(1.0,0,0);
			glVertex3f(minx, miny, minz);
			glVertex3f(maxx, miny, minz);
			glVertex3f(maxx, maxy, minz);
			glVertex3f(minx, maxy, minz);

			glVertex3f(minx, miny, maxz);
			glVertex3f(maxx, miny, maxz);
			glVertex3f(maxx, maxy, maxz);
			glVertex3f(minx, maxy, maxz);

			glVertex3f(minx, miny, minz);
			glVertex3f(minx, miny, maxz);
			glVertex3f(minx, maxy, maxz);
			glVertex3f(minx, maxy, minz);

			glVertex3f(maxx, miny, minz);
			glVertex3f(maxx, miny, maxz);
			glVertex3f(maxx, maxy, maxz);
			glVertex3f(maxx, maxy, minz);
		glEnd();
	}
	for(size_t j = 0; j < nchildren; j++){
		children[j]->draw(true);
	}
}

void BBox::print(std::string s){
	printf((s + "Box: min: %f %f %f max: %f %f %f\n").c_str(), minx, miny, minz, maxx, maxy, maxz);
	fflush(stdout);
}
