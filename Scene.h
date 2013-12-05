#ifndef CSE168_SCENE_H_INCLUDED
#define CSE168_SCENE_H_INCLUDED

#include <map>
#include <iostream>
#include "Miro.h"
#include "Object.h"
#include "BVH.h"
#include "PointLight.h"
#include "RectangleLight.h"

class Camera;
class Image;

typedef std::vector<PointLight*> PointLights;
typedef std::vector<RectangleLight*> RectangleLights;

class Scene
{
public:
	static Scene& getInstance() {
		static Scene instance;

		return instance;
	}
	void addObject(Object* pObj)        { m_objects.push_back(pObj); }
    const Objects* objects() const      {return &m_objects;}

	void addAnimation(Object* from, Object* to) { animations.insert(std::make_pair(from,to)); } 
	Object* getAnimation(Object* from) {
		std::map<Object*,Object*>::const_iterator pos = animations.find(from);
		if (pos == animations.end()){
			std::cerr << "Error: Animation for object not found!";
		}else{
			return pos->second;
		}
	}

    void addLight(PointLight* pObj)     	{m_lights.push_back(pObj);}
    void addRecLight(RectangleLight* pObj)     	{recLights.push_back(pObj);}
    const PointLights* lights() const        {return &m_lights;}
    const RectangleLights* reclights() const        {return &recLights;}

    void preCalc();
    void openGL(Camera *cam);

    void raytraceImage(Camera *cam, Image *img);
    bool trace(HitInfo& minHit, const Ray& ray,
               float tMin = 0.0f, float tMax = MIRO_TMAX) const;
    void setLightPos(Vector3 lightPosition){ lightPos = lightPosition;}
    void addMesh(TriangleMesh* mesh);
protected:
	Objects m_objects;
	std::map<Object*,Object*> animations; 
    BVH m_bvh;
    PointLights m_lights;
    RectangleLights recLights;
    PhotonMap* pMap;
    bool preCalcDone;
    Vector3 lightPos;
private:
	Scene() { preCalcDone = false; };                   // Constructor? (the {} brackets) are needed here.
    // Dont forget to declare these two. You want to make sure they
    // are unaccessable otherwise you may accidently get copies of
    // your singleton appearing.
    Scene(Scene const&);              // Don't Implement
    void operator=(Scene const&); // Don't implement
};

extern Scene * g_scene;

#endif // CSE168_SCENE_H_INCLUDED
