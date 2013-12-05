/***********************************************************
     Starter code for Assignment 3

     This code was originally written by Jack Wang for
		    CSC418, SPRING 2005

		Implementations of functions in raytracer.h, 
		and the main function which specifies the 
		scene to be rendered.	

***********************************************************/


#include "raytracer.h"
#include "bmp_io.h"
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <vector>
#include <assert.h>
#include "area_light_source.h"
#include <vector>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>
#include "assert.h"

using namespace std;
 
Raytracer::Raytracer() : _lightSource(NULL) {
	_root = new SceneDagNode();
}

Raytracer::~Raytracer() {
	delete _root;
}

SceneDagNode* Raytracer::addObject( SceneDagNode* parent, 
		SceneObject* obj, Material* mat ) {
	SceneDagNode* node = new SceneDagNode( obj, mat );
	node->parent = parent;
	node->next = NULL;
	node->child = NULL;
	
	// Add the object to the parent's child list, this means
	// whatever transformation applied to the parent will also
	// be applied to the child.
	if (parent->child == NULL) {
		parent->child = node;
	}
	else {
		parent = parent->child;
		while (parent->next != NULL) {
			parent = parent->next;
		}
		parent->next = node;
	}
	
	return node;;
}

LightListNode* Raytracer::addLightSource( LightSource* light ) {
	LightListNode* tmp = _lightSource;
	_lightSource = new LightListNode( light, tmp );
	return _lightSource;
}

void Raytracer::rotate( SceneDagNode* node, char axis, double angle ) {
	Matrix4x4 rotation;
	double toRadian = 2*M_PI/360.0;
	int i;
	
	for (i = 0; i < 2; i++) {
		switch(axis) {
			case 'x':
				rotation[0][0] = 1;
				rotation[1][1] = cos(angle*toRadian);
				rotation[1][2] = -sin(angle*toRadian);
				rotation[2][1] = sin(angle*toRadian);
				rotation[2][2] = cos(angle*toRadian);
				rotation[3][3] = 1;
			break;
			case 'y':
				rotation[0][0] = cos(angle*toRadian);
				rotation[0][2] = sin(angle*toRadian);
				rotation[1][1] = 1;
				rotation[2][0] = -sin(angle*toRadian);
				rotation[2][2] = cos(angle*toRadian);
				rotation[3][3] = 1;
			break;
			case 'z':
				rotation[0][0] = cos(angle*toRadian);
				rotation[0][1] = -sin(angle*toRadian);
				rotation[1][0] = sin(angle*toRadian);
				rotation[1][1] = cos(angle*toRadian);
				rotation[2][2] = 1;
				rotation[3][3] = 1;
			break;
		}
		if (i == 0) {
		    node->trans = node->trans*rotation; 	
			angle = -angle;
		} 
		else {
			node->invtrans = rotation*node->invtrans; 
		}	
	}
}

void Raytracer::translate( SceneDagNode* node, Vector3D trans ) {
	Matrix4x4 translation;
	translation[0][3] = trans[0];
	translation[1][3] = trans[1];
	translation[2][3] = trans[2];
	node->trans = node->trans*translation; 	
	translation[0][3] = -trans[0];
	translation[1][3] = -trans[1];
	translation[2][3] = -trans[2];
	node->invtrans = translation*node->invtrans; 
}

void Raytracer::scale( SceneDagNode* node, Point3D origin, double factor[3] ) {
	Matrix4x4 scale;
	
	scale[0][0] = factor[0];
	scale[0][3] = origin[0] - factor[0] * origin[0];
	scale[1][1] = factor[1];
	scale[1][3] = origin[1] - factor[1] * origin[1];
	scale[2][2] = factor[2];
	scale[2][3] = origin[2] - factor[2] * origin[2];
	node->trans = node->trans*scale; 	
	scale[0][0] = 1/factor[0];
	scale[0][3] = origin[0] - 1/factor[0] * origin[0];
	scale[1][1] = 1/factor[1];
	scale[1][3] = origin[1] - 1/factor[1] * origin[1];
	scale[2][2] = 1/factor[2];
	scale[2][3] = origin[2] - 1/factor[2] * origin[2];
	node->invtrans = scale*node->invtrans; 
}

