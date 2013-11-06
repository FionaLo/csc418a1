/***********************************************************
             CSC418, FALL 2009
 
                 penguin.cpp
                 author: Mike Pratscher
                 based on code by: Eron Steger, J. Radulovich

		Main source file for assignment 2
		Uses OpenGL, GLUT and GLUI libraries
  
    Instructions:
        Please read the assignment page to determine 
        exactly what needs to be implemented.  Then read 
        over this file and become acquainted with its 
        design. In particular, see lines marked 'README'.
		
		Be sure to also look over keyframe.h and vector.h.
		While no changes are necessary to these files, looking
		them over will allow you to better understand their
		functionality and capabilites.

        Add source code where it appears appropriate. In
        particular, see lines marked 'TODO'.

        You should not need to change the overall structure
        of the program. However it should be clear what
        your changes do, and you should use sufficient comments
        to explain your code.  While the point of the assignment
        is to draw and animate the character, you will
        also be marked based on your design.

***********************************************************/

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glui.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <map>

#include "keyframe.h"
#include "timer.h"
#include "vector.h"

// define some colors
#define GREY 0.5, 0.5, 0.5
#define ORANGE 1.0, 0.5, 0.0
#define BLUE 0.0, 0.0, 1.0
#define RED 1.0, 0.0, 0.0
#define BLACK 0.0, 0.0, 0.0
#define WHITE 1.0, 1.0, 1.0

// *************** GLOBAL VARIABLES *************************


const float PI = 3.14159;

const float SPINNER_SPEED = 0.1;

// --------------- USER INTERFACE VARIABLES -----------------

// Window settings
int windowID;				// Glut window ID (for display)
int Win[2];					// window (x,y) size

GLUI* glui_joints;			// Glui window with joint controls
GLUI* glui_keyframe;		// Glui window with keyframe controls
GLUI* glui_render;			// Glui window for render style

char msg[256];				// String used for status message
GLUI_StaticText* status;	// Status message ("Status: <msg>")


// ---------------- ANIMATION VARIABLES ---------------------

// Camera settings
bool updateCamZPos = false;
int  lastX = 0;
int  lastY = 0;
const float ZOOM_SCALE = 0.01;

GLdouble camXPos =  0.0;
GLdouble camYPos =  0.0;
GLdouble camZPos = -400;

const GLdouble CAMERA_FOVY = 60.0;
const GLdouble NEAR_CLIP   = 0.1;
const GLdouble FAR_CLIP    = 1000.0;

// Render settings
enum { WIREFRAME, SOLID, OUTLINED, MATTE, METALLIC };	// README: the different render styles
int renderStyle = WIREFRAME;			// README: the selected render style
bool outlining = false; // am I currently outlining (used to keep the lines black when drawing the outline, rather than changing color)
GLfloat light_color[4] = { 1, 1, 1, 0 }; // white light
int colored_materials = 0; // enable color when lighting is on

// Animation settings
int animate_mode = 0;			// 0 = no anim, 1 = animate

// Keyframe settings
const char filenameKF[] = "keyframes.txt";	// file for loading / saving keyframes

Keyframe* keyframes;			// list of keyframes

int maxValidKeyframe   = 0;		// index of max VALID keyframe (in keyframe list)
const int KEYFRAME_MIN = 0;
const int KEYFRAME_MAX = 32;	// README: specifies the max number of keyframes

// Frame settings
char filenameF[128];			// storage for frame filename

int frameNumber = 0;			// current frame being dumped
int frameToFile = 0;			// flag for dumping frames to file

const float DUMP_FRAME_PER_SEC = 24.0;		// frame rate for dumped frames
const float DUMP_SEC_PER_FRAME = 1.0 / DUMP_FRAME_PER_SEC;

// Time settings
Timer* animationTimer;
Timer* frameRateTimer;

const float TIME_MIN = 0.0;
const float TIME_MAX = 10.0;	// README: specifies the max time of the animation
const float SEC_PER_FRAME = 1.0 / 60.0;

// Joint settings

// README: This is the key data structure for
// updating keyframes in the keyframe list and
// for driving the animation.
//   i) When updating a keyframe, use the values
//      in this data structure to update the
//      appropriate keyframe in the keyframe list.
//  ii) When calculating the interpolated pose,
//      the resulting pose vector is placed into
//      this data structure. (This code is already
//      in place - see the animate() function)
// iii) When drawing the scene, use the values in
//      this data structure (which are set in the
//      animate() function as described above) to
//      specify the appropriate transformations.
Keyframe* joint_ui_data;

// README: To change the range of a particular DOF,
// simply change the appropriate min/max values below
// TODO: think seriously
const float ROOT_TRANSLATE_X_MIN =  -200;
const float ROOT_TRANSLATE_X_MAX =  200;
const float ROOT_TRANSLATE_Y_MIN = -100;
const float ROOT_TRANSLATE_Y_MAX =  100;
const float ROOT_TRANSLATE_Z_MIN = -200;
const float ROOT_TRANSLATE_Z_MAX =  200;
const float ROOT_ROTATE_X_MIN    = -180.0;
const float ROOT_ROTATE_X_MAX    =  180.0;
const float ROOT_ROTATE_Y_MIN    = -180.0;
const float ROOT_ROTATE_Y_MAX    =  180.0;
const float ROOT_ROTATE_Z_MIN    = -180.0;
const float ROOT_ROTATE_Z_MAX    =  180.0;
const float HEAD_MIN             = -30.0;
const float HEAD_MAX             =  30.0;
const float SHOULDER_PITCH_MIN   = -45.0;
const float SHOULDER_PITCH_MAX   =  45.0;
const float SHOULDER_YAW_MIN     = -45.0; // TODO: verify
const float SHOULDER_YAW_MAX     =  45.0;
const float SHOULDER_ROLL_MIN    =  0;
const float SHOULDER_ROLL_MAX    =  45.0;
const float HIP_PITCH_MIN        = -30.0; 
const float HIP_PITCH_MAX        =  30.0;
const float HIP_YAW_MIN          = -45.0; // TODO: lower so feet don't collide
const float HIP_YAW_MAX          =  45.0;
const float HIP_ROLL_MIN         = -45.0; // TODO: the legs can go into each other, is that a problem?
const float HIP_ROLL_MAX         =  45.0;
const float BEAK_MIN             =  0.0;
const float BEAK_MAX             =  0.7;
const float ELBOW_MIN            =  0.0; // TODO: think
const float ELBOW_MAX            = 75.0;
const float KNEE_MIN             =  -45.0;
const float KNEE_MAX             = 45.0;


/* Penguin Body Globals */
// note: depth and width are as the penguin is pictured in the a2 handout
const float TORSO_HEIGHT = 100;
const float TORSO_UPPER_DEPTH = 0.3 * TORSO_HEIGHT;
const float TORSO_LOWER_DEPTH = 1.5 * TORSO_UPPER_DEPTH;
const float TORSO_UPPER_WIDTH = 0.4 * TORSO_HEIGHT;
const float TORSO_LOWER_WIDTH = 2 * TORSO_UPPER_WIDTH;
const float TORSO_AVG_DEPTH = ((TORSO_LOWER_DEPTH + TORSO_LOWER_DEPTH) / 2);

