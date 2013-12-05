#include "util.h"
#include "scene_object.h"
#include <vector>
#include <algorithm>

using namespace std;

bool inUnitCircle(Point3D point) {
	double x = point[0];
	double z = point[2];
	return pow(x, 2) + pow(z, 2) <= 1.0;
}

bool betweenClosedRange(double val, double lowerBound, double upperBound) {
	return val >= lowerBound && val <= upperBound;
}

bool UnitCylinder::intersect( Ray3D& ray, const Matrix4x4& worldToModel,
		const Matrix4x4& modelToWorld ) {

	// a cylinder: unit circle in xz plane, y from -0.5 to 0.5

	bool intersection_occured = false;
	bool intersection_overwrite = false;
	bool hit_top = false;
	bool hit_bottom = false;

	double t_value;
	Ray3D rayModelSpace = Ray3D(worldToModel * ray.origin, worldToModel * ray.dir);
	vector<double> possible_t_values;

	double dx = rayModelSpace.dir[0];
	double dy = rayModelSpace.dir[1];
	double dz = rayModelSpace.dir[2];

	double px = rayModelSpace.origin[0];
	double py = rayModelSpace.origin[1];
	double pz = rayModelSpace.origin[2];

	double a = pow(dx, 2) + pow(dz, 2);
	double b = 2 * (px * dx + pz * dz);
	double c = pow(px, 2) + pow(pz, 2) - 1.0;

	double discriminant_val = discriminant(a, b, c);
	if (discriminant_val >= 0) { // real roots -> intersection
		double root1, root2;
		quadSolve(a, b, c, root1, root2);
		if (root1 > 0) {
			possible_t_values.push_back(root1);
		}
		if (root2 > 0) {
			possible_t_values.push_back(root2);
		}
	}
	// check top and bottom also:
	double t_top = (0.5 - py) / dy;
	double t_bottom = (-0.5 - py) / dy;
	if (inUnitCircle(rayModelSpace.point_at(t_top))) {
		possible_t_values.push_back(t_top);	
	}
	if (inUnitCircle(rayModelSpace.point_at(t_bottom))) {
		possible_t_values.push_back(t_bottom);	
	}
	
	// we want the smallest t value that is greater than 0 and meets the height requirements:
	std::vector<double> qualifying_t_values;
	for(unsigned int i = 0; i < possible_t_values.size(); i++) {
		double t = possible_t_values[i];
	    if (t > 0) {
	    	Point3D point = rayModelSpace.point_at(t);
	    	double y_val = point[1];
	    	if (betweenClosedRange(y_val, -0.5, 0.5)) {
				qualifying_t_values.push_back(t);
	    	}
	    }
	}
	intersection_occured = qualifying_t_values.size() > 0;
	if (intersection_occured) {
		t_value = *std::min_element(qualifying_t_values.begin(), qualifying_t_values.end());
		if (isSameDouble(t_value, t_top)) {
			hit_top = true;
		} else if (isSameDouble(t_value, t_bottom)) {
			hit_bottom = true;
		} 
	}
	

	if (intersection_occured && (ray.intersection.none || t_value < ray.intersection.t_value)) {
		intersection_overwrite = true;
		Intersection intersection;
		Point3D intersectionPointModelSpace = rayModelSpace.point_at(t_value);
		intersection.point = modelToWorld * intersectionPointModelSpace;
		Vector3D normal;
		if (hit_top) {
			normal = Vector3D(0, 1.0, 0); 
		} else if (hit_bottom) {
			normal = Vector3D(0, -1.0, 0);
		} else {
			normal = Vector3D(intersectionPointModelSpace[0], 0, intersectionPointModelSpace[2]);
		}
		normal.normalize();
		intersection.normal = worldToModel.transpose() * normal;
		intersection.t_value = t_value;
		intersection.none = false;
		ray.intersection = intersection;
	}

	return intersection_occured && intersection_overwrite;
}

bool UnitCone::intersect( Ray3D& ray, const Matrix4x4& worldToModel,
		const Matrix4x4& modelToWorld ) {

	// a cone: unit circle in xz plane, y from 0 to 1

	bool intersection_occured = false;
	bool intersection_overwrite = false;
	bool hit_bottom = false;

	double t_value;
	Ray3D rayModelSpace = Ray3D(worldToModel * ray.origin, worldToModel * ray.dir);
	vector<double> possible_t_values;

	double dx = rayModelSpace.dir[0];
	double dy = rayModelSpace.dir[1];
	double dz = rayModelSpace.dir[2];

	double px = rayModelSpace.origin[0];
	double py = rayModelSpace.origin[1];
	double pz = rayModelSpace.origin[2];

	double a = pow(dx, 2) + pow(dz, 2) - pow(dy, 2);
	double b = 2 * (px * dx + pz * dz) + 2 * dy - 2 * dy * py;
	double c = pow(px, 2) + pow(pz, 2) - pow(py, 2) - 1.0 + 2.0 * py;

	double discriminant_val = discriminant(a, b, c);
	if (discriminant_val >= 0) { // real roots -> intersection
		double root1, root2;
		quadSolve(a, b, c, root1, root2);
		if (root1 > 0) {
			possible_t_values.push_back(root1);
		}
		if (root2 > 0) {
			possible_t_values.push_back(root2);
		}
	}
	// check bottom also:
	double t_bottom = -py / dy;
	if (inUnitCircle(rayModelSpace.point_at(t_bottom))) {
		possible_t_values.push_back(t_bottom);	
	}
	
	// we want the smallest t value that is greater than 0 and meets the height requirements:
	std::vector<double> qualifying_t_values;
	for(unsigned int i = 0; i < possible_t_values.size(); i++) {
		double t = possible_t_values[i];
	    if (t > 0) {
	    	Point3D point = rayModelSpace.point_at(t);
	    	double y_val = point[1];
	    	if (betweenClosedRange(y_val, 0, 1)) {
				qualifying_t_values.push_back(t);
	    	}
	    }
	}
	intersection_occured = qualifying_t_values.size() > 0;
	if (intersection_occured) {
		t_value = *std::min_element(qualifying_t_values.begin(), qualifying_t_values.end());
		if (isSameDouble(t_value, t_bottom)) {
			hit_bottom = true;
		} 
	}
	

	if (intersection_occured && (ray.intersection.none || t_value < ray.intersection.t_value)) {
		intersection_overwrite = true;
		Intersection intersection;
		Point3D intersectionPointModelSpace = rayModelSpace.point_at(t_value);
		intersection.point = modelToWorld * intersectionPointModelSpace;
		Vector3D normal;
		if (hit_bottom) {
			normal = Vector3D(0, 0, -1.0);
		} else {
			normal = Vector3D(intersectionPointModelSpace[0], 1 - intersectionPointModelSpace[1], intersectionPointModelSpace[2]);
		}
		normal.normalize();
		intersection.normal = worldToModel.transpose() * normal;
		intersection.t_value = t_value;
		intersection.none = false;
		ray.intersection = intersection;
	}

	return intersection_occured && intersection_overwrite;
}

