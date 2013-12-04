#include "area_light_source.h"
#include <stdlib.h>

void AreaLight::shade( Ray3D& ray ) {
    PointLight pointLight = PointLight(Point3D(), diffuse, specular);

    for (unsigned int i = 0; i < NUM_SHADOW_RAYS; i++) {

        double uScale = (double)rand() / RAND_MAX;
        double vScale = (double)rand() / RAND_MAX;
        // pick a point somewhere on the light
        Point3D pointOnLight = pos + uScale * uLength * u + vScale * vLength * v;  
        Vector3D lightToObject = ray.intersection.point - pointOnLight;
        lightToObject.normalize();
        Ray3D rayLightToObjectWorldSpace = Ray3D(pointOnLight, lightToObject);
        Matrix4x4 modelToWorld;
        Matrix4x4 worldToModel;
        r.traverseScene(r._root, rayLightToObjectWorldSpace, modelToWorld, worldToModel);

        if (rayLightToObjectWorldSpace.intersection.point == ray.intersection.point) {
            pointLight.setPosition(pointOnLight);
            pointLight.shade(ray);
        }

    }

}