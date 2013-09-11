#ifndef Refraction_H_
#define Refraction_H_

#include "Material.h"

class PhotonMap;
extern const int maxPhotonBounces;

class ShadingModel : public Material
{
public:

	ShadingModel(const Vector3 & ka, float eta, const Vector3 & Rd, const Vector3 & Rs, const Vector3 & Rt);
    virtual ~ShadingModel();

    const Vector3 & kd() const {return Rd;}
    const Vector3 & ka() const {return m_ka;}

    void setKd(const Vector3 & kd) {Rd = kd;}
    void setKa(const Vector3 & ka) {m_ka = ka;}

    virtual void preCalc() {}

    virtual Vector3 shade(const Ray& ray, const HitInfo& hit,
                          const Scene& scene, int bounce, PhotonMap& pMap);

    void interact(const Ray& photon, const HitInfo& hit, const Scene& scene,
    								int bounce, Vector3 pPower, PhotonMap& pMap);

protected:
    Vector3 Rd, Rs, Rt, m_ka;
    const int maxBounces;
    const float eta;
    float outsideEta;
    Vector3 estimateRadiance(const HitInfo& hit, PhotonMap& pMap);
    Vector3 sampleRadiance(const HitInfo& hit, const Scene& scene, int bounce, PhotonMap& pMap);
};

#endif /* Refraction_H_ */

