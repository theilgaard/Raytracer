#include "ShadingModel.h"
#include "Ray.h"
#include "Scene.h"
#include "PointLight.h"
#include "RectangleLight.h"
#include <cstdlib>

#include <iostream>
using namespace std;

ShadingModel::ShadingModel(const Vector3 & ambient, float refrationIndex, const Vector3 & diffuse, const Vector3 & specular, const Vector3 & refrationcolor) :
	m_ka(ambient), maxBounces(6), eta(refrationIndex), outsideEta(1.00029f), Rd(diffuse), Rs(specular), Rt(refrationcolor)
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

Vector3
	ShadingModel::shade(const Ray& ray, const HitInfo& hit, const Scene& scene, int bounce, PhotonMap& pMap)
{
	Vector3 L = Vector3(0.0f, 0.0f, 0.0f);
	const Vector3 viewDir = -ray.d; // d is a unit vector
	Vector3 reflDir, refrDir, N = hit.N;
	bool inside = false;
	if (dot(ray.d, hit.N) > 0.0){
		N = -1 * hit.N;
		inside = true;
	}

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
	 // No more reflection/refraction, return diffuse component!

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
				if (!(Rs != Vector3(0.0f) || Rt != Vector3(0.0f)) || bounce > maxBounces)
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


	// Estimate the radiance from the Photon Map
 	//Vector3 collectedR = estimateRadiance(hit, pMap);
 	//L += Rd * 1/PI * collectedR;

	/*
 	if(Rd != Vector3(0.0f)){
		 //Final gathering using path tracing
		 Vector3 dr;
		 float cosi;
		 do{
			 do{
				 dr.x = 1.0 - 2.0 * (rand() / (float)RAND_MAX);
				 dr.y = 1.0 - 2.0 * (rand() / (float)RAND_MAX);
				 dr.z = 1.0 - 2.0 * (rand() / (float)RAND_MAX);
			 } while(dot(dr,dr) > 1.0);
			 cosi = dot(dr,hit.N);
		 } while(cosi < 0.0);
		 dr.normalize();
		 //cout << dr << endl;
		 Ray pathR(hit.P, dr, outsideEta);
		HitInfo pathHitInfo;
		Vector3 collectedR = estimateRadiance(hit, pMap);

		if(scene.trace(pathHitInfo, pathR, 0.001)){
			//Vector3 pathColor = pathHitInfo.material->shade(pathR, pathHitInfo, scene, bounce, pMap);
			//L += Rd * pathColor * cosi;
			collectedR += pathHitInfo.material->sampleRadiance(pathHitInfo, scene, bounce, pMap);;
		}

		//Vector3 temp = 50000 * collectedR;

	 }
	*/

    return L;
}
/*
Vector3 ShadingModel::estimateRadiance(const HitInfo& hit, PhotonMap& pMap){
	float *pos = new float[3];
	pos[0] = hit.P.x;
	pos[1] = hit.P.y;
	pos[2] = hit.P.z;
	float *normal = new float[3];
	normal[0] = hit.N.x;
	normal[1] = hit.N.y;
	normal[2] = hit.N.z;
	float max_dist = 1;
	int nphotons = 500;
	float *irrad = new float[3];
	pMap.irradiance_estimate(irrad, pos, normal, max_dist, nphotons);
	delete[] pos;
	delete[] normal;
	Vector3 R = Vector3(irrad[0], irrad[1], irrad[2]);
	delete[] irrad;
	return R;
}

Vector3
ShadingModel::sampleRadiance(const HitInfo& hit, const Scene& scene, int bounce, PhotonMap& pMap){
	Vector3 R;
	bounce++;
	if(bounce > maxBounces){
		//cout << "MAXbounces!" << endl;
		return R;
	}
	if(Rd != Vector3(0.0f)){
		 //Path tracing
		 Vector3 dr;
		 float cosi;
		 do{
			 do{
				 dr.x = 1.0 - 2.0 * (rand() / (float)RAND_MAX);
				 dr.y = 1.0 - 2.0 * (rand() / (float)RAND_MAX);
				 dr.z = 1.0 - 2.0 * (rand() / (float)RAND_MAX);
			 } while(dot(dr,dr) > 1.0);
			 cosi = dot(dr,hit.N);
		 } while(cosi < 0.0);
		 dr.normalize();
		 //cout << dr << endl;
		 Ray pathR(hit.P, dr, outsideEta);
		HitInfo pathHitInfo;
		if(scene.trace(pathHitInfo, pathR, 0.001)){
			//Vector3 pathColor = pathHitInfo.material->shade(pathR, pathHitInfo, scene, bounce, pMap);
			//L += Rd * pathColor * cosi;
			R += Rd * estimateRadiance(hit, pMap) + pathHitInfo.material->sampleRadiance(pathHitInfo, scene, bounce, pMap);
		}
	}
	return R;
}


//Used during the first pass
void ShadingModel::interact(const Ray& photon, const HitInfo& hit, const Scene& scene,
								int bounce, Vector3 power, PhotonMap& pMap){

	bounce++;
	if(bounce > maxPhotonBounces){
		//cout << "MAXbounces!" << endl;
		return;
	}

	if(outsideEta == -1.0){
		outsideEta = photon.eta;
	}

	//Photon always stored at diffuse surfaces, except first time
	if(Rd != Vector3(0.0) && bounce != 1){
		float *pos = new float[3];
		float *dir = new float[3];
		pos[0] = hit.P.x;
		pos[1] = hit.P.y;
		pos[2] = hit.P.z;
		dir[0] = photon.d.x;
		dir[1] = photon.d.y;
		dir[2] = photon.d.z;
		float *pArray = new float[3];
		//power = power;
		pArray[0] = power.x;
		pArray[1] = power.y;
		pArray[2] = power.z;
		//cout << power << endl;
		pMap.store(pArray, pos, dir);
	}

	//Decide how the photon should be reflected
	float r = rand() / (float)RAND_MAX; // r is in the range 0 to 1
	float Rdavr = Rd.avr();
	//printf("Rdavr %f\n", Rdavr);
	float Rsavr = Rs.avr();
	float Rtavr = Rt.avr();

	if( r < Rdavr){ //if r is in <0,Rdavr]
		//diffuse reflection

		Vector3 dr;
		float cosi;
		 do{
			 do{
				 dr.x = 1.0 - 2.0 * (rand() / (float)RAND_MAX);
				 dr.y = 1.0 - 2.0 * (rand() / (float)RAND_MAX);
				 dr.z = 1.0 - 2.0 * (rand() / (float)RAND_MAX);
			 } while(dot(dr,dr) > 1.0);
			 cosi = dot(dr,hit.N);
		 } while(cosi < 0.0);
		 dr.normalize();
		 //cout << dr << endl;

		/*
		Vector3 u = hit.N.perpendicular().normalized();
		Vector3 v = cross(hit.N, u).normalized();
		const float phi = 2 * PI* (rand() / (float)RAND_MAX);
		const float theta = asin(sqrt((rand() / (float)RAND_MAX)));
		dr = (cos(phi) * sin(theta) * u +  sin(phi) * sin(theta) * v + cos(theta) * hit.N).normalized();
		 

		 Ray pathP(hit.P, dr, outsideEta);
		HitInfo pathHitInfo;
		if(scene.trace(pathHitInfo, pathP, 0.001)){
			power = (power * Rd) /= Rdavr;
			pathHitInfo.material->interact(pathP, pathHitInfo, scene, bounce, power, pMap);
		}

	}

	else if(r > Rdavr && r <= (Rdavr + Rsavr)){ //if r is in <Rdavr,Rdavr+Rsavr]
		//specular reflection
		Vector3 d;
		d = -2 * dot(hit.N, photon.d) * hit.N + photon.d;
		Ray specP(hit.P, d, outsideEta);
		HitInfo specHitInfo;
		if(scene.trace(specHitInfo, specP, 0.001)){
			power = (power * Rs) /= Rsavr;
			specHitInfo.material->interact(specP, specHitInfo, scene, bounce, power, pMap);
		}
	}

	else if(r > (Rdavr + Rsavr) && r <= (Rdavr + Rsavr + Rtavr)){ //if r is in <Rdavr+Rsavr,(Rdavr + Rsavr + Rtavr)]
		//specular refraction (transmission)
		float n1, n2;
		//this code works as intended when the two materials are different
		//when they are equal n1/n2 will be 1 anyways
		if(photon.eta == outsideEta){
			//ray outside material, going inn
			n1 = outsideEta;
			n2 = eta;
		}
		else if(photon.eta == eta){
			//ray inside material, going out
			n1 = eta;
			n2 = outsideEta;
		}
		else{//some special intersection
			n1 = -1;
			n2 = -1;
		}

		//cout << "n1: " <<  n1 << " n2: " <<  n2 << endl;
		Vector3 N = hit.N;
		if(dot(-photon.d, hit.N) < 0){ //normal facing the wrong way
			N = -hit.N;
		}
		float n1Divn2 = n1 / n2;
		float wdotN = dot(-photon.d, N);
		float insideRoot = 1 - (n1Divn2  * n1Divn2) * (1.0 - wdotN * wdotN);
		if(insideRoot < 0){
			//cout << "Negative value inside root!!" << endl;
			//cout << L << endl;
			return;
		}
		float sroot = sqrtf(insideRoot);
		Vector3 wr = ((-n1Divn2) * (-photon.d - (wdotN * N))) - (sroot * N);
		//cout << wr << "Bounce: " << bounce << " View: " << viewDir << endl;

		Ray refrP(hit.P, wr, n2);
		HitInfo hitInfo;
		if(scene.trace(hitInfo, refrP, 0.0001)){
			power = (power * Rt) /= Rtavr;
			hitInfo.material->interact(refrP, hitInfo, scene, bounce, power, pMap);
		}
	}
	else{
		//printf("absorption!\n");
		//absorption
		//do nothing with the photon
	}
	*/




