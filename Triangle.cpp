//#include "windows.h"

#include "Triangle.h"
#include "TriangleMesh.h"
#include "Ray.h"
#include <iostream>
using namespace std;

Triangle::Triangle(TriangleMesh * m, unsigned int i) :
	m_mesh(m), m_index(i), v0(Vector3(0.0f)), v1(Vector3(0.0f)), v2(Vector3(0.0f)),
	n0(Vector3(0.0f)), n1(Vector3(0.0f)), n2(Vector3(0.0f))
{

}


Triangle::~Triangle()
{

}


void
Triangle::renderGL()
{
    TriangleMesh::TupleI3 ti3 = m_mesh->vIndices()[m_index];
    const Vector3 & v0 = m_mesh->vertices()[ti3.x]; //vertex a of triangle
    const Vector3 & v1 = m_mesh->vertices()[ti3.y]; //vertex b of triangle
    const Vector3 & v2 = m_mesh->vertices()[ti3.z]; //vertex c of triangle

    glBegin(GL_TRIANGLES);
        glVertex3f(v0.x, v0.y, v0.z);
        glVertex3f(v1.x, v1.y, v1.z);
        glVertex3f(v2.x, v2.y, v2.z);
    glEnd();
}



bool
Triangle::intersect(HitInfo& result, const Ray& r, float tMin, float tMax)
{
	triangleints++;
	bool positiveSign;
	float V_a =
		r.d.x*k_ax.x + r.dxoy*k_ax.y + r.dxoz*k_ax.z
		+ r.d.y*k_ay.x + r.dyoz*k_ay.y + r.dyox*k_ay.z
		+ r.d.z*k_az.x + r.dzox*k_az.y + r.dzoy*k_az.z;
	positiveSign = (V_a >= 0);
	//if(V_a < 0)
	//  return false;

	float V_b =
		r.d.x*k_bx.x + r.dxoy*k_bx.y + r.dxoz*k_bx.z
		+ r.d.y*k_by.x + r.dyoz*k_by.y + r.dyox*k_by.z
		+ r.d.z*k_bz.x + r.dzox*k_bz.y + r.dzoy*k_bz.z;
	if ((V_b < 0 && positiveSign) || (V_b >= 0 && !positiveSign))
		return false;

	float V_c =
		r.d.x*k_cx.x + r.dxoy*k_cx.y + r.dxoz*k_cx.z
		+ r.d.y*k_cy.x + r.dyoz*k_cy.y + r.dyox*k_cy.z
		+ r.d.z*k_cz.x + r.dzox*k_cz.y + r.dzoy*k_cz.z;
	if ((V_c < 0 && positiveSign) || (V_c >= 0 && !positiveSign))
		return false;

	float invSignSum = 1.0f / (V_a + V_b + V_c);
	float alpha = V_a*invSignSum;
	float beta = V_b*invSignSum;
	float gamma = V_c*invSignSum;

	result.P = alpha*v0 + beta*v1 + gamma*v2;
	float t = dot((result.P - r.o), r.d);
	if (t < tMin || t > tMax)
		return false;

	result.t = t;
	result.P = r.o + t * r.d; //alpha * v0 + beta * v1 + gamma * v2;
	result.N = alpha * n0 + beta * n1 + gamma * n2; //shading normal
	result.N.normalize();
	result.material = this->m_material;
	//printf("%p\n", result.material);
	//fflush(stdout);
    return true;
}

void Triangle::preCalc(){
	ti3 = m_mesh->vIndices()[m_index];
	ti3n = m_mesh->nIndices()[m_index];
	v0 = m_mesh->vertices()[ti3.x]; //vertex a of triangle
	v1 = m_mesh->vertices()[ti3.y]; //vertex b of triangle
	v2 = m_mesh->vertices()[ti3.z]; //vertex c of triangle

	n0 = m_mesh->normals()[ti3n.x]; //normals a of triangle
	n1 = m_mesh->normals()[ti3n.y]; //normals b of triangle
	n2 = m_mesh->normals()[ti3n.z]; //normals c of triangle

	n = cross((v1 - v0), (v2 - v0)); //geometric normal of plane/triangle

	max = Vector3(std::max(v0.x, std::max(v1.x, v2.x)),
				  std::max(v0.y, std::max(v1.y, v2.y)),
				  std::max(v0.z, std::max(v1.z, v2.z)));

	min = Vector3(std::min(v0.x, std::min(v1.x, v2.x)),
				  std::min(v0.y, std::min(v1.y, v2.y)),
				  std::min(v0.z, std::min(v1.z, v2.z)));

	//center of the bounding box:
	centroid = Vector3((max.x + min.x) / 2, (max.y + min.y) / 2, (max.z + min.z) / 2);

	// constants for doing ray/triangle intersection with signed volumes
	k_ax = Vector3(v1.y*v2.z - v1.z*v2.y, v1.z - v2.z, v2.y - v1.y)*(-1.0f / 6.0f);
	k_ay = Vector3(v1.z*v2.x - v1.x*v2.z, v1.x - v2.x, v2.z - v1.z)*(-1.0f / 6.0f);
	k_az = Vector3(v1.x*v2.y - v1.y*v2.x, v1.y - v2.y, v2.x - v1.x)*(-1.0f / 6.0f);

	k_bx = Vector3(v2.y*v0.z - v2.z*v0.y, v2.z - v0.z, v0.y - v2.y)*(-1.0f / 6.0f);
	k_by = Vector3(v2.z*v0.x - v2.x*v0.z, v2.x - v0.x, v0.z - v2.z)*(-1.0f / 6.0f);
	k_bz = Vector3(v2.x*v0.y - v2.y*v0.x, v2.y - v0.y, v0.x - v2.x)*(-1.0f / 6.0f);

	k_cx = Vector3(v0.y*v1.z - v0.z*v1.y, v0.z - v1.z, v1.y - v0.y)*(-1.0f / 6.0f);
	k_cy = Vector3(v0.z*v1.x - v0.x*v1.z, v0.x - v1.x, v1.z - v0.z)*(-1.0f / 6.0f);
	k_cz = Vector3(v0.x*v1.y - v0.y*v1.x, v0.y - v1.y, v1.x - v0.x)*(-1.0f / 6.0f);

}
