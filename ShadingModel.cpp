#include "ShadingModel.h"
#include "Ray.h"
#include "Scene.h"
#include "PointLight.h"
#include "RectangleLight.h"
#include <cstdlib>

#include <iostream>
using namespace std;

ShadingModel::ShadingModel(const Vector3 & ambient, float refrationIndex, const Vector3 & diffuse, const Vector3 & specular, const Vector3 & refrationcolor) :
	m_ka(ambient), maxBounces(3), eta(refrationIndex), outsideEta(-1.0f), Rd(diffuse), Rs(specular), Rt(refrationcolor)
{

}

ShadingModel::~ShadingModel()
{
}

Vector3
ShadingModel::shade(const Ray& ray, const HitInfo& hit, const Scene& scene, int bounce, PhotonMap& pMap)
{
	Vector3 L = Vector3(0.0f, 0.0f, 0.0f);
    const Vector3 viewDir = -ray.d; // d is a unit vector

    bounce++;
	if(bounce > maxBounces){
		return L;
	}

	if(outsideEta == -1.0){
		outsideEta = ray.eta;
	}

	Vector3 d;
 	if(Rs != Vector3(0.0f)){
		d = -2 * dot(hit.N, ray.d) * hit.N + ray.d;
		Ray specR(hit.P, d, outsideEta);
		HitInfo specHitInfo;
		if(scene.trace(specHitInfo, specR, 0.00029)){
			Vector3 mirrorColor = specHitInfo.material->shade(specR, specHitInfo, scene, bounce, pMap);
			L += Rs * mirrorColor;
		}
 	}

 	//Compute direct lighting for points lights
	const PointLights *lightlist = scene.lights();

	// Loop over all of the PointLights
	PointLights::const_iterator lightIter;
	for (lightIter = lightlist->begin(); lightIter != lightlist->end(); lightIter++)
	{
		PointLight* light = *lightIter;

		Vector3 l = light->position() - hit.P;

		// the inverse-squared falloff
		float falloff = l.length2(); //gives the length^2 of l which is also r^2

		// normalize the light direction
		float r = sqrt(falloff);
		l /= r;

		// get the diffuse component
		float nDotL = dot(hit.N, l);

		// Shadow computation
		Vector3 E = std::max(0.0f, nDotL * light->wattage() / (4 * PI * falloff));
		Ray shadowCheck(hit.P, l, 0);
		HitInfo temp;
		if(!scene.trace(temp, shadowCheck, 0.00029, r)){
			L += (1/PI) * Rd * E * light->color();
		} // else in shadow
		
		//Phong model
		if(Rs != Vector3(0.0f)){
			float exp = 50.0;
			float dDotl = dot(d,l);
			if(dDotl > 0){
				L += Rs * ((pow(dDotl, exp) * E) / nDotL);
			}
		}
	}

	/*
	//Compute direct lighting for Area lights
	const RectangleLights *reclightlist = scene.reclights();
	RectangleLights::const_iterator reclightIter;
	for (reclightIter = reclightlist->begin(); reclightIter != reclightlist->end(); reclightIter++)
	{
		RectangleLight* light = *reclightIter;
		int hits = 0;
		int tries = 10;
		for(int i = 0; i < tries; i++){
			Vector3 l = light->generateRandomPosition() - hit.P;
			float r = l.length();
			l /= r;
			Ray shadowCheck(hit.P, l, 0);
			HitInfo temp;
			if(!scene.trace(temp, shadowCheck, 0.0001, r)){
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
		float visibility = (hits / float(tries) );
		L +=  visibility * (1/PI) * Rd * E * light->color();
	}
	 */

	// add the ambient component
	 L += m_ka;

	// Refraction
 	if(Rt != Vector3(0.0f)){
		float n1, n2;
		//this code works as intended when the two materials are different
		//when they are equal n1/n2 will be 1 anyways
		if(ray.eta == outsideEta){
			//ray outside material, going inn
			n1 = outsideEta;
			n2 = eta;
		}
		else if(ray.eta == eta){
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
		if(dot(viewDir, hit.N) < 0){ //normal facing the wrong way
			N = -hit.N;
		}
		float n1Divn2 = n1 / n2;
		float wdotN = dot(viewDir, N);
		float insideRoot = 1 - (n1Divn2  * n1Divn2) * (1.0 - wdotN * wdotN);
		if(insideRoot < 0){
			//cout << "Negative value inside root!!" << endl;
			//cout << L << endl;
			return L;
		}
		float sroot = sqrtf(insideRoot);
		Vector3 wr = ((-n1Divn2) * (viewDir - (wdotN * N))) - (sroot * N);
		//cout << wr << "Bounce: " << bounce << " View: " << viewDir << endl;

		Ray refrR(hit.P, wr, n2);
		HitInfo hitInfo;
		if(scene.trace(hitInfo, refrR, 0.0001)){
				Vector3 color = hitInfo.material->shade(refrR, hitInfo, scene, bounce, pMap);
				L += Rt * color;
		}
    }

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
		 */

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

}


