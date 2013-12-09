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
	Triangle() : m_mesh(0),	m_mesh2(0), v0(Vector3(0.0f)), v1(Vector3(0.0f)), v2(Vector3(0.0f)),
		n0(Vector3(0.0f)), n1(Vector3(0.0f)), n2(Vector3(0.0f)) {};
	Triangle(TriangleMesh * m, unsigned int i = 0);
    Triangle(TriangleMesh * m, TriangleMesh * m2 = 0, unsigned int i = 0);
    virtual ~Triangle();

    void setIndex(unsigned int i) {m_index = i;}
    void setMesh(TriangleMesh* m) {m_mesh = m;}
	void setMesh2(TriangleMesh* m) {m_mesh2 = m;}

	TriangleMesh* getMesh() { return m_mesh; }
	TriangleMesh* getMesh2() { return m_mesh2; }

    virtual void renderGL();
    virtual bool intersectAnimated(HitInfo& result, const Ray& ray,
                           float tMin = 0.0f, float tMax = MIRO_TMAX);
    virtual bool intersect(HitInfo& result, const Ray& ray,
                           float tMin = 0.0f, float tMax = MIRO_TMAX);
    virtual void preCalc();
protected:
	TriangleMesh::TupleI3 ti3;
	TriangleMesh::TupleI3 ti3n;
	TriangleMesh::TupleI3 ti3_2;
	TriangleMesh::TupleI3 ti3n_2;
	Vector3 v0, v1, v2,
			n0, n1, n2,
			vf0, vf1, vf2,
			nf0, nf1, nf2;
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
	TriangleMesh* m_mesh2;
    unsigned int m_index;
};

#endif // CSE168_TRIANGLE_H_INCLUDED