Matrix4x4 Raytracer::initInvViewMatrix( Point3D eye, Vector3D view, 
		Vector3D up ) {
	Matrix4x4 mat; 
	Vector3D w;
	view.normalize();
	up = up - up.dot(view)*view;
	up.normalize();
	w = view.cross(up);

	mat[0][0] = w[0];
	mat[1][0] = w[1];
	mat[2][0] = w[2];
	mat[0][1] = up[0];
	mat[1][1] = up[1];
	mat[2][1] = up[2];
	mat[0][2] = -view[0];
	mat[1][2] = -view[1];
	mat[2][2] = -view[2];
	mat[0][3] = eye[0];
	mat[1][3] = eye[1];
	mat[2][3] = eye[2];

	return mat; 
}

void Raytracer::traverseScene( SceneDagNode* node, Ray3D& ray, Matrix4x4 modelToWorld, Matrix4x4 worldToModel ) {
	SceneDagNode *childPtr;

	// Applies transformation of the current node to the global
	// transformation matrices.
	Matrix4x4 newModelToWorld = modelToWorld*node->trans;
	Matrix4x4 newWorldToModel = node->invtrans*worldToModel; 
	if (node->obj) {
		// Perform intersection.
		if (node->obj->intersect(ray, newWorldToModel, newModelToWorld)) {
			ray.intersection.mat = node->mat;
		}
	}
	// Traverse the children.
	childPtr = node->child;
	while (childPtr != NULL) {
		traverseScene(childPtr, ray, newModelToWorld, newWorldToModel);
		childPtr = childPtr->next;
	}

}

void Raytracer::computeShading( Ray3D& ray ) {
	LightListNode* curLight = _lightSource;
	for (;;) {
		if (curLight == NULL) {
			break;
		}
		// Each lightSource provides its own shading function.
		LightSource* lightSource = curLight->light;

		// Implement shadows here if needed.
		#ifdef SHADOWS
			/* HARD SHADOWS */
			// cast a ray out from the light to the object and see if the intersection is the same one found by the current ray. If not, its in shadow, so we skip shading
	    	Vector3D lightToObject = ray.intersection.point - lightSource->get_position();
	    	lightToObject.normalize();
	    	Ray3D rayLightToObjectWorldSpace = Ray3D(lightSource->get_position(), lightToObject);
	    	traverseScene(_root, rayLightToObjectWorldSpace);
	    	if (rayLightToObjectWorldSpace.intersection.point == ray.intersection.point) {
				lightSource->shade(ray);
	    	}
	    #else 
	    	lightSource->shade(ray);
	    #endif

		curLight = curLight->next;
	}
	// even points in shadows get ambient lighting
	if (LightSource::RENDER_TYPE != SCENE_SIGNATURE) {
		Colour col = ray.col + ray.intersection.mat->ambient * getAmbientLight();
		col.clamp();
		ray.col = col;
	}
}

void Raytracer::initPixelBuffer() {
	int numbytes = _scrWidth * _scrHeight * sizeof(unsigned char);
	_rbuffer = new unsigned char[numbytes];
	_gbuffer = new unsigned char[numbytes];
	_bbuffer = new unsigned char[numbytes];
	for (int i = 0; i < _scrHeight; i++) {
		for (int j = 0; j < _scrWidth; j++) {
			_rbuffer[i*_scrWidth+j] = 0;
			_gbuffer[i*_scrWidth+j] = 0;
			_bbuffer[i*_scrWidth+j] = 0;
		}
	}
}

void Raytracer::flushPixelBuffer( char *file_name ) {
	bmp_write( file_name, _scrWidth, _scrHeight, _rbuffer, _gbuffer, _bbuffer );
	delete _rbuffer;
	delete _gbuffer;
	delete _bbuffer;
}

/* Put the reflected direction in r */
void reflect(Vector3D& d, Vector3D& n, Vector3D& r) {
	r = d - (2 * (d.dot(n)) * n);
	r.normalize();
}

/* Put the refracted direction in t. Returns false if total internal refraction */
bool refract(Vector3D d, Vector3D n, double n1, Vector3D& t) {
	double ddotn = d.dot(n);
	double n2 = 1.0; // assume air - basically all refractive objects are hollow
	double sqrtTerm = sqrt(1.0 - ((pow(n1, 2) * (1 - pow(ddotn, 2))) / pow(n2, 2)));
	if (sqrtTerm < 0) {
		return false;
	} 
	Vector3D term1 = (1 / n2) * (n1 * (d - (ddotn * n)));
	Vector3D term2 = sqrtTerm * n;
	t = term1 - term2;
	t.normalize();
	return true;
}


