/***********************************************************
     Starter code for Assignment 3

     This code was originally written by Jack Wang for
		    CSC418, SPRING 2005

		implements scene_object.h

***********************************************************/

#include <cmath>
#include <iostream>
#include "scene_object.h"

using namespace std;

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
	bool intersection_overwrite = false;

	Ray3D rayModelSpace = Ray3D(worldToModel * ray.origin, worldToModel * ray.dir);

	double t_value = - (rayModelSpace.origin[2] / rayModelSpace.dir[2]);

	Point3D intersectionPoint = rayModelSpace.point_at(t_value);

	if (t_value <= 0 || 
		intersectionPoint[0] < -0.5 || intersectionPoint[0] > 0.5 || 
		intersectionPoint[1] < -0.5 || intersectionPoint[1] > 0.5) {
		intersection_occured = false;
	}

	if (intersection_occured && (ray.intersection.none || t_value < ray.intersection.t_value)) {
		intersection_overwrite = true;
		Intersection intersection;
		intersection.normal = worldToModel.transpose() * Vector3D(0, 0, 1);
		intersection.point = modelToWorld * intersectionPoint;
		intersection.t_value = t_value;
		intersection.none = false;
		ray.intersection = intersection;
	}
	
	return intersection_occured && intersection_overwrite;
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

	// uses logic based on http://www.csee.umbc.edu/~olano/435f02/ray-sphere.html
	bool intersection_occured = false;
	bool intersection_overwrite = false;

	double t_value;
	Ray3D rayModelSpace = Ray3D(worldToModel * ray.origin, worldToModel * ray.dir);
	Vector3D rayOriginModelVector = rayModelSpace.origin - Point3D(0, 0, 0); // convert point to vector for dot product purposes

	/*
	 * A point on a unit sphere is given by the equation x^2 + y^2 +z^2 = r^2, or in vector form: p dot p = r^2
	 * A point on our ray is given by pO + td 
	 * Substituting, we get a quadratic equation in t
	*/
	double a = rayModelSpace.dir.dot(rayModelSpace.dir);
	double b = 2 * rayModelSpace.dir.dot(rayOriginModelVector);
	double c = rayOriginModelVector.dot(rayOriginModelVector) - 1;
	
	double discriminant_val = discriminant(a, b, c);
	if (discriminant_val >= 0) { // real roots -> intersection
		double root1, root2;
		quadSolve(a, b, c, root1, root2);
		// we want the smallest t value that is greater than 0
		if (root1 > 0 && root2 > 0 && root1 < root2) {
			t_value = root1;
			intersection_occured = true;
		} else if (root2 > 0) {
			t_value = root2;
			intersection_occured = true;
		} else if (root1 > 0) {
			t_value = root1;
			intersection_occured = true;
		}
	} 

	if (intersection_occured && (ray.intersection.none || t_value < ray.intersection.t_value)) {
		intersection_overwrite = true;
		Intersection intersection;
		Point3D intersectionPointModelSpace = rayModelSpace.point_at(t_value);
		intersection.point = modelToWorld * intersectionPointModelSpace;
		intersection.normal = worldToModel.transpose() * Vector3D(intersectionPointModelSpace[0], intersectionPointModelSpace[1], intersectionPointModelSpace[2]);
		intersection.t_value = t_value;
		intersection.none = false;
		ray.intersection = intersection;
	}

	return intersection_occured && intersection_overwrite;
}

bool NullObject::intersect( Ray3D& ray, const Matrix4x4& worldToModel,
		const Matrix4x4& modelToWorld ) {
    return false;
}

MyTriangle::MyTriangle(Vector3D norm, Point3D a, Point3D b, Point3D c) : n(norm), p0(a), p1(b), p2(c) {}

bool MyTriangle::intersect( Ray3D& ray, const Matrix4x4& worldToModel,
		const Matrix4x4& modelToWorld ) {

	bool intersection_occured = false;
	bool intersection_overwrite = false;

	Ray3D rayModelSpace = Ray3D(worldToModel * ray.origin, worldToModel * ray.dir);

	double denom = rayModelSpace.dir.dot(n);
	if (isSameDouble(denom,0)) {
		return false;
	}
	double t_value = -(rayModelSpace.origin - p0).dot(n) / denom;
	Point3D planeIntersect = rayModelSpace.point_at(t_value);

	// see if the point is contained in the 3 half planes
	if (((p1 - p0).cross(planeIntersect - p0)).dot(n) >= 0 &&
		((p2 - p1).cross(planeIntersect - p1)).dot(n) >= 0 &&
		((p0 - p2).cross(planeIntersect - p2)).dot(n) >= 0) {
		intersection_occured = true;
	}

	if (t_value <= 0) {
		intersection_occured = false;
	}		

	if (intersection_occured && (ray.intersection.none || t_value < ray.intersection.t_value)) {
		intersection_overwrite = true;
		Intersection intersection;
		intersection.normal = n;
		intersection.point = modelToWorld * planeIntersect;
		intersection.t_value = t_value;
		intersection.none = false;
		ray.intersection = intersection;
	}
	
	return intersection_occured && intersection_overwrite;
}


