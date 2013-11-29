#include "ShadingModel.h"
#include "Ray.h"
#include "Scene.h"
#include "PointLight.h"
#include "RectangleLight.h"
#include <cstdlib>

#include <iostream>
using namespace std;

ShadingModel::ShadingModel(const Vector3 & ambient, float refrationIndex, const Vector3 & diffuse, const Vector3 & specular, const Vector3 & refrationcolor) :
	m_ka(ambient), maxBounces(3), eta(refrationIndex), outsideEta(1.00029f), Rd(diffuse), Rs(specular), Rt(refrationcolor)
{

}

ShadingModel::~ShadingModel()
{
}

//rSchlick2 - Reflectance approximation
float reflectance(const Vector3& normal, const Vector3 incident, float n1, float n2, float cosI){
	float r0 = (n1 - n2) / (n1 + n2);
	r0 *= r0;
	float cosX = -1 * dot(normal, incident);
	if (n1 > n2){ // Inside, going out
		float n = n1 / n2;
		float sinT2 = n * n * (1.0 - cosI * cosI);
		if (sinT2 > 1.0) return 1.0; // Total internal reflection
		cosX = sqrtf(1.0 - sinT2);
	}
	float x = 1.0 - cosX;
	float kr = r0 + (1.0 - r0) * x * x * x * x * x;
	if (kr > 1.0) return 1.0;
	if (kr < 0.0) return 0.0;
	return kr;
}

Vector3	ShadingModel::shade(const Ray& ray, const HitInfo& hit, const Scene& scene, int bounce, PhotonMap& pMap)
{
	Vector3 L = Vector3(0.0f, 0.0f, 0.0f);
	const Vector3 viewDir = -ray.d; // d is a unit vector
	Vector3 reflDir, refrDir, N = hit.N;
	bool inside = false;
	if (dot(ray.d, hit.N) > 0.0){
		N = -1 * hit.N;
		inside = true;
	}

	// Reflection and refraction
	if ((Rs != Vector3(0.0f) || Rt != Vector3(0.0f)) && bounce < maxBounces){
		float n1, n2, n;
		// Check if we are inside. 

		n1 = 1.00029; // Air
		n2 = eta;
		n = !inside ? n1 / n2 : n2 / n1;
		float cosI = -1 * dot(ray.d, N);
		double Kr = !inside ? reflectance(N, ray.d, n1, n2, cosI) : reflectance(N, ray.d, n2, n1, cosI);
		// === Reflection === 
		if (Rs != Vector3(0.0f)){
			reflDir = ray.d + 2 * cosI * N;
			// Trace the ray
			Ray refL(hit.P, reflDir, eta);
			HitInfo specHitInfo;
			if (scene.trace(specHitInfo, refL, 0.00029)){
				Vector3 color = specHitInfo.material->shade(refL, specHitInfo, scene, bounce + 1, pMap);
				if (Rt != Vector3(0.0f)) // If refracted, use reflectance 
					L += Kr * color * Rs; 
				else
					L += color * Rs;
			}
		}
		// === Refraction ===
		if (Rt != Vector3(0.0f)){
			float sinT2 = (n  * n) * (1.0 - cosI * cosI);
			if (sinT2 > 1.0){
				return L; // Total internal reflection
			}
			float cosT = sqrtf(1.0 - sinT2);
			refrDir = n * ray.d + (n * cosI - cosT) * N;
			refrDir.normalize();
			// Trace the ray
			Ray refrR(hit.P, refrDir, eta);
			HitInfo hitInfo;
			if (scene.trace(hitInfo, refrR, 0.00029)){
				Vector3 color = hitInfo.material->shade(refrR, hitInfo, scene, bounce + 1, pMap);
				float dist = (hit.P - ray.o).length();
				Vector3 absorbance = color * 0.2 * -dist;
				Vector3 transparency = Vector3(expf(absorbance.x),
											   expf(absorbance.y),
											   expf(absorbance.z));
				if (!inside){ // Beer's Law.
					if (Rs != Vector3(0.0f)) // If reflected, use reflectance 
						L += (1.0 - Kr) * color * Rt * transparency;
					else
						L += color * Rt * transparency;
				}
				else{
					if (Rs != Vector3(0.0f)) // If reflected, use reflectance 
						L += (1.0 - Kr) * color * Rt;
					else
						L += color * Rt;
				}

			}
		}
	}
	 // No more reflection/refraction, add the diffuse component!

		// === Direct lighting for Point Lights ===
		const PointLights *lightlist = scene.lights();
		PointLights::const_iterator lightIter;
		for (lightIter = lightlist->begin(); lightIter != lightlist->end(); lightIter++)
		{
			PointLight* light = *lightIter;

			Vector3 l = light->position() - hit.P;

			// the inverse-squared falloff
			float falloff = l.length2(); //gives the length^2 of l which is also r^2

			// normalize the light direction
			float r = sqrtf(falloff);
			l /= r;

			// get the diffuse component
			float nDotL = dot(N, l);

			// Shadow computation
			Vector3 E = std::max(0.0f, (nDotL * light->wattage() / (PI * falloff)));
			Ray shadowCheck(hit.P, l, 0);
			HitInfo temp;
			if (!scene.trace(temp, shadowCheck, 0.0001, r)){
				//if (!(Rs != Vector3(0.0f) || Rt != Vector3(0.0f)) || bounce > maxBounces)
					L += Rd * E * light->color();
				//Phong highlight
				if (Rs != Vector3(0.0f)){
					float exp = 50.0;
					reflDir = -2 * dot(hit.N, ray.d) * hit.N + ray.d;
					float dDotl = dot(l, reflDir);
					if (dDotl > 0.0){
						L += Rs * ((powf(dDotl, exp) * E) / nDotL);
					}
				}
			}
			// else in shadow, and no light from this light.
		}
		// === Direct lighting for Area lights ===
		const RectangleLights *reclightlist = scene.reclights();
		RectangleLights::const_iterator reclightIter;
		for (reclightIter = reclightlist->begin(); reclightIter != reclightlist->end(); reclightIter++)
		{
			RectangleLight* light = *reclightIter;
			int hits = 0;
			int tries = 10;
			for (int i = 0; i < tries; i++){
				Vector3 l = light->generateRandomPosition() - hit.P;
				float r = l.length();
				l /= r;
				Ray shadowCheck(hit.P, l, 0);
				HitInfo temp;
				if (!scene.trace(temp, shadowCheck, 0.0001, r)){
					hits++;
				}
			}

			Vector3 l = light->getCenter() - hit.P;

			// the inverse-squared falloff
			float falloff = l.length2(); //gives the length^2 of l which is also r^2

			// normalize the light direction
			float r = sqrt(falloff);
			l /= r;

			// get the diffuse component
			float nDotL = dot(hit.N, l);
			Vector3 E = std::max(0.0f, nDotL * light->wattage() / (4 * PI * falloff));
			float visibility = (hits / float(tries));
			if (!(Rs != Vector3(0.0f) || Rt != Vector3(0.0f)) || bounce > maxBounces)
				L += visibility  * Rd * E * light->color();
			// Phong highlight
			if (Rs != Vector3(0.0f) && hits != 0){
				float exp = 30.0;
				float cosI = -1 * dot(l, hit.N);
				reflDir = ray.d + 2 * cosI * hit.N;
				float dDotl = dot(N, l);
				if (dDotl > 0){
					L += Rs * ((powf(dDotl, exp) * E) / nDotL);
				}
			}
		}

	// add the ambient component
	 L += m_ka;

    return L;
}