Colour Raytracer::shadeRay( Ray3D& ray, int depth, bool debug ) {
	Colour col(0.0, 0.0, 0.0); 

	if (depth <= 0) {
		return col;
	}

	traverseScene(_root, ray); 
	
	// Don't bother shading if the ray didn't hit 
	// anything.
	if (!ray.intersection.none) {
		computeShading(ray); 
		col = ray.col;

	// You'll want to call shadeRay recursively (with a different ray, 
	// of course) here to implement reflection/refraction effects.  

		// Refraction effects:
		if (ray.intersection.mat->isDieletric()) {
			// algorithm from textbook for dieletrics
			Vector3D d = ray.dir;
			d.normalize();
			Vector3D n = ray.intersection.normal;
			n.normalize();

			double c, kr, kg, kb;
			Colour k;
			Vector3D t;

			Vector3D r;
			reflect(d, n, r);

			bool totalInternalReflection = false;

			Ray3D reflectionRay = Ray3D(ray.intersection.point + EPSILON * r, r);

			if (d.dot(n) < 0) {
				refract(d, n, ray.intersection.mat->indexOfRefraction, t);
				c = -d.dot(n);
				k = Colour(1.0, 1.0, 1.0);
			} else {
				double t_val = ray.intersection.t_value;
				double ar = 0.15 * ray.intersection.mat->diffuse[0]; 
				double ag = 0.15 * ray.intersection.mat->diffuse[1];
				double ab = 0.15 * ray.intersection.mat->diffuse[2];
				kr = exp(-ar * t_val);
				kg = exp(-ag * t_val);
				kb = exp(-ab * t_val);
				k = Colour(kr, kg, kb);
				if (refract(d, -n, 1.0 / ray.intersection.mat->indexOfRefraction, t)) {
					c = t.dot(n);
				}
                else {
				    col = col + k * shadeRay(reflectionRay, depth - 1);
					totalInternalReflection = true;
				}
			}
			if (!totalInternalReflection) {
				Ray3D transmittedRay = Ray3D(ray.intersection.point + EPSILON * t, t);
				double r0 = pow(ray.intersection.mat->indexOfRefraction - 1, 2) / pow(ray.intersection.mat->indexOfRefraction + 1, 2);
				double R = r0 + (1 - r0) * pow(1 - c, 5);
				Colour reflect = R * shadeRay(reflectionRay, depth - 1); 
				Colour refract = (1 - R) * shadeRay(transmittedRay, depth - 1, true);
				col = col + k * (refract + reflect);
			}
			// material wasn't refractive, but it is reflective:
		} else if (!(ray.intersection.mat->specular == Colour(0, 0, 0))) {
				Vector3D oppositeRayDir = -ray.dir;
				oppositeRayDir.normalize();
				Vector3D normal = ray.intersection.normal;
				normal.normalize();

				Vector3D reflectionDirection = 2 * (oppositeRayDir.dot(normal)) * normal - oppositeRayDir;
				reflectionDirection.normalize();
				Ray3D reflectionRay = Ray3D(ray.intersection.point + 0.001 * reflectionDirection, reflectionDirection);
				Colour reflectionColour = shadeRay(reflectionRay, depth - 1);

				Vector3D distanceVector = ray.intersection.point - reflectionRay.intersection.point;
				double distance = distanceVector.length();
				double dampingFactor = 1.0 / pow(distance, 2.0);
				col = col + dampingFactor * reflectionColour * ray.intersection.mat->specular;
		}
	}

	col.clamp();
	return col; 
}	

double fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
//    cout << "f is " << f << endl;
    return fMin + f * (fMax - fMin);
}


void Raytracer::render( int width, int height, Point3D eye, Vector3D view, 
		Vector3D up, double fov, char* fileName ) {

	_scrWidth = width;
	_scrHeight = height;
    srand(1);

	initPixelBuffer();
	pthread_t threads[NUM_THREADS];
	// distribute the work among threads by splitting the image into segments
	for (int k = 0; k < NUM_THREADS; k++) {
		// Give threads the variables they need
		ThreadParam* threadParam = (ThreadParam*) malloc(sizeof(ThreadParam));
		threadParam->iStart = k * height / NUM_THREADS;
		threadParam->iEnd = (k + 1) * height / NUM_THREADS;
		threadParam->eye = eye;
		threadParam->view = view;
		threadParam->up = up;
		threadParam->fov = fov;
		threadParam->r = this;
		pthread_create(&threads[k], NULL, &Raytracer::tracer_helper, threadParam); 
	}

	// wait for threads to finish
	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	flushPixelBuffer(fileName);
}
 
