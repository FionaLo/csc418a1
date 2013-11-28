#include "scene_object.h"
#include "util.h"
#include <vector>
#include <algorithm>

using namespace std;

bool UnitCylinder::intersect( Ray3D& ray, const Matrix4x4& worldToModel,
		const Matrix4x4& modelToWorld ) {

	bool intersection_occured = false;
	bool intersection_overwrite = false;

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
	double c = pow(px, 2) + pow(pz, 2) - 1;

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

		// essentially before we checked if we hit an infinite cylinder. let's take height into account
		if (possible_t_values.size() > 0) {
			double y1 = rayModelSpace.point_at(root1)[1];
			double y2 = rayModelSpace.point_at(root2)[1];
			// we hit the bottom
			if (y1 > -0.5 && y2 < -0.5 || y1 < -0.5 && y2 > -0.5) {
				possible_t_values.push_back((-0.5 - py) / dy);
			} 
			// we hit the top
			if (y1 > 0.5 && y2 < 0.5 || y1 < 0.5 && y2 > 0.5) {
				possible_t_values.push_back((0.5 - py) / dy);
			}
		}
	} 

	// we want the smallest t value that is greater than 0 and meets the height requirements:
	std::vector<double> qualifying_t_values;
	for(unsigned int i = 0; i < possible_t_values.size(); i++) {
		double t = possible_t_values[i];
	    if (t > 0) {
	    	Point3D point = rayModelSpace.point_at(t);
	    	double y_val = point[1];
	    	if (y_val <= 0.5 && y_val >= -0.5) {
				qualifying_t_values.push_back(t);
	    	}
	    }
	}
	intersection_occured = qualifying_t_values.size() > 0;
	if (intersection_occured) {
		t_value = *std::min_element(qualifying_t_values.begin(), qualifying_t_values.end());
	}
	

	if (intersection_occured && (ray.intersection.none || t_value < ray.intersection.t_value)) {
		intersection_overwrite = true;
		Intersection intersection;
		Point3D intersectionPointModelSpace = rayModelSpace.point_at(t_value);
		intersection.point = modelToWorld * intersectionPointModelSpace;
		// TODO:
		intersection.normal = worldToModel.transpose() * Vector3D(intersectionPointModelSpace[0], intersectionPointModelSpace[1], intersectionPointModelSpace[2]);
		intersection.t_value = t_value;
		intersection.none = false;
		ray.intersection = intersection;
	}

	return intersection_occured && intersection_overwrite;
}
