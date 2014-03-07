//#include "windows.h"

#include "Miro.h"
#include "Console.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"
#include "Console.h"
#include "PointLight.h"
#include "AreaLight.h"
#include "math.h"
#include "TriangleMesh.h"

#include <iostream>
#include <ctime>
using namespace std;

Scene * g_scene = 0;
int nrays = 0;
int boxints = 0;
int triangleints = 0;


int nPhotons = 1000;
const int maxPhotonBounces = 3;

void Scene::addMesh(TriangleMesh* mesh)
{
	for (int i = 0; i < mesh->numTris(); ++i)
	{
		Triangle* t = new Triangle(mesh, i);

		t->setMaterial(mesh->materials()[i]);
		g_scene->addObject(t);
	}
}

void Scene::addMesh(TriangleMesh* mesh, TriangleMesh * mesh2)
{
	for (int i = 0; i < mesh->numTris(); ++i)
	{
		Triangle* t = new Triangle(mesh, mesh2, i);

		t->setMaterial(mesh->materials()[i]);
		g_scene->addObject(t);
	}
}



void
Scene::openGL(Camera *cam)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    cam->drawGL();

	// draw objects
    for (size_t i = 0; i < m_objects.size(); ++i){
    	glColor3f(1.0,1.0,1.0);
        m_objects[i]->renderGL();
    }
	// draw lights
	for (size_t i = 0; i < m_lights.size(); ++i) {
		glPushMatrix();
			glTranslatef(m_lights[i]->position().x, m_lights[i]->position().y, m_lights[i]->position().z);
			glColor3f(1.0, 1.0, 0.0);
			glutSolidSphere(0.2, 10, 10);
		glPopMatrix();
	}

    if(preCalcDone){
		m_accStruct->draw();
    }
    else{
    	cout << "Remember to do preCalc()!!!" << endl;
    }

    glutSwapBuffers();
}

void
Scene::preCalc()
{
	bool encapsulateBoth = false;

	// Determine Acceleration structure
	switch(m_accStruct_type){
	case ACCSTRUCT_BVH4D:
		{
		BVH4D *bvh4d = new BVH4D(temporalSamples);
		m_accStruct = bvh4d;
		break;
		}
	case ACCSTRUCT_BVHREFIT:
		{
		BVHRefit *bvhrefit = new BVHRefit();
		m_accStruct = bvhrefit;
		break;
		}
	case ACCSTRUCT_BVHREFITFULL:
		{
		BVHRefit *bvhrefit = new BVHRefit();
		m_accStruct = bvhrefit;
		encapsulateBoth = true;
		break;
		}
	case ACCSTRUCT_BVH:
	default:
		{
		BVH *bvh = new BVH();
		m_accStruct = bvh;
		encapsulateBoth = true;
		break;
		}
	}

	// Precalc objects. (Bounding boxes, etc.)
	Objects::iterator it;
	for (it = m_objects.begin(); it != m_objects.end(); it++)
	{
		Object* pObject = *it;
		((Triangle*)pObject)->setEncapsulateBoth(encapsulateBoth);
		pObject->preCalc();
		
	}

	printf("[+] Building Acceleration Structure...\n");
	fflush(stdout);
	int start = glutGet(GLUT_ELAPSED_TIME);
    m_accStruct->build(&m_objects);
	m_accStruct->draw();
    int end = glutGet(GLUT_ELAPSED_TIME);
    printf("[+] Time used to build Acceleration Structure: %i min and %i sec\n", (end - start) / 60000, (end - start) / 1000 % 60);
    printf("\n");
    fflush(stdout);

	preCalcDone = true;
}

void
Scene::raytraceImage(Camera *cam, Image *img)
{
    Ray ray;
    HitInfo hitInfo;
    Vector3 shadeResult;
//    memset(pixelResult, 0, img->width*img->height*sizeof(Vector3));
	nrays = 0;
	boxints = 0;
	triangleints = 0;
	float g = 2.2;
    srand(static_cast <unsigned> (time(0))); // Seed the random numbers. 

    int start = glutGet(GLUT_ELAPSED_TIME);
    // loop over all pixels in the image
	for(int t = 0; t < temporalSamples; t++){	// Temporal Stochastic sampling
		//float time = (rand() / (float)RAND_MAX); // Randomly or uniformly
		float time = float(t)/float(temporalSamples);
		for (int j = 0; j < img->height(); ++j){
			for (int i = 0; i < img->width(); ++i){
				Vector3 pixelSum;
				bool hit = false;
        		for(int s = 0; s < samples; s++){			// Stochastic sampling
					float dx = 0.5 * (rand() / (float)RAND_MAX) - 1.0;
					float dy = 0.5 * (rand() / (float)RAND_MAX) - 1.0;
					ray = cam->eyeRay(i + dx, j + dy, img->width(), img->height(), 1.00029, time);
					nrays++;
					if (trace(hitInfo, ray))
					{
						shadeResult = hitInfo.material->shade(ray, hitInfo, *this, 0, *pMap);
						pixelSum += shadeResult;
						hit = true;
					}
				}
        		if(hit){
					shadeResult = pixelSum / (samples * temporalSamples);
        		}
        		else{
					shadeResult = cam->bgColor();
        		}
//				Vector3 result = Vector3(pow(shadeResult.x, 1/ g), pow(shadeResult.y, 1/ g), pow(shadeResult.z, 1/ g));
				pixelResult[i][j].x += shadeResult.x;
				pixelResult[i][j].y += shadeResult.y;
				pixelResult[i][j].z += shadeResult.z;
			//	img->setPixel(i, j, pixelResult[i][j]);
			}
		//   img->drawScanline(j);
		//    glFinish();
			
			int end = glutGet(GLUT_ELAPSED_TIME);
			int est = ((end - start) / (j + 1)) * (img->height() - j - 1) * (t + 1) * temporalSamples;
			printf("Time: %f \t | ", time);
			printf("Rendering Progress: %.3f%% \r", (t  *img->height() + j) / (temporalSamples * float(img->height())) * 100.f);
			//printf("Estimated time left: %i min and %i sec\r", est/60000, (est/1000) % 60);
			fflush(stdout);
		}
		openGL(cam); // Outcomment this for (slightly) added performance. 
	}
    int end = glutGet(GLUT_ELAPSED_TIME);

    for (int j = 0; j < img->height(); ++j){
        for (int i = 0; i < img->width(); ++i){
			Vector3 result = Vector3(pow(pixelResult[i][j].x, 1/ g), pow(pixelResult[i][j].y, 1/ g), pow(pixelResult[i][j].z, 1/ g));
			img->setPixel(i, j, result);
		}
		img->drawScanline(j);
		glFinish();
	}

    printf("Rendering Progress: 100.000%\n");

    printf("\n");
    //printf("Time used to raytrace: %i sec\n", (end - start)/1000 );
    printf("Time used to raytrace: %i min and %i sec\n", (end - start)/60000, ((end - start)/1000) % 60);
    printf("Number of rays: %i\n", nrays);
    printf("Number of ray-box intersections: %i\n", boxints);
    printf("Number of ray-triangle intersections: %i\n", triangleints);
    fflush(stdout);
}

bool
Scene::trace(HitInfo& minHit, const Ray& ray, float tMin, float tMax) const
{
    return m_accStruct->intersect(minHit, ray, tMin, tMax);
}