void Raytracer::doRender(int iStart, int iEnd, Point3D eye, Vector3D view, Vector3D up, double fov) {
	// Construct a ray for each pixel.
	int width = _scrWidth;
	int height = _scrHeight;
	Matrix4x4 viewToWorld;

	double factor = (double(height)/2)/tan(fov*M_PI/360.0);
    double z_focus_intersect = FOCAL_DISTANCE;
    double x_focus_intersect, y_focus_intersect;
	viewToWorld = initInvViewMatrix(eye, view, up);

	for (int i = iStart; i < iEnd; i++) {
		for (int j = 0; j < _scrWidth; j++) {
			// Sets up ray origin and direction in view space, 
			// image plane is at z = -1.
			Point3D origin(0, 0, 0);
			Point3D imagePlane;
			imagePlane[0] = (-double(width)/2 + 0.5 + j)/factor;
			imagePlane[1] = (-double(height)/2 + 0.5 + i)/factor;
			imagePlane[2] = -1;

#ifdef DOF
            Colour col;
            // Cast ray from center of eye (center of aperture), through pixel of interest
            // to the focus plane.
            // Find point of intersection.
            Vector3D ray_dir = imagePlane - origin;
            ray_dir.normalize();
            double t_intersect = z_focus_intersect / ray_dir[2];
            double x_focus_intersect = t_intersect * ray_dir[0];
            double y_focus_intersect = t_intersect * ray_dir[1]; 
            
            Point3D focus_point(x_focus_intersect, y_focus_intersect, z_focus_intersect);

            // Randomly cast rays from within aperture towards the focus point and capture color.
            for (int k = 0; k < NUM_APERTURE_RAYS; k++) {
                double aperture_theta  = fRand(0, 2 * (double) M_PI);
                double aperture_radius = fRand(0, (double) APERTURE);
                Point3D ray_origin(cos(aperture_theta) * aperture_radius, sin(aperture_theta) * aperture_radius, 0); 	
                Ray3D rayViewSpace(ray_origin, focus_point - ray_origin);     
			    Ray3D rayWorldSpace(viewToWorld * rayViewSpace.origin, viewToWorld * rayViewSpace.dir);
                
                // Sum up final colour
                col = col + shadeRay(rayWorldSpace, SHADE_DEPTH);
            }
          
            // Find the final average colour
            col = (double) 1.0 / NUM_APERTURE_RAYS * col;
#else
            // TODO: Convert ray to world space and call 
            // shadeRay(ray) to generate pixel colour.         
            Ray3D rayViewSpace(origin, imagePlane - origin);
            Ray3D rayWorldSpace(viewToWorld * rayViewSpace.origin, viewToWorld * rayViewSpace.dir);
            
			Colour col = shadeRay(rayWorldSpace, SHADE_DEPTH); 
#endif
            _rbuffer[i*width+j] = int(col[0]*255);
			_gbuffer[i*width+j] = int(col[1]*255);
			_bbuffer[i*width+j] = int(col[2]*255);
		}
	}
}

void Raytracer::setAmbientLight(Colour colour) {
	ambientLight = colour;
}
Colour Raytracer::getAmbientLight() {
	return ambientLight;
}

SceneDagNode* Raytracer::loadTriangeMesh(string filename, Material* material) {

  	//open text file for input
	ifstream infile(filename.c_str(), ios::in);
	if(!infile) {
		std::cerr <<" failed to open file\n";
		exit(-1);
	}

	// read in coords
	vector<Point3D> coords;
	string textline;
	while(getline(infile, textline, '\n')) {
		double a, b, c;
		stringstream ss(textline);
		ss >> a >> b >> c;
		coords.push_back(Point3D(a, b, c));
	}
    SceneDagNode* meshContainer = addObject( new NullObject(), material );

	// format: normal, 3 coords, normal, 3 coords etc.
    assert(coords.size() % 4 == 0);
	for (unsigned int i = 0; i < coords.size(); i+=4) {
		MyTriangle* myTriangle = new MyTriangle(
					 Vector3D (coords[i] - Point3D(0, 0, 0)),
					 Point3D  (coords[i+1]),
					 Point3D  (coords[i+2]),
					 Point3D  (coords[i+3]));
        addObject(meshContainer, myTriangle, material);
    }

    return meshContainer;

}


