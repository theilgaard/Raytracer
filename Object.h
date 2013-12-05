#ifndef CSE168_OBJECT_H_INCLUDED
#define CSE168_OBJECT_H_INCLUDED

#include <vector>
#include "Miro.h"
#include "ShadingModel.h"
#include "Matrix4x4.h"

class Object
{
public:
    Object() {}
    virtual ~Object() { }
	Vector3 max;
	Vector3 min;
	Vector3 centroid;

	
	void setFutureMatrix(Matrix4x4* m) { m_fm = m; }
    void setMaterial(ShadingModel* m) {m_material = m;}

    virtual void renderGL() {}
    virtual void preCalc() {}


    virtual bool intersect(HitInfo& result, const Ray& ray,
                           float tMin = 0.0f, float tMax = MIRO_TMAX) = 0;
	virtual bool intersectAnimated(HitInfo& result, const Ray& ray,
                           float tMin = 0.0f, float tMax = MIRO_TMAX) = 0;

protected:
    ShadingModel* m_material;
	Matrix4x4* m_fm;
};

typedef std::vector<Object*> Objects;

#endif // CSE168_OBJECT_H_INCLUDED