const float HEAD_HEIGHT = 0.5 * TORSO_HEIGHT;
const float HEAD_LOWER_WIDTH = 1.3 * TORSO_UPPER_WIDTH;
const float HEAD_UPPER_WIDTH = 0.8 * HEAD_LOWER_WIDTH;
const float HEAD_LOWER_DEPTH = 1.2 * TORSO_UPPER_DEPTH;
const float HEAD_UPPER_DEPTH = 0.8 * HEAD_LOWER_DEPTH;

const float BEAK_HEIGHT = 0.2 * HEAD_HEIGHT;
const float BEAK_LOWER_WIDTH = 0.5 * HEAD_LOWER_WIDTH;
const float BEAK_UPPER_WIDTH = 0.8 * BEAK_LOWER_WIDTH;
const float BEAK_LOWER_DEPTH = 0.3 * HEAD_LOWER_DEPTH;
const float BEAK_UPPER_DEPTH = 0.8 * BEAK_LOWER_DEPTH;

const float LEG_LENGTH = TORSO_HEIGHT * 0.15;
const float LEG_WIDTH = TORSO_LOWER_DEPTH * 0.1;

const float FOOT_THICKNESS = TORSO_HEIGHT * 0.05;
const float FOOT_HEIGHT = TORSO_LOWER_DEPTH * 0.6; // 2d triangle height
const float FOOT_WIDTH = TORSO_LOWER_WIDTH * 0.25;

const float FIN_HEIGHT = TORSO_HEIGHT / 2;
const float FIN_UPPER_DEPTH = TORSO_LOWER_DEPTH * 0.2;
const float FIN_LOWER_DEPTH = TORSO_UPPER_DEPTH * 0.2;
const float FIN_LOWER_WIDTH = TORSO_UPPER_WIDTH / 3;
const float FIN_UPPER_WIDTH = TORSO_LOWER_WIDTH / 3;
const float FIN_AVG_DEPTH = (FIN_LOWER_DEPTH + FIN_UPPER_DEPTH) / 2;

const float LOWER_FIN_WIDTH = FIN_LOWER_WIDTH;
const float LOWER_FIN_HEIGHT = FIN_HEIGHT * 0.3;
const float LOWER_FIN_DEPTH = FIN_LOWER_DEPTH;

// an estimate of the penguin's size
const float PENGUIN_HEIGHT = TORSO_HEIGHT + LEG_LENGTH + HEAD_HEIGHT;


// macros
#define COLOUR_CHANGE(COLOR) if (!outlining) { glColor3f(COLOR); }
#define DEG2RAD(val) (val / (180 / PI))
#define RAD2DEG(val) (val * (180 / PI))


// ***********  FUNCTION HEADER DECLARATIONS ****************


// Initialization functions
void initDS();
void initGlut(int argc, char** argv);
void initGlui();
void initGl();


// Callbacks for handling events in glut
void reshape(int w, int h);
void animate();
void display(void);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);


// Functions to help draw the object
Vector getInterpolatedJointDOFS(float time);
void drawCube();
void drawTrapazoid(const float height, const float upper_width, const float lower_width, const float upper_depth, const float lower_depth);
void drawTriangle(const float thickness, const float width, const float height);
void drawPenguin();
void drawLeg(bool front);
void drawFin(bool front);
void jointAt(const float x, const float y, const float z, const std::map<char, float> rotations);

// Image functions
void writeFrame(char* filename, bool pgm, bool frontBuffer);


// ******************** FUNCTIONS ************************



// main() function
// Initializes the user interface (and any user variables)
// then hands over control to the event handler, which calls 
// display() whenever the GL window needs to be redrawn.
int main(int argc, char** argv)
{

    // Process program arguments
    if(argc != 3) {
        printf("Usage: demo [width] [height]\n");
        printf("Using 640x480 window by default...\n");
        Win[0] = 640;
        Win[1] = 480;
    } else {
        Win[0] = atoi(argv[1]);
        Win[1] = atoi(argv[2]);
    }


    // Initialize data structs, glut, glui, and opengl
	initDS();
    initGlut(argc, argv);
    initGlui();
    initGl();

    // Invoke the standard GLUT main event loop
    glutMainLoop();

    return 0;         // never reached
}


// Create / initialize global data structures
void initDS()
{
	keyframes = new Keyframe[KEYFRAME_MAX];
	for( int i = 0; i < KEYFRAME_MAX; i++ )
		keyframes[i].setID(i);

	animationTimer = new Timer();
	frameRateTimer = new Timer();
	joint_ui_data  = new Keyframe();
}


// Initialize glut and create a window with the specified caption 
void initGlut(int argc, char** argv)
{
	// Init GLUT
	glutInit(&argc, argv);

    // Set video mode: double-buffered, color, depth-buffered
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Create window
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Win[0],Win[1]);
    windowID = glutCreateWindow(argv[0]);

    // Setup callback functions to handle events
    glutReshapeFunc(reshape);	// Call reshape whenever window resized
    glutDisplayFunc(display);	// Call display whenever new frame needed
	glutMouseFunc(mouse);		// Call mouse whenever mouse button pressed
	glutMotionFunc(motion);		// Call motion whenever mouse moves while button pressed
}


// Load Keyframe button handler. Called when the "load keyframe" button is pressed
void loadKeyframeButton(int)
{
	// Get the keyframe ID from the UI
	int keyframeID = joint_ui_data->getID();

	// Update the 'joint_ui_data' variable with the appropriate
	// entry from the 'keyframes' array (the list of keyframes)
	*joint_ui_data = keyframes[keyframeID];

	// Sync the UI with the 'joint_ui_data' values
	glui_joints->sync_live();
	glui_keyframe->sync_live();

	// Let the user know the values have been loaded
	sprintf(msg, "Status: Keyframe %d loaded successfully", keyframeID);
	status->set_text(msg);
}

// Update Keyframe button handler. Called when the "update keyframe" button is pressed
void updateKeyframeButton(int)
{
	///////////////////////////////////////////////////////////
	// TODO:
	//   Modify this function to save the UI joint values into
	//   the appropriate keyframe entry in the keyframe list
	//   when the user clicks on the 'Update Keyframe' button.
	//   Refer to the 'loadKeyframeButton' function for help.
	///////////////////////////////////////////////////////////

	// Get the keyframe ID from the UI
	int keyframeID = joint_ui_data->getID();

	// Update the 'maxValidKeyframe' index variable
	// (it will be needed when doing the interpolation)
	maxValidKeyframe = std::max(maxValidKeyframe, keyframeID);

	// Update the appropriate entry in the 'keyframes' array
	// with the 'joint_ui_data' data
	keyframes[keyframeID] = *joint_ui_data;

	// Let the user know the values have been updated
	sprintf(msg, "Status: Keyframe %d updated successfully", keyframeID);
	status->set_text(msg);
}

