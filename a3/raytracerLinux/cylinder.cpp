#include "scene_object.h"
#include "util.h"

bool UnitCylinder::intersect( Ray3D& ray, const Matrix4x4& worldToModel,
		const Matrix4x4& modelToWorld ) {

	bool intersection_occured = false;
	bool intersection_overwrite = false;

	double t_value;
	Ray3D rayModelSpace = Ray3D(worldToModel * ray.origin, worldToModel * ray.dir);

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

		// // essentially before we checked if we hit an infinite cylinder. let's take height into account
		if (intersection_occured) {
			double y1 = rayModelSpace.point_at(root1)[1];
			double y2 = rayModelSpace.point_at(root2)[1];
			bool hit_bottom = false;

			// // we hit the bottom
			// if (y1 > -0.5 && y2 < -0.5 || y1 < -0.5 && y2 > -0.5) {
			// 	hit_bottom = true;
			// 	t_value = (-0.5 - py) / dy;
			// } 
			// else if (y1 > 0.5 && y2 < 0.5 || y1 < 0.5 && y2 > 0.5) {
			// // we hit the top
			// 	double top_t_val = (0.5 - py) / dy;
			// 	if (!hit_bottom) {
			// 		t_value = top_t_val;
			// 	} else {
			// 		t_value = top_t_val < t_value ? top_t_val : t_value;
			// 	}
			// }


			Point3D point = rayModelSpace.point_at(t_value);
			// one final height check
			if (point[1] < -0.5 || point[1] > 0.5) {
				intersection_occured = false;
			}
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
