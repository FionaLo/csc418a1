
        case REFRACTION_DEMO:
            refractionDemo(raytracer,glass,jade,gold,weird,shiny);
            break;
        case DOF_DEMO:
            dofDemo(raytracer,glass,jade,gold,weird,shiny);
            break;



void refractionDemo(Raytracer& raytracer, Material& glass, Material& jade, Material& gold, Material& weird, Material& shiny ){
	// Defines a point light source.
	raytracer.setAmbientLight(Colour(0.9, 0.9, 0.9));
	raytracer.addLightSource( new PointLight(Point3D(0, 0, 5), 
				Colour(0.9, 0.9, 0.9)) );
	raytracer.addLightSource( new PointLight(Point3D(0, 5, -5), 
				Colour(0.4, 0.4, 0.4)) );


	// Add a unit square into the scene with material mat.
	SceneDagNode* sphere = raytracer.addObject( new UnitSphere(), &glass );
	SceneDagNode* sphere2 = raytracer.addObject( new UnitSphere(), &jade );
	SceneDagNode* sphere3 = raytracer.addObject( new UnitSphere(), &glass );
	SceneDagNode* plane = raytracer.addObject( new UnitSquare(), &gold );
	SceneDagNode* plane2 = raytracer.addObject( new UnitSquare(), &weird );
	SceneDagNode* plane3 = raytracer.addObject( new UnitSquare(), &shiny );

	// Apply some transformations to the unit square.
	double factor1[3] = { 0.5, 0.5, 0.5 };
	double factor2[3] = { 10.0, 10.0, 10.0 };
	double factor3[3] = { 0.4, 0.4, 0.4 };
	double cylinder_scale[3] = { 1.0, 2.0, 1.0 };

	raytracer.translate(sphere, Vector3D(0, 0, -3));	
	raytracer.translate(sphere2, Vector3D(-2, 0.4, -5));	
	raytracer.scale(sphere2, Point3D(0, 0, 0), factor1);
	raytracer.translate(sphere3, Vector3D(-1, -1, -3.5));	
	raytracer.scale(sphere3, Point3D(0, 0, 0), factor3);

	raytracer.translate(plane, Vector3D(0, 0, -10));	
	raytracer.scale(plane, Point3D(0, 0, 0), factor2);
 
	raytracer.translate(plane2, Vector3D(0, -5, -5));	
	raytracer.rotate(plane2, 'x', -90); 
	raytracer.scale(plane2, Point3D(0, 0, 0), factor2);
	raytracer.translate(plane3, Vector3D(-5, 0, -5));	
	raytracer.rotate(plane3, 'y', 90); 
	raytracer.scale(plane3, Point3D(0, 0, 0), factor2);

}

void dofDemo(Raytracer& raytracer, Material& glass, Material& jade, Material& gold, Material& weird, Material& shiny ){
	// Defines a point light source.
	raytracer.setAmbientLight(Colour(0.9, 0.9, 0.9));
	raytracer.addLightSource( new PointLight(Point3D(0, 0, 5), 
				Colour(0.9, 0.9, 0.9)) );
	raytracer.addLightSource( new PointLight(Point3D(0, 5, -5), 
				Colour(0.4, 0.4, 0.4)) );


	// Add a unit square into the scene with material mat.
	SceneDagNode* sphere = raytracer.addObject( new UnitSphere(), &gold );
	SceneDagNode* sphere2 = raytracer.addObject( new UnitSphere(), &gold );
	SceneDagNode* sphere3 = raytracer.addObject( new UnitSphere(), &gold );
	SceneDagNode* sphere4 = raytracer.addObject( new UnitSphere(), &gold );
	SceneDagNode* sphere5 = raytracer.addObject( new UnitSphere(), &gold );

	// Apply some transformations to the unit square.
	raytracer.translate(sphere , Vector3D(-3  , -1  , -5));	
	raytracer.translate(sphere2, Vector3D(-1.5, -0.5, -7));	
	raytracer.translate(sphere3, Vector3D( 0  , -0  , -9));	
	raytracer.translate(sphere4, Vector3D( 1.5,  0.5, -11));	
	raytracer.translate(sphere5, Vector3D( 3  ,  1  , -13));	

}

