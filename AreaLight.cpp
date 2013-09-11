#include "AreaLight.h"
#include "Scene.h"
#include <stdlib.h>

void AreaLight::emitPhotons(const int nPhotons, const Scene& scene, PhotonMap& pMap){
	for(int i = 0; i < nPhotons; i++){
		Vector3 o = generateRandomPosition();
		//Find random direction
		float u = rand() / (float)RAND_MAX;
		float v = 2 * PI * (rand() / (float)RAND_MAX);
		float sqrtu = sqrt(u);
		Vector3 d = Vector3(cos(v) * sqrtu, sin(v) * sqrtu, sqrt(1 - u));
		d.normalize();
		Ray photon(o, d, 1.00029);
		HitInfo hitInfo;
		if(scene.trace(hitInfo, photon, 0.001)){
			//float photonPower = m_wattage / nPhotons;
			Vector3 power = Vector3( m_color.x * m_wattage, m_color.y * m_wattage, m_color.z * m_wattage);
			hitInfo.material->interact(photon, hitInfo, scene, 0, power, pMap);
		}
	}

}

Vector3 AreaLight::generateRandomPosition(){
	return Vector3(0.0f);
}

