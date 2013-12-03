#include "light_source.h"
#include "raytracer.h"

// A rectangular light source
class AreaLight : public LightSource {
public:
	AreaLight( Point3D pos, Vector3D u, Vector3D v, double uLength, double vLength, Colour col, Raytracer& r ) : pos(pos), u(u), v(v), uLength(uLength), vLength(vLength), r(r) {
		v.normalize();
		u.normalize();
		// weaken the contribution of each ray we cast
		specular = (1.0 / NUM_SHADOW_RAYS) * col;
		diffuse = (1.0 / NUM_SHADOW_RAYS) * col;
	}
	Point3D get_position() const { return pos; }
	void shade( Ray3D& );
private:
	Point3D pos;
	Vector3D u;
	Vector3D v;
	double uLength;
	double vLength;
	Colour diffuse; 
	Colour specular; 
	Raytracer& r;
};
