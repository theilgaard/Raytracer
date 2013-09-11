#ifndef CSE168_TRIANGLE_H_INCLUDED
#define CSE168_TRIANGLE_H_INCLUDED

#include "Object.h"
#include "TriangleMesh.h"

extern int triangleints;

/*
    The Triangle class stores a pointer to a mesh and an index into its
    triangle array. The mesh stores all data needed by this Triangle.
*/
class Triangle : public Object
{
public:
    Triangle(TriangleMesh * m = 0, unsigned int i = 0);
    virtual ~Triangle();

    void setIndex(unsigned int i) {m_index = i;}
    void setMesh(TriangleMesh* m) {m_mesh = m;}

    virtual void renderGL();
    virtual bool intersect(HitInfo& result, const Ray& ray,
                           float tMin = 0.0f, float tMax = MIRO_TMAX);
    virtual void preCalc();
protected:
	TriangleMesh::TupleI3 ti3;
	TriangleMesh::TupleI3 ti3n;
	Vector3 v0, v1, v2,
			n0, n1, n2;
	Vector3 n;
	Vector3 k_ax;
	Vector3 k_ay;
	Vector3 k_az;
	Vector3 k_bx;
	Vector3 k_by;
	Vector3 k_bz;
	Vector3 k_cx;
	Vector3 k_cy;
	Vector3 k_cz;
    TriangleMesh* m_mesh;
    unsigned int m_index;
};

#endif // CSE168_TRIANGLE_H_INCLUDED