// Load Keyframes From File button handler. Called when the "load keyframes from file" button is pressed
//
// ASSUMES THAT THE FILE FORMAT IS CORRECT, ie, there is no error checking!
//
void loadKeyframesFromFileButton(int)
{
	// Open file for reading
	FILE* file = fopen(filenameKF, "r");
	if( file == NULL )
	{
		sprintf(msg, "Status: Failed to open file %s", filenameKF);
		status->set_text(msg);
		return;
	}

	// Read in maxValidKeyframe first
	fscanf(file, "%d", &maxValidKeyframe);

	// Now read in all keyframes in the format:
	//    id
	//    time
	//    DOFs
	//
	for( int i = 0; i <= maxValidKeyframe; i++ )
	{
		fscanf(file, "%d", keyframes[i].getIDPtr());
		fscanf(file, "%f", keyframes[i].getTimePtr());

		for( int j = 0; j < Keyframe::NUM_JOINT_ENUM; j++ )
			fscanf(file, "%f", keyframes[i].getDOFPtr(j));
	}

	// Close file
	fclose(file);

	// Let the user know the keyframes have been loaded
	sprintf(msg, "Status: Keyframes loaded successfully");
	status->set_text(msg);
}

// Save Keyframes To File button handler. Called when the "save keyframes to file" button is pressed
void saveKeyframesToFileButton(int)
{
	// Open file for writing
	FILE* file = fopen(filenameKF, "w");
	if( file == NULL )
	{
		sprintf(msg, "Status: Failed to open file %s", filenameKF);
		status->set_text(msg);
		return;
	}

	// Write out maxValidKeyframe first
	fprintf(file, "%d\n", maxValidKeyframe);
	fprintf(file, "\n");

	// Now write out all keyframes in the format:
	//    id
	//    time
	//    DOFs
	//
	for( int i = 0; i <= maxValidKeyframe; i++ )
	{
		fprintf(file, "%d\n", keyframes[i].getID());
		fprintf(file, "%f\n", keyframes[i].getTime());

		for( int j = 0; j < Keyframe::NUM_JOINT_ENUM; j++ )
			fprintf(file, "%f\n", keyframes[i].getDOF(j));

		fprintf(file, "\n");
	}

	// Close file
	fclose(file);

	// Let the user know the keyframes have been saved
	sprintf(msg, "Status: Keyframes saved successfully");
	status->set_text(msg);
}

// Animate button handler.  Called when the "animate" button is pressed.
void animateButton(int)
{
  // synchronize variables that GLUT uses
  glui_keyframe->sync_live();

  // toggle animation mode and set idle function appropriately
  if( animate_mode == 0 )
  {
	// start animation
	frameRateTimer->reset();
	animationTimer->reset();

	animate_mode = 1;
	GLUI_Master.set_glutIdleFunc(animate);

	// Let the user know the animation is running
	sprintf(msg, "Status: Animating...");
	status->set_text(msg);
  }
  else
  {
	// stop animation
	animate_mode = 0;
	GLUI_Master.set_glutIdleFunc(NULL);

	// Let the user know the animation has stopped
	sprintf(msg, "Status: Animation stopped");
	status->set_text(msg);
  }
}

// Render Frames To File button handler. Called when the "Render Frames To File" button is pressed.
void renderFramesToFileButton(int)
{
	// Calculate number of frames to generate based on dump frame rate
	int numFrames = int(keyframes[maxValidKeyframe].getTime() * DUMP_FRAME_PER_SEC) + 1;

	// Generate frames and save to file
	frameToFile = 1;
	for( frameNumber = 0; frameNumber < numFrames; frameNumber++ )
	{
		// Get the interpolated joint DOFs
		joint_ui_data->setDOFVector( getInterpolatedJointDOFS(frameNumber * DUMP_SEC_PER_FRAME) );

		// Let the user know which frame is being rendered
		sprintf(msg, "Status: Rendering frame %d...", frameNumber);
		status->set_text(msg);

		// Render the frame
		display();
	}
	frameToFile = 0;

	// Let the user know how many frames were generated
	sprintf(msg, "Status: %d frame(s) rendered to file", numFrames);
	status->set_text(msg);
}

// Quit button handler.  Called when the "quit" button is pressed.
void quitButton(int)
{
  exit(0);
}

