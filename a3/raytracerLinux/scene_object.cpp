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
	bool intersection_occured = false;
    bool xy_flag;
    bool yz_flag;
    bool zx_flag;
    double x_val, y_val, z_val;
    double xt_value, yt_value, zt_value;
	Ray3D rayModelSpace = Ray3D(worldToModel * ray.origin, worldToModel * ray.dir);

	zt_value = - (rayModelSpace.origin[2] / rayModelSpace.dir[2]);
	x_val = rayModelSpace.origin[0] + t_value * rayModelSpace.dir[0];
	y_val = rayModelSpace.origin[1] + t_value * rayModelSpace.dir[1];

	if (t_value > 0 && sqrt( x_val * x_val + y_val * y_val) <= 1) {
		intersection_occured = true;
        xy_flag = true;
	}

	xt_value = - (rayModelSpace.origin[0] / rayModelSpace.dir[0]);
	y_val = rayModelSpace.origin[1] + t_value * rayModelSpace.dir[1];
	z_val = rayModelSpace.origin[2] + t_value * rayModelSpace.dir[2];

	if (t_value > 0 && sqrt( y_val * y_val + z_val * z_val) <= 1) {
		intersection_occured = true;
        yz_flag = true;
	}

	yt_value = - (rayModelSpace.origin[1] / rayModelSpace.dir[1]);
	x_val = rayModelSpace.origin[0] + t_value * rayModelSpace.dir[0];
	z_val = rayModelSpace.origin[2] + t_value * rayModelSpace.dir[2];

	if (t_value > 0 && sqrt( x_val * x_val + z_val * z_val) <= 1) {
		intersection_occured = true;
        zx_flag = true;
	}

    if (xy_flag) {
        z_val = sqrt(1 - x_val * x_val - y_val * y_val);
        t_value = zt_value;
    } else if (yz_flag) {
        x_val = sqrt(1 - z_val * z_val - y_val * y_val);
        t_value = xt_value;
    } else if (zx_flag) { 
        y_val = sqrt(1 - z_val * z_val - x_val * x_val);
        t_value = yt_value;
    }


	if (intersection_occured && (ray.intersection.none || t_value < ray.intersection.t_value)) {
		Intersection intersection;
		intersection.normal = modelToWorld.transpose() * Vector3D(0, 0, 1);
		intersection.point = modelToWorld * Point3D(x_val, y_val, 0);
		intersection.t_value = t_value;
		intersection.none = false;
		ray.intersection = intersection;
	}



	
	return false;
}