/* The default scene, given with the assignment */
void defaultScene(Raytracer& raytracer) {

}

/* Defines materials */
Material gold( Colour(0.3, 0.3, 0.3), Colour(0.75164, 0.60648, 0.22648), 
                Colour(0.628281, 0.555802, 0.366065), 
                51.2 );
Material red( Colour(0.7, 0, 0), Colour(0.7, 0.05, 0.05), 
                Colour(0.916228, 0.616228, 0.516228), 
                45.0 );
Material blue( Colour(0, 0, 0.7), Colour(0, 0.05, 0.8), 
                Colour(0.916228, 0.616228, 0.516228), 
                45.0 );
Material jade( Colour(0, 0, 0), Colour(0.54, 0.89, 0.63), 
                Colour(0.316228, 0.316228, 0.316228), 
                12.8 );
Material shiny( Colour(0, 0, 0), Colour(0.54, 0.0, 0.63),
                Colour(0.0, 0.0, 0.0), 
                30.0 );
Material highSphere( Colour(0, 0, 0), Colour(0.7, 0.05, 0.05), 
                Colour(0.916228, 0.616228, 0.516228), 
                45.0 );
Material weird( Colour(0.4, 0, 0.7), Colour(0.1, 0.445, 0.95), 
                Colour(0.228, 0.628, 0.58), 
                12.0 );

void spaceInvaders(Raytracer& raytracer) {

	// Define ambient lighting
	raytracer.setAmbientLight(Colour(0.9, 0.9, 0.9));
    // Defines a point light source.
    raytracer.addLightSource( new PointLight(Point3D(5, 40, -40), 
                            Colour(0.9, 0.9, 0.9) ) );


    double factor[3] = { 0.2, 0.2, 0.2 };
    double wallFactor[3] = { 100.0, 100.0, 100.0 };

    // Middle Row 
	SceneDagNode* space_invaderMiddleMiddle = raytracer.loadTriangeMesh("space_invader.stl", &gold);
    raytracer.translate(space_invaderMiddleMiddle, Vector3D(0, 25, -70));
	raytracer.scale(space_invaderMiddleMiddle, Point3D(0, 0, 0), factor);
	SceneDagNode* space_invaderMiddleLeft = raytracer.loadTriangeMesh("space_invader.stl", &gold);
    raytracer.translate(space_invaderMiddleLeft, Vector3D(-13, 10, -75));
	raytracer.scale(space_invaderMiddleLeft, Point3D(0, 0, 0), factor);

	SceneDagNode* space_invaderMiddleRight = raytracer.loadTriangeMesh("space_invader.stl", &gold);
    raytracer.translate(space_invaderMiddleRight, Vector3D(13, 10, -65));
	raytracer.scale(space_invaderMiddleRight, Point3D(0, 0, 0), factor);

	SceneDagNode* wall = raytracer.addObject( new UnitSquare(), &jade );
	raytracer.translate(wall, Vector3D(0, 0, -80));
	raytracer.scale(wall, Point3D(0, 0, 0), wallFactor); 

}

