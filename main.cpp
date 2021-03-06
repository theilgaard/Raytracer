//#include "windows.h"

#include <math.h>
#include "Miro.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"
#include "Console.h"

#include "PointLight.h"
#include "TriangleMesh.h"
#include "Triangle.h"
#include "MiroWindow.h"
#include "Lambert.h"
#include "ShadingModel.h"
#include "RectangleLight.h"


#include <iostream>
using namespace std;

// local helper function definitions
namespace
{

void
addMeshTrianglesToScene(TriangleMesh * mesh, ShadingModel * material)
{
    // create all the triangles in the bunny mesh and add to the scene
    for (int i = 0; i < mesh->numTris(); ++i)
    {
        Triangle* t = new Triangle;
        t->setIndex(i);
        t->setMesh(mesh);
        t->setMaterial(material);
        g_scene->addObject(t);
    }
}


inline Matrix4x4
translate(float x, float y, float z)
{
    Matrix4x4 m;
    m.setColumn4(Vector4(x, y, z, 1));
    return m;
}


inline Matrix4x4
scale(float x, float y, float z)
{
    Matrix4x4 m;
    m.m11 = x;
    m.m22 = y;
    m.m33 = z;
    return m;
}

// angle is in degrees
inline Matrix4x4
rotate(float angle, float x, float y, float z)
{
    float rad = angle*(PI/180.);

    float x2 = x*x;
    float y2 = y*y;
    float z2 = z*z;
    float c = cos(rad);
    float cinv = 1-c;
    float s = sin(rad);
    float xy = x*y;
    float xz = x*z;
    float yz = y*z;
    float xs = x*s;
    float ys = y*s;
    float zs = z*s;
    float xzcinv = xz*cinv;
    float xycinv = xy*cinv;
    float yzcinv = yz*cinv;

    Matrix4x4 m;
    m.set(x2 + c*(1-x2), xy*cinv+zs, xzcinv - ys, 0,
          xycinv - zs, y2 + c*(1-y2), yzcinv + xs, 0,
          xzcinv + ys, yzcinv - xs, z2 + c*(1-z2), 0,
          0, 0, 0, 1);
    return m;
}

} // namespace