// Initialize GLUI and the user interface
void initGlui()
{
	GLUI_Panel* glui_panel;
	GLUI_Spinner* glui_spinner;
	GLUI_RadioGroup* glui_radio_group;

    GLUI_Master.set_glutIdleFunc(NULL);


	// Create GLUI window (joint controls) ***************
	//
	glui_joints = GLUI_Master.create_glui("Joint Control", 0, Win[0]+12, 0);

    // Create controls to specify root position and orientation
	glui_panel = glui_joints->add_panel("Root");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_TRANSLATE_X));
	glui_spinner->set_float_limits(ROOT_TRANSLATE_X_MIN, ROOT_TRANSLATE_X_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_TRANSLATE_Y));
	glui_spinner->set_float_limits(ROOT_TRANSLATE_Y_MIN, ROOT_TRANSLATE_Y_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate z:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_TRANSLATE_Z));
	glui_spinner->set_float_limits(ROOT_TRANSLATE_Z_MIN, ROOT_TRANSLATE_Z_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_ROTATE_X));
	glui_spinner->set_float_limits(ROOT_ROTATE_X_MIN, ROOT_ROTATE_X_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_ROTATE_Y));
	glui_spinner->set_float_limits(ROOT_ROTATE_Y_MIN, ROOT_ROTATE_Y_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate z:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_ROTATE_Z));
	glui_spinner->set_float_limits(ROOT_ROTATE_Z_MIN, ROOT_ROTATE_Z_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify head rotation
	glui_panel = glui_joints->add_panel("Head");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "head:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::HEAD));
	glui_spinner->set_float_limits(HEAD_MIN, HEAD_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify beak
	glui_panel = glui_joints->add_panel("Beak");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "beak:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::BEAK));
	glui_spinner->set_float_limits(BEAK_MIN, BEAK_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);


	glui_joints->add_column(false);


	// Create controls to specify right arm
	glui_panel = glui_joints->add_panel("Right arm");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_SHOULDER_PITCH));
	glui_spinner->set_float_limits(SHOULDER_PITCH_MIN, SHOULDER_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_SHOULDER_YAW));
	glui_spinner->set_float_limits(SHOULDER_YAW_MIN, SHOULDER_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_SHOULDER_ROLL));
	glui_spinner->set_float_limits(SHOULDER_ROLL_MIN, SHOULDER_ROLL_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "elbow:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_ELBOW));
	glui_spinner->set_float_limits(ELBOW_MIN, ELBOW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify left arm
	glui_panel = glui_joints->add_panel("Left arm");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_SHOULDER_PITCH));
	glui_spinner->set_float_limits(SHOULDER_PITCH_MIN, SHOULDER_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_SHOULDER_YAW));
	glui_spinner->set_float_limits(SHOULDER_YAW_MIN, SHOULDER_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_SHOULDER_ROLL));
	glui_spinner->set_float_limits(SHOULDER_ROLL_MIN, SHOULDER_ROLL_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "elbow:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_ELBOW));
	glui_spinner->set_float_limits(ELBOW_MIN, ELBOW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);


	glui_joints->add_column(false);


	// Create controls to specify right leg
	glui_panel = glui_joints->add_panel("Right leg");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_HIP_PITCH));
	glui_spinner->set_float_limits(HIP_PITCH_MIN, HIP_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_HIP_YAW));
	glui_spinner->set_float_limits(HIP_YAW_MIN, HIP_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_HIP_ROLL));
	glui_spinner->set_float_limits(HIP_ROLL_MIN, HIP_ROLL_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "knee:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_KNEE));
	glui_spinner->set_float_limits(KNEE_MIN, KNEE_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify left leg
	glui_panel = glui_joints->add_panel("Left leg");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_HIP_PITCH));
	glui_spinner->set_float_limits(HIP_PITCH_MIN, HIP_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_HIP_YAW));
	glui_spinner->set_float_limits(HIP_YAW_MIN, HIP_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_HIP_ROLL));
	glui_spinner->set_float_limits(HIP_ROLL_MIN, HIP_ROLL_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "knee:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_KNEE));
	glui_spinner->set_float_limits(KNEE_MIN, KNEE_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);


	///////////////////////////////////////////////////////////
	// TODO (for controlling light source position & additional
	//      rendering styles):
	//   Add more UI spinner elements here. Be sure to also
	//   add the appropriate min/max range values to this
	//   file, and to also add the appropriate enums to the
	//   enumeration in the Keyframe class (keyframe.h).
	///////////////////////////////////////////////////////////

	//
	// ***************************************************

	// TODO: the keyframe enums, you have to add this
	// Create GLUI window (keyframe controls) ************
	//
	glui_keyframe = GLUI_Master.create_glui("Keyframe Control", 0, 0, Win[1]+64);

	// Create a control to specify the time (for setting a keyframe)
	glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
	glui_spinner = glui_keyframe->add_spinner_to_panel(glui_panel, "Time:", GLUI_SPINNER_FLOAT, joint_ui_data->getTimePtr());
	glui_spinner->set_float_limits(TIME_MIN, TIME_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create a control to specify a keyframe (for updating / loading a keyframe)
	glui_keyframe->add_column_to_panel(glui_panel, false);
	glui_spinner = glui_keyframe->add_spinner_to_panel(glui_panel, "Keyframe ID:", GLUI_SPINNER_INT, joint_ui_data->getIDPtr());
	glui_spinner->set_int_limits(KEYFRAME_MIN, KEYFRAME_MAX-1, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_keyframe->add_separator();

	// Add buttons to load and update keyframes
	// Add buttons to load and save keyframes from a file
	// Add buttons to start / stop animation and to render frames to file
	glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
	glui_keyframe->add_button_to_panel(glui_panel, "Load Keyframe", 0, loadKeyframeButton);
	glui_keyframe->add_button_to_panel(glui_panel, "Load Keyframes From File", 0, loadKeyframesFromFileButton);
	glui_keyframe->add_button_to_panel(glui_panel, "Start / Stop Animation", 0, animateButton);
	glui_keyframe->add_column_to_panel(glui_panel, false);
	glui_keyframe->add_button_to_panel(glui_panel, "Update Keyframe", 0, updateKeyframeButton);
	glui_keyframe->add_button_to_panel(glui_panel, "Save Keyframes To File", 0, saveKeyframesToFileButton);
	glui_keyframe->add_button_to_panel(glui_panel, "Render Frames To File", 0, renderFramesToFileButton);

	glui_keyframe->add_separator();

	// Add status line
	glui_panel = glui_keyframe->add_panel("");
	status = glui_keyframe->add_statictext_to_panel(glui_panel, "Status: Ready");

	// Add button to quit
	glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
	glui_keyframe->add_button_to_panel(glui_panel, "Quit", 0, quitButton);
	//
	// ***************************************************


	// Create GLUI window (render controls) ************
	//
	glui_render = GLUI_Master.create_glui("Render Control", 0, 367, Win[1]+64);

	// Create control to specify the render style
	glui_panel = glui_render->add_panel("Render Style");
	glui_radio_group = glui_render->add_radiogroup_to_panel(glui_panel, &renderStyle);
	glui_render->add_radiobutton_to_group(glui_radio_group, "Wireframe");
	glui_render->add_radiobutton_to_group(glui_radio_group, "Solid");
	glui_render->add_radiobutton_to_group(glui_radio_group, "Solid w/ outlines");
	glui_render->add_radiobutton_to_group(glui_radio_group, "Matte");
	glui_render->add_radiobutton_to_group(glui_radio_group, "Metallic");

	// Create control to specify lighting info
	glui_panel = glui_render->add_panel("Lighting");
	glui_render->add_checkbox_to_panel(glui_panel, "Colored Materials", &colored_materials);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "Light Angle XY Plane", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::LIGHT_ANGLE_XY));
	glui_spinner->set_float_limits(-360, 360, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);


	//
	// ***************************************************


	// Tell GLUI windows which window is main graphics window
	glui_joints->set_main_gfx_window(windowID);
	glui_keyframe->set_main_gfx_window(windowID);
	glui_render->set_main_gfx_window(windowID);
}


// Performs most of the OpenGL intialization
void initGl(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);
}


// Calculates the interpolated joint DOF vector
// using Catmull-Rom interpolation of the keyframes
Vector getInterpolatedJointDOFS(float time)
{
	// Need to find the keyframes bewteen which
	// the supplied time lies.
	// At the end of the loop we have:
	//    keyframes[i-1].getTime() < time <= keyframes[i].getTime()
	//
	int i = 0;
	while( i <= maxValidKeyframe && keyframes[i].getTime() < time )
		i++;

	// If time is before or at first defined keyframe, then
	// just use first keyframe pose
	if( i == 0 )
		return keyframes[0].getDOFVector();

	// If time is beyond last defined keyframe, then just
	// use last keyframe pose
	if( i > maxValidKeyframe )
		return keyframes[maxValidKeyframe].getDOFVector();

	// Need to normalize time to (0, 1]
	time = (time - keyframes[i-1].getTime()) / (keyframes[i].getTime() - keyframes[i-1].getTime());

	// Get appropriate data points and tangent vectors
	// for computing the interpolation
	Vector p0 = keyframes[i-1].getDOFVector();
	Vector p1 = keyframes[i].getDOFVector();

	Vector t0, t1;
	if( i == 1 )							// special case - at beginning of spline
	{
		t0 = keyframes[i].getDOFVector() - keyframes[i-1].getDOFVector();
		t1 = (keyframes[i+1].getDOFVector() - keyframes[i-1].getDOFVector()) * 0.5;
	}
	else if( i == maxValidKeyframe )		// special case - at end of spline
	{
		t0 = (keyframes[i].getDOFVector() - keyframes[i-2].getDOFVector()) * 0.5;
		t1 = keyframes[i].getDOFVector() - keyframes[i-1].getDOFVector();
	}
	else
	{
		t0 = (keyframes[i].getDOFVector()   - keyframes[i-2].getDOFVector()) * 0.5;
		t1 = (keyframes[i+1].getDOFVector() - keyframes[i-1].getDOFVector()) * 0.5;
	}

	// Return the interpolated Vector
	Vector a0 = p0;
	Vector a1 = t0;
	Vector a2 = p0 * (-3) + p1 * 3 + t0 * (-2) + t1 * (-1);
	Vector a3 = p0 * 2 + p1 * (-2) + t0 + t1;

	return (((a3 * time + a2) * time + a1) * time + a0);
}


