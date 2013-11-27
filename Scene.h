#ifndef CSE168_SCENE_H_INCLUDED
#define CSE168_SCENE_H_INCLUDED

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
	Scene();
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

protected:
	Objects m_objects;
    BVH m_bvh;
    PointLights m_lights;
    RectangleLights recLights;
    PhotonMap* pMap;
    bool preCalcDone;
    Vector3 lightPos;
};

extern Scene * g_scene;

#endif // CSE168_SCENE_H_INCLUDED