void makeScene(){
	g_camera = new Camera;
	g_scene = &Scene::getInstance();
	g_image = new Image;
	float lSourceDistance = 140;
	Vector3 focusSphere = Vector3(2.1, 1.6, -1.4);
	Vector3 ldir = Vector3(-1.19136, 2.91208, -2.51118) - focusSphere;
	Vector3 lightPos = focusSphere + ldir.normalized() * lSourceDistance;
	g_scene->setLightPos(lightPos);
	g_image->resize(400, 300);
	//g_image->resize(700, 512);

	// set up the camera
	g_camera->setBGColor(Vector3(0.0f, 0.0f, 0.0f));
	g_camera->setEye(Vector3(-2.6, 1.7, 0.4));
	g_camera->setLookAt(Vector3(2.1, 1.3, -1.2));
	g_camera->setUp(Vector3(0, 1, 0));
	g_camera->setFOV(45);

	// create and place a point light source
	PointLight * light2 = new PointLight;
	light2->setPosition(lightPos);
	light2->setColor(Vector3(1, 1, 1));
	light2->setWattage(25000);
	g_scene->addLight(light2);


	ShadingModel* Light = new ShadingModel(Vector3(0.0f), -1.0, Vector3(0.8f), Vector3(0.0f), Vector3(0.0f));
	ShadingModel* Cabinet = new ShadingModel(Vector3(0.0f), -1.0, Vector3(0.0f,0.1f,0.8f), Vector3(0.0f), Vector3(0.0f));
	ShadingModel* WoodDiff = new ShadingModel(Vector3(0.0f), -1.0, Vector3(0.8f), Vector3(0.0f), Vector3(0.0f));
	ShadingModel* CupPlat = new ShadingModel(Vector3(0.0f), 1.33, Vector3(0.08f), Vector3(0.2f), Vector3(0.8f));
	ShadingModel* Floor = new ShadingModel(Vector3(0.0f), -1.0, Vector3(0.8f), Vector3(0.0f), Vector3(0.0f));
	ShadingModel* WoodWall = new ShadingModel(Vector3(0.0f), -1.0, Vector3(0.0f, 0.1, 0.0), Vector3(0.0f), Vector3(0.0f));
	ShadingModel* WhiteWall = new ShadingModel(Vector3(0.0f), -1.0, Vector3(1.0f), Vector3(0.0f), Vector3(0.0f));
	ShadingModel* WallColorDiff = new ShadingModel(Vector3(0.0f), -1.0, Vector3(0.8f), Vector3(0.0f), Vector3(0.0f));
	ShadingModel* BeerBottleGreen = new ShadingModel(Vector3(0.0f), 1.5, Vector3(0.0f,0.28f,0.02f), Vector3(0.2f), Vector3(0.6f));
	ShadingModel* BeerBottleBrown = new ShadingModel(Vector3(0.0f), 1.5, Vector3(0.22f,0.09f,0.0f), Vector3(0.2f), Vector3(0.6f));
	ShadingModel* BeerBottleBluen = new ShadingModel(Vector3(0.0f), 1.5, Vector3(0.002f,0.00f,0.08f), Vector3(0.2f), Vector3(0.6f));
	ShadingModel* coffeecup = new ShadingModel(Vector3(0.0f), -1.0, Vector3(0.1f), Vector3(0.2f), Vector3(0.0f));
	ShadingModel* coffeetable = new ShadingModel(Vector3(0.0f), 1.1, Vector3(0.02f,0.02f,0.02f), Vector3(0.02f), Vector3(0.7f));
	ShadingModel* couch = new ShadingModel(Vector3(0.0f), -1.0, Vector3(0.431f,0.314f,0.235f), Vector3(0.0f), Vector3(0.0f));
	ShadingModel* barrel = new ShadingModel(Vector3(0.0f), -1.0, Vector3(0.23f,0.136f,0.024), Vector3(0.0f), Vector3(0.0f));
	ShadingModel* barrelrings = new ShadingModel(Vector3(0.0f), -1.0, Vector3(0.8f), Vector3(0.0f), Vector3(0.0f));
	ShadingModel* water = new ShadingModel(Vector3(0.0f), 1.3, Vector3(0.3,0.3,0.8f), Vector3(0.7f), Vector3(0.7f));
	ShadingModel* tableAndCouchLegs = new ShadingModel(Vector3(0.1f), -1.0, Vector3(0.7f), Vector3(0.4f), Vector3(0.0f));
	ShadingModel* lamps = new ShadingModel(Vector3(0.0f), -1.0, Vector3(0.01f), Vector3(0.0f), Vector3(0.0f));
	ShadingModel* WalllampCopper = new ShadingModel(Vector3(0.0f), -1.0, Vector3(0.8f,0.4f,0.16f), Vector3(0.2f), Vector3(0.0f));
	ShadingModel* WalllampWhite = new ShadingModel(Vector3(0.0f), -1.0, Vector3(0.9f), Vector3(0.08f), Vector3(0.0f));
	ShadingModel* WineBottle = new ShadingModel(Vector3(0.0f), 1.5, Vector3(0.01684f,0.0469f,0.0f), Vector3(0.2f), Vector3(0.4f));

	TriangleMesh * object = new TriangleMesh;
	object->connectNameToMaterial("Light",Light);
	object->connectNameToMaterial("Cabinet",Cabinet);
	object->connectNameToMaterial("CupPlate_CupPlateDiffuceMap.jpg_CupPlateDiffuceMap.jpg",CupPlat);
	object->connectNameToMaterial("Foor_FloorDiffuce.jpg_FloorDiffuce.jpg",Floor);
	object->connectNameToMaterial("WoodWall_WoodWallDiffuce.jpg_WoodWallDiffuce.jpg",WoodWall);
	object->connectNameToMaterial("Wood_WoodDiffuce.jpg_WoodDiffuce.jpg",WoodWall);
	object->connectNameToMaterial("WallColor_WallColorDiffuce.jpg_WallColorDiffuce.jpg",WhiteWall);
	object->connectNameToMaterial("WhiteWall",WhiteWall);
	object->connectNameToMaterial("beerbottleGreen",BeerBottleGreen);
	object->connectNameToMaterial("beerbottleBrown",BeerBottleBrown);
	object->connectNameToMaterial("beerbottleBlue",BeerBottleBluen);
	object->connectNameToMaterial("brett_PlanksNe.jpg",barrel);
	object->connectNameToMaterial("top_PlanksNe.jpg",barrel);
	object->connectNameToMaterial("ring_rust.jpg",barrelrings);
	object->connectNameToMaterial("brett_PlanksNe.jpg",barrel);
	object->connectNameToMaterial("mtl_001",water);
	object->connectNameToMaterial("Pewter_Metallic_",coffeecup);
	object->connectNameToMaterial("brett_PlanksNe.jpg",barrel);
	object->connectNameToMaterial("mia_material_x_passes7SG",coffeetable);
	object->connectNameToMaterial("mia_material_x_passes4SG",couch);
	object->connectNameToMaterial("mia_material_x_passes5SG",coffeecup); // Coffee table legs and couch legs
	object->connectNameToMaterial("mia_material_x_passes5SG1",coffeecup);// Given the metallic colour
	object->connectNameToMaterial("mia_material_x_passes6SG",coffeecup);
	object->connectNameToMaterial("mia_material_x_passes3SG",coffeecup);
	object->connectNameToMaterial("glascup",CupPlat);
	object->connectNameToMaterial("BEIGE_PLSTC",lamps); // Kitchen lamps
	object->connectNameToMaterial("Chrome_matte",lamps);// Kitchen lamps
	object->connectNameToMaterial("SV",lamps);			// Kitchen lamps
	object->connectNameToMaterial("WalllampCopper",WalllampCopper);
	object->connectNameToMaterial("WalllampWhite",WalllampWhite);
	object->connectNameToMaterial("chrome",coffeecup);
	object->connectNameToMaterial("leather_brown",couch);
	object->connectNameToMaterial("fan",lamps);
	object->connectNameToMaterial("fanwings",coffeecup);
	object->connectNameToMaterial("winebottle",WineBottle);
	object->load("objects/LivingRoom3.obj");
	//object->load("teapot.obj");
	g_scene->addMesh(object);

	/*
	ShadingModel* mat = new ShadingModel(Vector3(0.0f), -1.0, Vector3(0.7), Vector3(0.0f), Vector3(0.0f));

	TriangleMesh * object = new TriangleMesh;
	object->load("LivingRoom2.obj");
	addMeshTrianglesToScene(object, mat);
	*/

	g_scene->preCalc();
}

