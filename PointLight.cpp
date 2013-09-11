#include "PointLight.h"
#include "Scene.h"
#include <stdlib.h>

void PointLight::emitPhotons(const int nPhotons, const Scene& scene, PhotonMap& pMap, Vector3 lFocusPos, float focusSphereR){
	float photonPower;
	Vector3 d;
	Vector3 rd, u, v;
	if(focusSphereR != 0.0){
		rd = m_position - lFocusPos; //vector from focus sphere to light source
		u = rd.perpendicular();
		v = cross(rd, u);
		u.normalize();
		v.normalize();
	}
	return;
	//for(int i = 0; i < nPhotons; i++){
	while(pMap.getStoredPhotonsNumber() < nPhotons){
		if(focusSphereR != 0.0){
			Vector3 p;
			do{
				 float x = focusSphereR - focusSphereR * 2 * (rand() / (float)RAND_MAX);
				 float y = focusSphereR - focusSphereR * 2 * (rand() / (float)RAND_MAX);
				 p = u * x + v * y;
			 } while (p.length() > focusSphereR);
			d = (lFocusPos + p) - m_position;
			d.normalize();
			float circleArea = PI * focusSphereR * focusSphereR;
			float saLightSource = 4 * PI * rd.length(); //surface area of the big sphere around light source
			float sphereVis = circleArea / saLightSource; //an estimate for how visible the focus sphere is from the light source
			photonPower = (m_wattage * sphereVis) / nPhotons;
		}
		else{
			//Find random direction
			do {
				 d.x = 1.0 - 2.0 * (rand() / (float)RAND_MAX);
				 d.y = 1.0 - 2.0 * (rand() / (float)RAND_MAX);
				 d.z = 1.0 - 2.0 * (rand() / (float)RAND_MAX);
			} while(dot(d,d) > 1.0);
			d.normalize();
			photonPower = m_wattage / nPhotons;
		}

		Ray photon(m_position, d, 1.00029);
		HitInfo hitInfo;
		if(scene.trace(hitInfo, photon, 0.001)){
			Vector3 power = Vector3( m_color.x * photonPower, m_color.y * photonPower, m_color.z * photonPower);
			hitInfo.material->interact(photon, hitInfo, scene, 0, power, pMap);
		}
	}

}
