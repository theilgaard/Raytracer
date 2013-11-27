//#include "windows.h"

#include "Miro.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"
#include "Console.h"
#include "PointLight.h"
#include "AreaLight.h"
#include "math.h"
#include "TriangleMesh.h"

#include <iostream>
using namespace std;

Scene * g_scene = 0;
int nrays = 0;
int boxints = 0;
int triangleints = 0;


int nPhotons = 1000;
const int maxPhotonBounces = 3;

Scene::Scene()
{
	preCalcDone = false;
}

void Scene::addMesh(TriangleMesh* mesh)
{
	for (int i = 0; i < mesh->numTris(); ++i)
	{
		Triangle* t = new Triangle(mesh, i);

		t->setMaterial(mesh->materials()[i]);
		g_scene->addObject(t);
	}
}


void
Scene::openGL(Camera *cam)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    cam->drawGL();

	/*
    glPushMatrix();
    	glTranslatef(lightPos.x, lightPos.y, lightPos.z);
		glutSolidSphere(0.2, 10, 10);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(lFocusPos.x, lFocusPos.y, lFocusPos.z);
		glutSolidSphere(focusSphereR, 10, 10);
	glPopMatrix();
	*/

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
		//m_bvh.draw();
    }
    else{
    	cout << "Remember to do preCalc()!!!" << endl;
    }

    glutSwapBuffers();
}

void
Scene::preCalc()
{
	Objects::iterator it;
	for (it = m_objects.begin(); it != m_objects.end(); it++)
	{
		Object* pObject = *it;
		pObject->preCalc();
	}

	printf("Building BVH...\n");
	fflush(stdout);
	int start = glutGet(GLUT_ELAPSED_TIME);
    m_bvh.build(&m_objects);
    int end = glutGet(GLUT_ELAPSED_TIME);
    printf("Time used to build BVH: %i min and %i sec\n", (end - start) / 60000, (end - start) / 1000 % 60);
	printf("Total number of boxes: %i\n", m_bvh.nBoxes);
	printf("Number of leaf nodes: %i\n", m_bvh.nLeafs);
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
	nrays = 0;
	boxints = 0;
	triangleints = 0;
	float g = 2.2;
	const int samples = 4;
    
    int start = glutGet(GLUT_ELAPSED_TIME);
    // loop over all pixels in the image
    for (int j = 0; j < img->height(); ++j)
    {
        for (int i = 0; i < img->width(); ++i)
        {
			Vector3 pixelSum;
			bool hit = false;
        	for(int s = 0; s < samples; s++){
				float dx = 0.5 * (rand() / (float)RAND_MAX) - 1.0;
				float dy = 0.5 * (rand() / (float)RAND_MAX) - 1.0;
				ray = cam->eyeRay(i + dx, j + dy, img->width(), img->height(), 1.00029);
				nrays++;
				if (trace(hitInfo, ray))
				{
					shadeResult = hitInfo.material->shade(ray, hitInfo, *this, 0, *pMap);
					pixelSum += shadeResult;
					hit = true;
				}
            }
        	if(hit){
				shadeResult = pixelSum / samples;
        	}
        	else{
				shadeResult = cam->bgColor();
        	}
			Vector3 result = Vector3(pow(shadeResult.x, 1/ g), pow(shadeResult.y, 1/ g), pow(shadeResult.z, 1/ g));
			img->setPixel(i, j, result);
        }
        img->drawScanline(j);
        glFinish();
        int end = glutGet(GLUT_ELAPSED_TIME);
        int est = ((end - start) / (j + 1)) * (img->height() - j - 1);
        printf("Rendering Progress: %.3f%% ", j/float(img->height())*100.0f);
        printf("Estimated time left: %i min and %i sec\r", est/60000, (est/1000) % 60);
        fflush(stdout);
    }
    int end = glutGet(GLUT_ELAPSED_TIME);

    printf("Rendering Progress: 100.000%\n");
    debug("done Raytracing!\n");

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
    return m_bvh.intersect(minHit, ray, tMin, tMax);
}