void makeRefractedBallsScene(){
	g_camera = new Camera;
	g_scene = &Scene::getInstance();
	g_image = new Image;
	float lSourceDistance = 16;
	Vector3 focusSphere = Vector3(2.1, 1.6, -1.4);
	Vector3 ldir = Vector3(0, 0, 0);
	Vector3 lightPos1 = Vector3(10, 8, -10);
	Vector3 lightPos2 = Vector3(-10, 8, -10);
	g_scene->setLightPos(lightPos1);
	g_image->resize(400, 300);

	// set up the camera
	g_camera->setBGColor(Vector3(0.1f));
	g_camera->setEye(Vector3(-4, 4, -4));
	g_camera->setLookAt(Vector3(0, 2, 0));
	g_camera->setUp(Vector3(0, 1, 0));
	g_camera->setFOV(45);

	// Create 1st light source
	PointLight * light1 = new PointLight;
	light1->setPosition(lightPos1);
	light1->setColor(Vector3(1, 1, 1));
	light1->setWattage(150);
	g_scene->addLight(light1);

	// Create 2nd light source
	PointLight * light2 = new PointLight;
	light2->setPosition(lightPos2);
	light2->setColor(Vector3(1, 1, 1));
	light2->setWattage(150);
	g_scene->addLight(light2);

	// Material properties (ambient, refraction index, diffuse, specular, refraction)
	ShadingModel* material1 = new ShadingModel(Vector3(.0f), 1.5, Vector3(0.01), Vector3(0.90, 0.46, 0.36), Vector3(0.90, 0.46, 0.36)); // red ball
	ShadingModel* material2 = new ShadingModel(Vector3(.0f), 1.005, Vector3(0.01), Vector3(0.90, 0.76, 0.46), Vector3(0.0f)); // yellow ball
	ShadingModel* material3 = new ShadingModel(Vector3(.0f), 1.5, Vector3(0.05), Vector3(0.65, 0.77, 0.97), Vector3(0.65, 0.77, 0.97)); // blue ball
	ShadingModel* material4 = new ShadingModel(Vector3(.0f), 1, Vector3(0.65, 0.97, 0.46), Vector3(0), Vector3(0.0f));  // green rectangle
	ShadingModel* glass = new ShadingModel(Vector3(.0f), 1.52, Vector3(0.01), Vector3(0.6), Vector3(.95)); // clear glass
	ShadingModel* lightblue = new ShadingModel(Vector3(0.0), 1, Vector3(84.0/255.0, 170.0/255, 1.0), Vector3(0.0), Vector3(0.0)); // Light blue lambertian.
	ShadingModel* floorMat = new ShadingModel(Vector3(.0f), 1, Vector3(0.8), Vector3(0.2f), Vector3(0.0f));
	TriangleMesh * object = new TriangleMesh;

	// Link materials to names.
	object->connectNameToMaterial("Material.001", material1);
	object->connectNameToMaterial("Material.002", material2);
	object->connectNameToMaterial("Material.003", material3);
	object->connectNameToMaterial("rectangle", material4);
	object->connectNameToMaterial("rectangle2", material4);

	// Scene object file.
	object->setDefaultMaterial(lightblue);
	object->load("objects/balls_smooth.obj");
	//object->load("objects/sphere_super_smooth.obj");
	g_scene->addMesh(object);

	// Floor triangle
	TriangleMesh * floor = new TriangleMesh;
	floor->createSingleTriangle();
	floor->setV1(Vector3(0, 0, 500));
	floor->setV2(Vector3(500, 0, -500));
	floor->setV3(Vector3(-500, 0, 0));
	floor->setN1(Vector3(0, 1, 0));
	floor->setN2(Vector3(0, 1, 0));
	floor->setN3(Vector3(0, 1, 0));

	Triangle* t = new Triangle;
	t->setIndex(0);
	t->setMesh(floor);
	t->setMaterial(floorMat);
	g_scene->addObject(t);

	g_scene->preCalc();
}
Matrix4x4* fm;