// Callback idle function for animating the scene
void animate()
{
	// Only update if enough time has passed
	// (This locks the display to a certain frame rate rather
	//  than updating as fast as possible. The effect is that
	//  the animation should run at about the same rate
	//  whether being run on a fast machine or slow machine)
	if( frameRateTimer->elapsed() > SEC_PER_FRAME )
	{
		// Tell glut window to update itself. This will cause the display()
		// callback to be called, which renders the object (once you've written
		// the callback).
		glutSetWindow(windowID);
		glutPostRedisplay();

		// Restart the frame rate timer
		// for the next frame
		frameRateTimer->reset();
	}
}


// Handles the window being resized by updating the viewport
// and projection matrices
void reshape(int w, int h)
{
	// Update internal variables and OpenGL viewport
	Win[0] = w;
	Win[1] = h;
	glViewport(0, 0, (GLsizei)Win[0], (GLsizei)Win[1]);

    // Setup projection matrix for new window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(CAMERA_FOVY, (GLdouble)Win[0]/(GLdouble)Win[1], NEAR_CLIP, FAR_CLIP);
}



// display callback
//
// README: This gets called by the event handler
// to draw the scene, so this is where you need
// to build your scene -- make your changes and
// additions here. All rendering happens in this
// function. For Assignment 2, updates to the
// joint DOFs (joint_ui_data) happen in the
// animate() function.
void display(void)
{
    // Clear the screen with the background colour
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

    // Setup the model-view transformation matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	// Specify camera transformation
	glTranslatef(camXPos, camYPos, camZPos);


	// Get the time for the current animation step, if necessary
	if( animate_mode )
	{
		float curTime = animationTimer->elapsed();

		if( curTime >= keyframes[maxValidKeyframe].getTime() )
		{
			// Restart the animation
			animationTimer->reset();
			curTime = animationTimer->elapsed();
		}

		///////////////////////////////////////////////////////////
		// README:
		//   This statement loads the interpolated joint DOF vector
		//   into the global 'joint_ui_data' variable. Use the
		//   'joint_ui_data' variable below in your model code to
		//   drive the model for animation.
		///////////////////////////////////////////////////////////
		// Get the interpolated joint DOFs
		joint_ui_data->setDOFVector( getInterpolatedJointDOFS(curTime) );

		// Update user interface
		joint_ui_data->setTime(curTime);
		glui_keyframe->sync_live();
	}


    ///////////////////////////////////////////////////////////
    // TODO:
	//   Modify this function to draw the scene.
	//   This should include function calls that apply
	//   the appropriate transformation matrices and render
	//   the individual body parts.
	//   Use the 'joint_ui_data' data structure to obtain
	//   the joint DOFs to specify your transformations.
	//   Sample code is provided below and demonstrates how
	//   to access the joint DOF values. This sample code
	//   should be replaced with your own.
	//   Use the 'renderStyle' variable and the associated
	//   enumeration to determine how the geometry should be
	//   rendered.
    ///////////////////////////////////////////////////////////



	// SAMPLE CODE **********

	// determine render style and set glPolygonMode appropriately
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
	if (renderStyle == MATTE || renderStyle == METALLIC) {

		if (colored_materials) {
			glEnable(GL_COLOR_MATERIAL);
		} else {
			glDisable(GL_COLOR_MATERIAL);
		}

		glEnable(GL_LIGHTING);
		float radius = PENGUIN_HEIGHT * 1.5;
		float light_angle_xy = joint_ui_data->getDOF(Keyframe::LIGHT_ANGLE_XY);
		float light_x = radius * cos(DEG2RAD(light_angle_xy));
		float light_y = radius * sin(DEG2RAD(light_angle_xy));
		GLfloat light_pos[] = { light_x, light_y, 0, 0 }; // directional light
		// mark the light's angle
		glPushMatrix();
			glTranslatef(light_x, light_y, 0);
			glutSolidSphere(5, 20, 20);
		glPopMatrix();

	    const GLfloat ambient[4] = { 0.1, 0.1, 0.1, 1.0 };
	    const GLfloat specular[4] = { 1.0, 1.0, 1.0, 1.0 };
	    const GLfloat diffuse[4] = { 1.0, 1.0, 1.0, 1.0};
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient );
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse );
		glLightfv(GL_LIGHT0, GL_SPECULAR, specular );
		glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
		glEnable(GL_LIGHT0);	
	} else {
		glDisable(GL_LIGHTING);
	}

	if (renderStyle == WIREFRAME) {
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		drawPenguin();
	} else if (renderStyle == SOLID) { 
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		drawPenguin();
	} else if (renderStyle == OUTLINED) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		drawPenguin();

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(1, 5);
		glColor3f(BLACK);
		glLineWidth(5);
		outlining = true;
		drawPenguin();
		outlining = false;
		glDisable(GL_POLYGON_OFFSET_LINE);
		glLineWidth(1);
	} else if (renderStyle == METALLIC) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		// values for chrome (http://devernay.free.fr/cours/opengl/materials.html)
		glMaterialf( GL_FRONT, GL_SHININESS, 0.6 * 128 );
		const GLfloat ambient[4] = {0.25, 0.25, 0.25, 1.0 };
	    const GLfloat specular[4] = { 0.774597, 0.774597, 0.774597, 1.0 };
	    const GLfloat diffuse[4] = { 0.4, 0.4, 0.4, 1.0 };
		glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
		glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
		drawPenguin();
	} else if (renderStyle == MATTE) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glMaterialf( GL_FRONT, GL_SHININESS, 0 );
		const GLfloat ambient[4] = {0.25, 0.25, 0.25, 1.0 };
	    const GLfloat specular[4] = { 0.1, 0.1, 0.1, 1.0 };
	    const GLfloat diffuse[4] = { 0.70, 0.70, 0.70, 1.0 };
	    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
		glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
		drawPenguin();
	}

	// SAMPLE CODE **********

    // Execute any GL functions that are in the queue just to be safe
    glFlush();

	// Dump frame to file, if requested
	if( frameToFile )
	{
		sprintf(filenameF, "frame%03d.ppm", frameNumber);
		writeFrame(filenameF, false, false);
	}

    // Now, show the frame buffer that we just drew into.
    // (this prevents flickering).
    glutSwapBuffers();
}