void shapeScene(Raytracer& raytracer) {

    // Defines a point light source.
    raytracer.setAmbientLight(Colour(0.9, 0.9, 0.9));
    raytracer.addLightSource( new AreaLight(Point3D(0, 0, 5), Vector3D(0, 1, 0), Vector3D(1, 0, 0), 0.3, 0.3,  
                            Colour(0.9, 0.9, 0.9), raytracer ));

    // Add a unit square into the scene with material mat.
    SceneDagNode* sphere = raytracer.addObject( new UnitSphere(), &gold );
    SceneDagNode* sphere2 = raytracer.addObject( new UnitSphere(), &shiny );
    SceneDagNode* sphere3 = raytracer.addObject( new UnitSphere(), &highSphere );

    SceneDagNode* plane = raytracer.addObject( new UnitSquare(), &jade );
    SceneDagNode* cylinder = raytracer.addObject( new UnitCylinder(), &weird );

    // Apply some transformations to the unit square.
    double factor1[3] = { 1.0, 2.0, 1.0 };
    double factor2[3] = { 6.0, 6.0, 6.0 };
    double factor3[3] = { 0.4, 0.4, 0.4 };
    double cylinder_scale[3] = { 1.0, 2.0, 1.0 };

    raytracer.scale(cylinder, Point3D(0, 0, 0), cylinder_scale);
    raytracer.translate(cylinder, Vector3D(3, 1, -5));


    raytracer.translate(sphere, Vector3D(0, 0, -5));        
    raytracer.rotate(sphere, 'x', -45); 
    raytracer.rotate(sphere, 'z', 45); 
    raytracer.scale(sphere, Point3D(0, 0, 0), factor1);

    raytracer.translate(sphere2, Vector3D(-3, 0, -5));        

    raytracer.scale(sphere3, Point3D(0, 0, 0), factor3);
    raytracer.translate(sphere3, Vector3D(0, 1, -4));        

    raytracer.scale(cylinder, Point3D(0, 0, 0), factor3);
    raytracer.translate(cylinder, Vector3D(-1, -1, -1));


    raytracer.translate(plane, Vector3D(0, 0, -7));        
    raytracer.rotate(plane, 'z', 45); 
    raytracer.scale(plane, Point3D(0, 0, 0), factor2);
}

void quadraticScene(Raytracer& raytracer) {
    // Defines a point light source.
    raytracer.setAmbientLight(Colour(0.9, 0.9, 0.9));
    // Defines a point light source.
    raytracer.addLightSource( new PointLight(Point3D(2, 0, 5), 
                            Colour(0.9, 0.9, 0.9) ) );

    // Add a unit square into the scene with material mat.
    SceneDagNode* cone1 = raytracer.addObject( new UnitCone(), &gold );
    SceneDagNode* cylinder1 = raytracer.addObject( new UnitCylinder(), &jade );
    SceneDagNode* cone2 = raytracer.addObject( new UnitCone(), &red );
    SceneDagNode* cylinder2 = raytracer.addObject( new UnitCylinder(), &blue );
    SceneDagNode* plane = raytracer.addObject( new UnitSquare(), &jade );


    double cylinder_scale[3] = { 1.0, 2.0, 1.0 };
    double cone_scale[3] = { 2.0, 5.0, 2.0 };
    double factor2[3] = { 100.0, 100.0, 6.0 };

    raytracer.translate(cylinder1, Vector3D(3, -2, -5));
    raytracer.scale(cylinder1, Point3D(0, 0, 0), cylinder_scale);

    raytracer.translate(cone1, Vector3D(-3, -3, -5));
    raytracer.scale(cone1, Point3D(0,0,0), cone_scale);

    raytracer.translate(cone2, Vector3D(0, -3, -5));

    raytracer.translate(cylinder2, Vector3D(3, 3, -10));

    raytracer.translate(plane, Vector3D(0, 0, -12));        
    raytracer.scale(plane, Point3D(0, 0, 0), factor2);

}

int main(int argc, char* argv[])
{	
	// Build your scene and setup your camera here, by calling 
	// functions from Raytracer.  The code here sets up an example
	// scene and renders it from two different view points, DO NOT
	// change this if you're just implementing part one of the 
	// assignment.  
	Raytracer raytracer;
	int width = 320; 
	int height = 240; 

	if (argc == 3) {
		width = atoi(argv[1]);
		height = atoi(argv[2]);
	}

	assert(height % NUM_THREADS == 0);


	Scene scene = SPACE_INVADERS;
	/* Define scene objects and transformations here */
	switch(scene) {
		case DEFAULT:
			defaultScene(raytracer);
			break;
		case SPACE_INVADERS:
			spaceInvaders(raytracer);
			break;
		case SHAPE_SCENE:
			shapeScene(raytracer); 
			break;
		case QUADRATIC_SCENE:
			quadraticScene(raytracer);
			break;
		default:
			throw "No scene set";
	}

	// Camera parameters.
	Point3D eye(0, 0, 1);
	Vector3D view(0, 0, -1);
	Vector3D up(0, 1, 0);
	double fov = 60;

	// Render the scene, feel free to make the image smaller for
	// testing purposes.	
	raytracer.render(width, height, eye, view, up, fov, (char*) "view1.bmp");
	
	// Render it from a different point of view.
	Point3D eye2(4, 2, 1);
	Vector3D view2(-1, 0, -6);
	raytracer.render(width, height, eye2, view2, up, fov, (char*) "view2.bmp");
	
	return 0;
}

