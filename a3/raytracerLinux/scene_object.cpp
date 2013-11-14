/***********************************************************
     Starter code for Assignment 3

     This code was originally written by Jack Wang for
		    CSC418, SPRING 2005

		implements scene_object.h

***********************************************************/

#include <cmath>
#include <iostream>
#include "scene_object.h"

bool UnitSquare::intersect( Ray3D& ray, const Matrix4x4& worldToModel,
		const Matrix4x4& modelToWorld ) {
	// TODO: implement intersection code for UnitSquare, which is
	// defined on the xy-plane, with vertices (0.5, 0.5, 0), 
	// (-0.5, 0.5, 0), (-0.5, -0.5, 0), (0.5, -0.5, 0), and normal
	// (0, 0, 1).
	//
	// Your goal here is to fill ray.intersection with correct values
	// should an intersection occur.  This includes intersection.point, 
	// intersection.normal, intersection.none, intersection.t_value.   
	//
	// HINT: Remember to first transform the ray into object space  
	// to simplify the intersection test.
	bool intersection_occured = true;
	Ray3D rayModelSpace = Ray3D(worldToModel * ray.origin, worldToModel * ray.dir);
	double t_value = - (rayModelSpace.origin[2] / rayModelSpace.dir[2]);
	double x_val = rayModelSpace.origin[0] + t_value * rayModelSpace.dir[0];
	double y_val = rayModelSpace.origin[1] + t_value * rayModelSpace.dir[1];
	if (t_value <= 0 || x_val < -0.5 || x_val > 0.5 || y_val < -0.5 || y_val > 0.5) {
		intersection_occured = false;
	}
	if (intersection_occured && (ray.intersection.none || t_value < ray.intersection.t_value)) {
		Intersection intersection;
		intersection.normal = modelToWorld.transpose() * Vector3D(0, 0, 1);
		intersection.point = modelToWorld * Point3D(x_val, y_val, 0);
		intersection.t_value = t_value;
		intersection.none = false;
		ray.intersection = intersection;
	}
	
	return intersection_occured;
}

bool UnitSphere::intersect( Ray3D& ray, const Matrix4x4& worldToModel,
		const Matrix4x4& modelToWorld ) {
	// TODO: implement intersection code for UnitSphere, which is centred 
	// on the origin.  
	//
	// Your goal here is to fill ray.intersection with correct values
	// should an intersection occur.  This includes intersection.point, 
	// intersection.normal, intersection.none, intersection.t_value.   
	//
	// HINT: Remember to first transform the ray into object space  
	// to simplify the intersection test.
	
	return false;
}