void drawPenguin() {
	COLOUR_CHANGE(WHITE)
	glPushMatrix();

		// Root transfomrations
		glTranslatef(joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_X),
					 joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Y),
					 joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Z));

		// note that since rotations are not commutative, this order matters!
		glRotatef(joint_ui_data->getDOF(Keyframe::ROOT_ROTATE_X), 1, 0, 0);
		glRotatef(joint_ui_data->getDOF(Keyframe::ROOT_ROTATE_Y), 0, 1, 0);
		glRotatef(joint_ui_data->getDOF(Keyframe::ROOT_ROTATE_Z), 0, 0, 1);

		COLOUR_CHANGE(BLUE)
		// draw torso
		drawTrapazoid(TORSO_HEIGHT, TORSO_UPPER_WIDTH, TORSO_LOWER_WIDTH, TORSO_UPPER_DEPTH, TORSO_LOWER_DEPTH);
		COLOUR_CHANGE(WHITE)

		// draw fins
		drawFin(true);
		drawFin(false);

		// upper body - head and beak
		glPushMatrix();
			glTranslatef(0, TORSO_HEIGHT / 2 + HEAD_HEIGHT / 2, 0); // move head up
			glRotatef(joint_ui_data->getDOF(Keyframe::HEAD), 0, 1, 0); // rotate head
			// draw head
			COLOUR_CHANGE(GREY)
			drawTrapazoid(HEAD_HEIGHT, HEAD_UPPER_WIDTH, HEAD_LOWER_WIDTH, HEAD_UPPER_DEPTH, HEAD_LOWER_DEPTH);
			// draw beak - 2 parts (upper + lower)
			COLOUR_CHANGE(ORANGE)
			glPushMatrix();
				// get how much the beak should be open
				const float beak_slider_trans = (joint_ui_data->getDOF(Keyframe::BEAK) * BEAK_HEIGHT) / 2;
				glTranslatef(- HEAD_LOWER_WIDTH / 2, BEAK_HEIGHT / 2, 0); // move beak up to head
				glPushMatrix();
					glTranslatef(0, beak_slider_trans, 0); 
					drawTrapazoid(BEAK_HEIGHT, BEAK_UPPER_WIDTH, BEAK_LOWER_WIDTH, BEAK_UPPER_DEPTH, BEAK_LOWER_DEPTH);
				glPopMatrix();
				glPushMatrix();
					glTranslatef(0, - BEAK_HEIGHT - beak_slider_trans, 0); // move beak up to head
					glScalef(1, -1, 1);
					drawTrapazoid(BEAK_HEIGHT, BEAK_UPPER_WIDTH, BEAK_LOWER_WIDTH, BEAK_UPPER_DEPTH, BEAK_LOWER_DEPTH);
				glPopMatrix();
			glPopMatrix();
		glPopMatrix();

		// lower body
		glPushMatrix();
			glTranslatef(0, -TORSO_HEIGHT / 2 - LEG_LENGTH, 0); // move legs down
			drawLeg(true);
			drawLeg(false);
		glPopMatrix();
	glPopMatrix();
}

void drawLeg(bool left) {
	glPushMatrix();
		
		glTranslatef(0, 0, left ? TORSO_LOWER_DEPTH / 4 : -TORSO_LOWER_DEPTH / 4);

		std::map<char, float> leg_rotations;
		leg_rotations['x'] = joint_ui_data->getDOF(left ? Keyframe::L_HIP_ROLL : Keyframe::R_HIP_ROLL);
		leg_rotations['y'] = joint_ui_data->getDOF(left ? Keyframe::L_HIP_YAW : Keyframe::R_HIP_YAW);
		leg_rotations['z'] = joint_ui_data->getDOF(left ? Keyframe::L_HIP_PITCH : Keyframe::R_HIP_PITCH);
		jointAt(0, LEG_LENGTH, 0, leg_rotations);

		glPushMatrix();
			glScalef(LEG_WIDTH, LEG_LENGTH, LEG_WIDTH);
			COLOUR_CHANGE(GREY)
			drawCube();
		glPopMatrix();
		glPushMatrix();
			glTranslatef(-FOOT_HEIGHT / 2, -LEG_LENGTH / 2 - FOOT_THICKNESS, 0);
			std::map<char, float> knee_rotations;
			knee_rotations['z'] = joint_ui_data->getDOF(left ? Keyframe::L_KNEE : Keyframe::R_KNEE);
			jointAt(FOOT_HEIGHT / 2 - LEG_WIDTH, 0, 0, knee_rotations);
			glRotatef(-90, 0, 1, 0);
			COLOUR_CHANGE(ORANGE)
			drawTriangle(FOOT_THICKNESS, FOOT_WIDTH, FOOT_HEIGHT);
		glPopMatrix();
	glPopMatrix();
}

void drawFin(bool left) {
	glPushMatrix();
		glTranslatef(0, 0, left ? TORSO_AVG_DEPTH / 2 : -TORSO_AVG_DEPTH / 2);
		glRotatef(-15, 0, 0, 1);
		std::map<char, float> shoulder_rotations;
		shoulder_rotations['x'] = joint_ui_data->getDOF(left ? Keyframe::L_SHOULDER_ROLL : Keyframe::R_SHOULDER_ROLL);
		if (left) { 
			shoulder_rotations['x'] = -shoulder_rotations['x']; // prevent the arms from going inside the body
		}
		shoulder_rotations['y'] = joint_ui_data->getDOF(left ? Keyframe::L_SHOULDER_YAW : Keyframe::R_SHOULDER_YAW);
		shoulder_rotations['z'] = joint_ui_data->getDOF(left ? Keyframe::L_SHOULDER_PITCH : Keyframe::R_SHOULDER_PITCH);
		jointAt(0, FIN_HEIGHT / 2, 0, shoulder_rotations);
		float angle = atan(((TORSO_LOWER_DEPTH - TORSO_UPPER_DEPTH) / 2) / TORSO_HEIGHT) * 180 / PI;
		glRotatef(left ? -angle : angle, 1, 0, 0);
		drawTrapazoid(FIN_HEIGHT, FIN_UPPER_WIDTH, FIN_LOWER_WIDTH, FIN_UPPER_DEPTH, FIN_LOWER_DEPTH);
		glPushMatrix();
			glTranslatef(- LOWER_FIN_WIDTH / 4, -FIN_HEIGHT / 2 - LOWER_FIN_HEIGHT / 4 , left ? FIN_LOWER_DEPTH / 2 : -FIN_LOWER_DEPTH / 2);
			std::map<char, float> elbow_rotations;
			elbow_rotations['z'] = joint_ui_data->getDOF(left ? Keyframe::L_ELBOW : Keyframe::R_ELBOW);
			jointAt(0, 0, 0, elbow_rotations); // TODO: move this
			glScalef(LOWER_FIN_WIDTH / 2, LOWER_FIN_HEIGHT / 2, FIN_AVG_DEPTH / 2);
			glRotatef(60, 0, 0, 1);
			drawCube();
		glPopMatrix();
	glPopMatrix();
}

