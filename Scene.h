#ifndef CSE168_SCENE_H_INCLUDED
#define CSE168_SCENE_H_INCLUDED

#include <map>
#include <iostream>
#include "Miro.h"
#include "Object.h"
#include "PointLight.h"
#include "RectangleLight.h"
#include "AccStructure.h"
#include "BVH.h"
#include "BVH4D.h"
#include "BVH4DSAH.h"
#include "BVH4DSAHMIX.h"
#include "BVHRefit.h"

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

	enum
    {
        ACCSTRUCT_BVH   = 0,
        ACCSTRUCT_BVHREFIT = 1,
		ACCSTRUCT_BVHREFITFULL = 2,
		ACCSTRUCT_BVH4D = 3,
		ACCSTRUCT_BVH4DSAH = 4,
		ACCSTRUCT_BVH4DSAHMIX = 5
    };

	void addObject(Object* pObj)        { m_objects.push_back(pObj); }
    const Objects* objects() const      {return &m_objects;}

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
	void addMesh(TriangleMesh* mesh, TriangleMesh* mesh2); // mesh at start time and mesh2 at stop time
protected:
	Objects m_objects;
    AccStructure *m_accStruct;
	int m_accStruct_type;
    PointLights m_lights;
    RectangleLights recLights;
    PhotonMap* pMap;
    bool preCalcDone;
    Vector3 lightPos;
	Vector3 pixelResult[4000][3000];
	int samples;
	int temporalSamples;
private:
	Scene() { 
		m_accStruct_type = ACCSTRUCT_BVHREFIT;
		m_accStruct = NULL;
		preCalcDone = false; 
		samples = 1;
		temporalSamples = 16;
	};                   // Constructor? (the {} brackets) are needed here.
    // Dont forget to declare these two. You want to make sure they
    // are unaccessable otherwise you may accidently get copies of
    // your singleton appearing.
    Scene(Scene const&);              // Don't Implement
    void operator=(Scene const&); // Don't implement
};

extern Scene * g_scene;

#endif // CSE168_SCENE_H_INCLUDED
