/***********************************************************
     Starter code for Assignment 3

     This code was originally written by Jack Wang for
		    CSC418, SPRING 2005

		classes defining primitives in the scene

***********************************************************/

#include "util.h"

// All primitives should provide a intersection function.  
// To create more primitives, inherit from SceneObject.
// Namely, you can create, Sphere, Cylinder, etc... classes
// here.
class SceneObject {
public:
	// Returns true if an intersection occured, false otherwise.
	virtual bool intersect( Ray3D&, const Matrix4x4&, const Matrix4x4& ) = 0;
};

// Example primitive you can create, this is a unit square on 
// the xy-plane.
class UnitSquare : public SceneObject {
public:
	bool intersect( Ray3D& ray, const Matrix4x4& worldToModel,
			const Matrix4x4& modelToWorld );
};

class UnitSphere : public SceneObject {
public:
	bool intersect( Ray3D& ray, const Matrix4x4& worldToModel,
			const Matrix4x4& modelToWorld );
};

class UnitCylinder : public SceneObject {
public:
	bool intersect( Ray3D& ray, const Matrix4x4& worldToModel,
			const Matrix4x4& modelToWorld );
};

class MyTriangle : public SceneObject {
public:
	MyTriangle(Vector3D norm, Point3D a, Point3D b, Point3D c);
	bool intersect( Ray3D& ray, const Matrix4x4& worldToModel,
			const Matrix4x4& modelToWorld );

private:
	Point3D p0, p1, p2;
	Vector3D n;
};



static double discriminant(double a, double b, double c) {
	return (b * b) - (4 * a * c);
}

static void quadSolve(double a, double b, double c, double &root1, double &root2) {
	double term1 = -b;
	double term2 = sqrt(discriminant(a, b, c));
	root1 = (term1 + term2) / (2 * a);
	root2 = (term1 - term2) / (2 * a);
}
