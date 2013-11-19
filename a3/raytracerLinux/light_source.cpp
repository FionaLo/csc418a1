/***********************************************************
     Starter code for Assignment 3

     This code was originally written by Jack Wang for
		    CSC418, SPRING 2005

		implements light_source.h

***********************************************************/

#include <cmath>
#include "light_source.h"

//RenderType LightSource::RENDER_TYPE = SCENE_SIGNATURE;
//RenderType LightSource::RENDER_TYPE = DIFFUSE_AMBIENT; 
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
    Vector3D light_vector = _pos - Point3D(0,0,0);
    double ldotn = light_vector.dot(normal_vector);
    Vector3D reflect_vector = 2 * ldotn * normal_vector - light_vector;
    Vector3D view_vector = -ray.dir;

    normal_vector.normalize();
    light_vector.normalize();
    reflect_vector.normalize();
    view_vector.normalize();


	if (LightSource::RENDER_TYPE == SCENE_SIGNATURE) {
        ray.col = ray.intersection.mat->diffuse;
	}
    else if (LightSource::RENDER_TYPE == DIFFUSE_AMBIENT) {
        ray.col =  ray.intersection.mat->ambient * _col_ambient + 
                    dmax(0,normal_vector.dot(light_vector)) * ray.intersection.mat->diffuse * _col_diffuse;
    }
    else if (LightSource::RENDER_TYPE == PHONG) {
        ray.col =  ray.intersection.mat->ambient * _col_ambient + 
                    dmax(0, normal_vector.dot(light_vector)) * ray.intersection.mat->diffuse * _col_diffuse +
                    dmax(0, pow(view_vector.dot(reflect_vector),ray.intersection.mat->specular_exp)) * ray.intersection.mat->specular * _col_specular;
    }


}

