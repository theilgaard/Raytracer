#ifndef CSE168_OBJECT_H_INCLUDED
#define CSE168_OBJECT_H_INCLUDED

#include <vector>
#include "Miro.h"
#include "ShadingModel.h"

class Object
{
public:
    Object() {}
    virtual ~Object() {}
	Vector3 max;
	Vector3 min;
	Vector3 centroid;

    void setMaterial(ShadingModel* m) {m_material = m;}

    virtual void renderGL() {}
    virtual void preCalc() {}
	
	virtual void interpolate(float time) {}

    virtual bool intersect(HitInfo& result, const Ray& ray,
                           float tMin = 0.0f, float tMax = MIRO_TMAX) = 0;

protected:
    ShadingModel* m_material;
	virtual void reCalc() {}
};

typedef std::vector<Object*> Objects;

#endif // CSE168_OBJECT_H_INCLUDED
