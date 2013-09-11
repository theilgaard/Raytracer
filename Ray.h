#ifndef CSE168_RAY_H_INCLUDED
#define CSE168_RAY_H_INCLUDED

#include "Vector3.h"
#include "ShadingModel.h"

class Ray
{
public:
	Vector3 o,      //!< Origin of ray
			d,      //!< Direction of ray
			inv;	//!< Inverse direction of ray
    float eta;
	int sign[3];
	float	dxoy, dxoz,
			dyoz, dyox,
			dzox, dzoy;

	Ray() : o(), d(Vector3(0.0f, 0.0f, 1.0f)), eta(-2)
	{
		inv = Vector3(1.0f) / d;
		sign[0] = (inv.x < 0);
		sign[1] = (inv.y < 0);
		sign[2] = (inv.z < 0);
		dxoy = d.x*o.y;
		dxoz = d.x*o.z;
		dyoz = d.y*o.z;
		dyox = d.y*o.x;
		dzox = d.z*o.x;
		dzoy = d.z*o.y;
    }

    Ray(const Vector3& o, const Vector3& d, float eta) : o(o), d(d), eta(eta)
    {
		inv = Vector3(1.0f) / d;
		sign[0] = (inv.x < 0);
		sign[1] = (inv.y < 0);
		sign[2] = (inv.z < 0);
		dxoy = d.x*o.y;
		dxoz = d.x*o.z;
		dyoz = d.y*o.z;
		dyox = d.y*o.x;
		dzox = d.z*o.x;
		dzoy = d.z*o.y;
    }
};


//! Contains information about a ray hit with a surface.
/*!
    HitInfos are used by object intersection routines. They are useful in
    order to return more than just the hit distance.
*/
class HitInfo
{
public:
    float t;                            //!< The hit distance
    Vector3 P;                          //!< The hit point
    Vector3 N;                          //!< Shading normal vector
    ShadingModel* material;           //!< Material of the intersected object

    //! Default constructor.
    explicit HitInfo(float t = 0.0f,
                     const Vector3& P = Vector3(),
                     const Vector3& N = Vector3(0.0f, 1.0f, 0.0f)) :
        t(t), P(P), N(N), material (0)
    {
        // empty
    }
};

#endif // CSE168_RAY_H_INCLUDED