// create a joint at x, y, z offset from the current location
// specify the axes to be rotated along in a map<char, float>, example:
// { 'x' : 37, 'y': 32 } would rotate along x by 37 and along y by 32. 
// valid keys are 'x' 'y' and 'z'. Order will always be applied in xyz 
// if a key is not present, no rotation occurs along its axis
void jointAt(const float x, const float y, const float z, std::map<char, float> rotations) {
    // keep the current color so we can swap it back later
    float currentColor[4];
    glGetFloatv(GL_CURRENT_COLOR, currentColor);
    glColor3f(RED);
    glTranslatef(x, y, z); // move origin back 
	glutSolidSphere(1.0, 20.0, 20.0); // mark the joint
	glColor3f(currentColor[0], currentColor[1], currentColor[2]);
	if (rotations.count('x')) {
		glRotatef(rotations['x'], 1, 0, 0);
	}
	if (rotations.count('y')) {
		glRotatef(rotations['y'], 0, 1, 0);
	}
	if (rotations.count('z')) {
		glRotatef(rotations['z'], 0, 0, 1);
	}
    glTranslatef(-x, -y, -z); // move origin to joint
    // restore current color
}


// Handles mouse button pressed / released events
void mouse(int button, int state, int x, int y)
{
	// If the RMB is pressed and dragged then zoom in / out
	if( button == GLUT_RIGHT_BUTTON )
	{
		if( state == GLUT_DOWN )
		{
			lastX = x;
			lastY = y;
			updateCamZPos = true;
		}
		else
		{
			updateCamZPos = false;
		}
	}
}


// Handles mouse motion events while a button is pressed
void motion(int x, int y)
{
	// If the RMB is pressed and dragged then zoom in / out
	if( updateCamZPos )
	{
		// Update camera z position
		camZPos += (x - lastX) * ZOOM_SCALE;
		lastX = x;

		// Redraw the scene from updated camera position
		glutSetWindow(windowID);
		glutPostRedisplay();
	}
}


// Draw a unit cube, centered at the current location
// README: Helper code for drawing a cube
void drawCube()
{
	glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1); glVertex3f(-1.0, -1.0, 1.0);
		glNormal3f(0, 0, 1); glVertex3f( 1.0, -1.0, 1.0);
		glNormal3f(0, 0, 1); glVertex3f( 1.0,  1.0, 1.0);
		glNormal3f(0, 0, 1); glVertex3f(-1.0,  1.0, 1.0);

		// draw back face
		glNormal3f(0, 0, -1); glVertex3f( 1.0, -1.0, -1.0);
		glNormal3f(0, 0, -1); glVertex3f(-1.0, -1.0, -1.0);
		glNormal3f(0, 0, -1); glVertex3f(-1.0,  1.0, -1.0);
		glNormal3f(0, 0, -1); glVertex3f( 1.0,  1.0, -1.0);

		// draw left face
		glNormal3f(-1, 0, 0); glVertex3f(-1.0, -1.0, -1.0);
		glNormal3f(-1, 0, 0); glVertex3f(-1.0, -1.0,  1.0);
		glNormal3f(-1, 0, 0); glVertex3f(-1.0,  1.0,  1.0);
		glNormal3f(-1, 0, 0); glVertex3f(-1.0,  1.0, -1.0);

		// draw right face
		glNormal3f(1, 0, 0); glVertex3f( 1.0, -1.0,  1.0);
		glNormal3f(1, 0, 0); glVertex3f( 1.0, -1.0, -1.0);
		glNormal3f(1, 0, 0); glVertex3f( 1.0,  1.0, -1.0);
		glNormal3f(1, 0, 0); glVertex3f( 1.0,  1.0,  1.0);

		// draw top
		glNormal3f(0, 1, 0); glVertex3f(-1.0,  1.0,  1.0);
		glNormal3f(0, 1, 0); glVertex3f( 1.0,  1.0,  1.0);
		glNormal3f(0, 1, 0); glVertex3f( 1.0,  1.0, -1.0);
		glNormal3f(0, 1, 0); glVertex3f(-1.0,  1.0, -1.0);

		// draw bottom
		glNormal3f(0, -1, 0); glVertex3f(-1.0, -1.0, -1.0);
		glNormal3f(0, -1, 0); glVertex3f( 1.0, -1.0, -1.0);
		glNormal3f(0, -1, 0); glVertex3f( 1.0, -1.0,  1.0);
		glNormal3f(0, -1, 0); glVertex3f(-1.0, -1.0,  1.0);

	glEnd();
}

// depth and width are as pictured in orientation on a2 handout
void drawTrapazoid(const float height, const float upper_width, const float lower_width, const float upper_depth, const float lower_depth)
{
	const float h_upper_width = upper_width / 2;
	const float h_upper_depth = upper_depth / 2;
	const float h_lower_width = lower_width / 2;
	const float h_lower_depth = lower_depth / 2;	
	const float h_height = height / 2;
	const float non_overlap_hwidth = h_lower_width - h_upper_width;
	const float non_overlap_hdepth = h_lower_depth - h_upper_depth;
	glBegin(GL_QUADS);

		// draw top
		glNormal3f(0, 1, 0); glVertex3f(h_upper_width, h_height, h_upper_depth);
		glNormal3f(0, 1, 0); glVertex3f(h_upper_width, h_height, -h_upper_depth);
		glNormal3f(0, 1, 0); glVertex3f(-h_upper_width,  h_height, -h_upper_depth);
		glNormal3f(0, 1, 0); glVertex3f(-h_upper_width,  h_height, h_upper_depth);

		// draw bottom
		glNormal3f(0, -1, 0); glVertex3f(h_lower_width, -h_height, h_lower_depth);
		glNormal3f(0, -1, 0); glVertex3f(h_lower_width, -h_height, -h_lower_depth);
		glNormal3f(0, -1, 0); glVertex3f(-h_lower_width,  -h_height, -h_lower_depth);
		glNormal3f(0, -1, 0); glVertex3f(-h_lower_width,  -h_height, h_lower_depth);

		// TODO: normals
		// draw left side
		glNormal3f(-height, non_overlap_hwidth, 0); glVertex3f(-h_lower_width, -h_height,  h_lower_depth);
		glNormal3f(-height, non_overlap_hwidth, 0); glVertex3f(-h_lower_width, -h_height,  -h_lower_depth);
		glNormal3f(-height, non_overlap_hwidth, 0); glVertex3f(-h_upper_width,  h_height, -h_upper_depth);
		glNormal3f(-height, non_overlap_hwidth, 0); glVertex3f(-h_upper_width,  h_height,  h_upper_depth);

		// draw right side
		glNormal3f(height, non_overlap_hwidth, 0); glVertex3f(h_lower_width, -h_height,  h_lower_depth);
		glNormal3f(height, non_overlap_hwidth, 0); glVertex3f(h_lower_width, -h_height,  -h_lower_depth);
		glNormal3f(height, non_overlap_hwidth, 0); glVertex3f(h_upper_width,  h_height, -h_upper_depth);
		glNormal3f(height, non_overlap_hwidth, 0); glVertex3f(h_upper_width,  h_height,  h_upper_depth);

		// draw front
		glNormal3f(0, non_overlap_hdepth, height); glVertex3f(h_lower_width,  -h_height,  h_lower_depth);
		glNormal3f(0, non_overlap_hdepth, height); glVertex3f(h_upper_width,  h_height,  h_upper_depth);
		glNormal3f(0, non_overlap_hdepth, height); glVertex3f(-h_upper_width,  h_height, h_upper_depth);
		glNormal3f(0, non_overlap_hdepth, height); glVertex3f(-h_lower_width,  -h_height, h_lower_depth);

		// draw back
		glNormal3f(0, non_overlap_hdepth, -height); glVertex3f(h_lower_width,  -h_height,  -h_lower_depth);
		glNormal3f(0, non_overlap_hdepth, -height); glVertex3f(h_upper_width,  h_height,  -h_upper_depth);
		glNormal3f(0, non_overlap_hdepth, -height); glVertex3f(-h_upper_width,  h_height, -h_upper_depth);
		glNormal3f(0, non_overlap_hdepth, -height); glVertex3f(-h_lower_width,  -h_height, -h_lower_depth);

	glEnd();
}