void makeAnimatedSphere(){
	g_camera = new Camera;
	g_scene = &Scene::getInstance();
	g_image = new Image;
	float lSourceDistance = 16;
	Vector3 focusSphere = Vector3(2.1, 1.6, -1.4);
	Vector3 ldir = Vector3(0, 0, 0);
	Vector3 lightPos1 = Vector3(10, 8, -10);
	Vector3 lightPos2 = Vector3(-10, 8, -10);
	g_scene->setLightPos(lightPos1);
	g_image->resize(400, 300);

	// set up the camera
	g_camera->setBGColor(Vector3(0.1f));
	g_camera->setEye(Vector3(-12, 8, 0));
	g_camera->setLookAt(Vector3(0, 2, 0));
	g_camera->setUp(Vector3(0, 1, 0));
	g_camera->setFOV(45);

	// Create 1st light source
	PointLight * light1 = new PointLight;
	light1->setPosition(lightPos1);
	light1->setColor(Vector3(1, 1, 1));
	light1->setWattage(150);
	g_scene->addLight(light1);

	// Create 2nd light source
	PointLight * light2 = new PointLight;
	light2->setPosition(lightPos2);
	light2->setColor(Vector3(1, 1, 1));
	light2->setWattage(150);
	g_scene->addLight(light2);

	ShadingModel* lightblue = new ShadingModel(Vector3(0.0), 1, Vector3(30.0/255.0, 144.0/255, 1.0), Vector3(0.0), Vector3(0.0)); // Light blue lambertian.
	ShadingModel* floorMat = new ShadingModel(Vector3(.0f), 1, Vector3(0.8), Vector3(0.2f), Vector3(0.0f));
	
	TriangleMesh * object = new TriangleMesh;
	fm = &translate(0.0, 4.0, 0.0);
	object->setDefaultMaterial(lightblue);

	TriangleMesh * object2 = new TriangleMesh;
	object2->setDefaultMaterial(lightblue);
	int sceneSelector = 6;

	switch(sceneSelector){
	case 0:
		object->load("objects/cubeRot.obj");	// 192 triangles
		object2->load("objects/cubeRot2.obj");
		g_camera->setEye(Vector3(-10.834f, 10.391f, -3.829f));
		g_camera->setViewDir(Vector3(0.841f, -0.434f, 0.323f));
		break;
	case 1:
		object->load("objects/sphere_super_smooth.obj"); // 1920 triangles
		object2->load("objects/sphere_super_smooth2.obj");
		g_camera->setEye(Vector3(-10.013f, 8.035f, -0.220f));
		g_camera->setViewDir(Vector3(0.907f, -0.421f, 0.032f));
		break;
	case 2:
		object->load("objects/wooddoll_00tri.obj");	// 5378 triangles
		object2->load("objects/wooddoll_03tri.obj");
		g_camera->setEye(Vector3(-0.745f, 0.439f, 0.051f));
		g_camera->setViewDir(Vector3(0.967f, -0.254f, -0.033f));
		break;
	case 3:
		object->load("objects/hand_00tri.obj"); // 15,855 triangles
		object2->load("objects/hand_03tri.obj");
		g_camera->setEye(Vector3(-0.734f, 0.439f, -0.522f));
		g_camera->setViewDir(Vector3(0.895f, -0.086f, 0.437f));
		break;
	case 4:
		object->load("objects/24cell000tri.obj");	// 122,800 triangles
		object2->load("objects/24cell008tri.obj");
		g_camera->setEye(Vector3(-10.134f, 1.342f, -5.112f));
		g_camera->setViewDir(Vector3(0.894f, -0.103f, 0.436f));
		break;
	case 5:
		object->load("objects/ben_00tri.obj");		// 78,000 triangles
		object2->load("objects/ben_10tri.obj");
		g_camera->setEye(Vector3(-0.826f, 0.630f, 0.153f));
		g_camera->setViewDir(Vector3(0.938f, -0.321f, -0.132f));
		break;
	case 6:
		object->load("objects/marbles025tri.obj");	// 8,800 triangles
		object2->load("objects/marbles032tri.obj");
		g_camera->setEye(Vector3(-0.574f, 0.524f, 0.106f));
		g_camera->setViewDir(Vector3(0.906f, -0.403f, -0.128f));
		break;
	case 7:
		object->load("objects/Toasters004tri.obj");	// 11,100 triangles
		object2->load("objects/Toasters014tri.obj");
		g_camera->setEye(Vector3(-0.317f, 0.395f, -0.944f));
		g_camera->setViewDir(Vector3(0.319f, -0.368f, 0.873f));
		break;
	case 8:
	{
		object->load("objects/f160tri.obj");	// 172,200 triangles
		object2->load("objects/f170tri.obj");
		g_camera->setEye(Vector3(-0.043f, 0.177f, 0.309f));
		g_camera->setViewDir(Vector3(0.164f, -0.311f, -0.936f));
		Vector3 lightPos12 = Vector3(0, 8, 2);
		g_scene->setLightPos(lightPos12);
		// Create 2nd light source
		PointLight * light2 = new PointLight;
		light2->setPosition(lightPos12);
		light2->setColor(Vector3(1, 1, 1));
		light2->setWattage(50);
		g_scene->addLight(light2);
		break;
	}
	case 9:
		object->load("objects/tesseract000tri.obj");	// 172,100 triangles
		object2->load("objects/tesseract004tri.obj");
		g_camera->setEye(Vector3(-0.732f, 0.874f, -2.079f));
		g_camera->setViewDir(Vector3(0.319f, -0.368f, 0.873f));
		break;
	case 10:
		object->load("objects/dragon00tri.obj");		// 252,200 triangles
		object2->load("objects/dragon05tri.obj");
		g_camera->setEye(Vector3(-0.079f, 0.141f, -0.228f));
		g_camera->setViewDir(Vector3(0.274f, -0.143f, 0.951f));
		break;
	default:
		break;
	}

	g_scene->addMesh(object, object2);
	
	// Floor triangle
	TriangleMesh * floor = new TriangleMesh;
	floor->createSingleTriangle();
	floor->setV1(Vector3(0, 0, 500));
	floor->setV2(Vector3(500, 0, -500));
	floor->setV3(Vector3(-500, 0, 0));
	floor->setN1(Vector3(0, 1, 0));
	floor->setN2(Vector3(0, 1, 0));
	floor->setN3(Vector3(0, 1, 0));
	Triangle* t = new Triangle();

	t->setIndex(0);
	t->setMesh(floor);
	t->setMaterial(floorMat);
	g_scene->addObject(t);

	g_scene->preCalc();
}

int
main(int argc, char*argv[])
{
    // create a scene
	//makeScene();
	//makeRefractedBallsScene();
	makeAnimatedSphere();

    MiroWindow miro(&argc, argv);
    miro.mainLoop();

    return 0; // never executed
}


