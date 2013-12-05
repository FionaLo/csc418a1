/***********************************************************
     Starter code for Assignment 3

     This code was originally written by Jack Wang for
		    CSC418, SPRING 2005

		implements light_source.h

***********************************************************/

#include <cmath>
#include "light_source.h"
#include "area_light_source.h"
    
/* Set the type of shading here */
// RenderType LightSource::RENDER_TYPE = SCENE_SIGNATURE;
// RenderType LightSource::RENDER_TYPE = DIFFUSE_AMBIENT; 
RenderType LightSource::RENDER_TYPE = PHONG; 

double dmax(double a, double b) {
    return a < b ? b : a;
}

void PointLight::shade( Ray3D& ray ) {
	// TODO: implement this function to fill in values for ray.col 
	// using phong shading.  Make sure your vectors are normalized, and
	// clamp colour values to 1.0.
	//
	// It is assumed at this point that the intersection information in ray 
	// is available.  So be sure that traverseScene() is called on the ray 
	// before this function.  

    // TODO:
	
    Vector3D normal_vector = ray.intersection.normal;
    normal_vector.normalize();

    Vector3D light_vector = _pos - ray.intersection.point;
    light_vector.normalize();

    double ldotn = light_vector.dot(normal_vector);

    Vector3D reflect_vector = (2 * ldotn * normal_vector) - light_vector;
    reflect_vector.normalize();

    Vector3D view_vector = -ray.dir;
    view_vector.normalize();

    Material *material = ray.intersection.mat;

    Colour col;
    switch(LightSource::RENDER_TYPE) {
        case SCENE_SIGNATURE:
            col = material->diffuse;
        break;
        case DIFFUSE_AMBIENT:
            col = dmax(0, ldotn) * material->diffuse * _col_diffuse;
        break;
        case PHONG:
            {
            double vdotr = view_vector.dot(reflect_vector);
            col = dmax(0, ldotn) * material->diffuse * _col_diffuse +
                  dmax(0, pow(vdotr, material->specular_exp)) * material->specular * _col_specular;
            }
        break;
        default:
            throw "Invalid render type";
    }
    col.clamp();
    ray.col = ray.col + col;
    ray.col.clamp();
}