// Draw an isocelles triangle. Note that height and width refer to properties of the 2D triangle.
void drawTriangle(const float thickness, const float width, const float height) {
	const float h_thickness = thickness / 2;
	const float h_width = width / 2;
	const float h_height = height / 2;

	glBegin(GL_TRIANGLES);

		// draw top
		glNormal3f(0, 1, 0); glVertex3f(0,  h_thickness,  -h_height);
		glNormal3f(0, 1, 0); glVertex3f(h_width,  h_thickness,  h_height);
		glNormal3f(0, 1, 0); glVertex3f(-h_width, h_thickness, h_height);

		// draw bottom
		glNormal3f(0, -1, 0); glVertex3f(0,  -h_thickness,  -h_height);
		glNormal3f(0, -1, 0); glVertex3f(h_width,  -h_thickness,  h_height);
		glNormal3f(0, -1, 0); glVertex3f(-h_width, -h_thickness, h_height);


	glEnd();
	glBegin(GL_QUADS);

		glNormal3f(height, 0, -h_width); glVertex3f(0,  h_thickness,  -h_height);
		glNormal3f(height, 0, -h_width); glVertex3f(0,  -h_thickness,  -h_height);
		glNormal3f(height, 0, -h_width); glVertex3f(h_width,  -h_thickness,  h_height);
		glNormal3f(height, 0, -h_width); glVertex3f(h_width,  h_thickness,  h_height);

		glNormal3f(0, 0, 1); glVertex3f(h_width,  -h_thickness,  h_height);
		glNormal3f(0, 0, 1); glVertex3f(h_width,  h_thickness,  h_height);
		glNormal3f(0, 0, 1); glVertex3f(-h_width,  h_thickness,  h_height);
		glNormal3f(0, 0, 1); glVertex3f(-h_width,  -h_thickness,  h_height);

		glNormal3f(-height, 0, -h_width); glVertex3f(-h_width,  h_thickness,  h_height);
		glNormal3f(-height, 0, -h_width); glVertex3f(-h_width,  -h_thickness,  h_height);
		glNormal3f(-height, 0, -h_width); glVertex3f(0,  -h_thickness,  -h_height);
		glNormal3f(-height, 0, -h_width); glVertex3f(0,  h_thickness,  -h_height);


	glEnd();
}

///////////////////////////////////////////////////////////
//
// BELOW ARE FUNCTIONS FOR GENERATING IMAGE FILES (PPM/PGM)
//
///////////////////////////////////////////////////////////

void writePGM(char* filename, GLubyte* buffer, int width, int height, bool raw=true)
{
	FILE* fp = fopen(filename,"wt");

	if( fp == NULL )
	{
		printf("WARNING: Can't open output file %s\n",filename);
		return;
	}

	if( raw )
	{
		fprintf(fp,"P5\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			fwrite(&buffer[y*width],sizeof(GLubyte),width,fp);
			/*
			for(int x=0;x<width;x++)
			{
				fprintf(fp,"%c",int(buffer[x+y*width];
			}
			*/
		}
	}
	else
	{
		fprintf(fp,"P2\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			for(int x=0;x<width;x++)
			{
				fprintf(fp,"%d ",int(buffer[x+y*width]));
			}
			fprintf(fp,"\n");
		}
	}

	fclose(fp);
}

#define RED_OFFSET   0
#define GREEN_OFFSET 1
#define BLUE_OFFSET  2

void writePPM(char* filename, GLubyte* buffer, int width, int height, bool raw=true)
{
	FILE* fp = fopen(filename,"wt");

	if( fp == NULL )
	{
		printf("WARNING: Can't open output file %s\n",filename);
		return;
	}

	if( raw )
	{
		fprintf(fp,"P6\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			for(int x=0;x<width;x++)
			{
				GLubyte* pix = &buffer[4*(x+y*width)];

				fprintf(fp,"%c%c%c",int(pix[RED_OFFSET]),
									int(pix[GREEN_OFFSET]),
									int(pix[BLUE_OFFSET]));
			}
		}
	}
	else
	{
		fprintf(fp,"P3\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			for(int x=0;x<width;x++)
			{
				GLubyte* pix = &buffer[4*(x+y*width)];

				fprintf(fp,"%d %d %d ",int(pix[RED_OFFSET]),
									   int(pix[GREEN_OFFSET]),
									   int(pix[BLUE_OFFSET]));
			}
			fprintf(fp,"\n");
		}
	}

	fclose(fp);
}

void writeFrame(char* filename, bool pgm, bool frontBuffer)
{
	static GLubyte* frameData = NULL;
	static int currentSize = -1;

	int size = (pgm ? 1 : 4);

	if( frameData == NULL || currentSize != size*Win[0]*Win[1] )
	{
		if (frameData != NULL)
			delete [] frameData;

		currentSize = size*Win[0]*Win[1];

		frameData = new GLubyte[currentSize];
	}

	glReadBuffer(frontBuffer ? GL_FRONT : GL_BACK);

	if( pgm )
	{
		glReadPixels(0, 0, Win[0], Win[1],
					 GL_LUMINANCE, GL_UNSIGNED_BYTE, frameData);
		writePGM(filename, frameData, Win[0], Win[1]);
	}
	else
	{
		glReadPixels(0, 0, Win[0], Win[1],
					 GL_RGBA, GL_UNSIGNED_BYTE, frameData);
		writePPM(filename, frameData, Win[0], Win[1]);
	}
}
