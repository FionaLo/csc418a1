/***********************************************************
             CSC418/2504, Winter 2013
  
                 robot.cpp
                 
       Simple demo program using OpenGL and the glut/glui 
       libraries

  
    Instructions:
        Please read the assignment page to determine 
        exactly what needs to be implemented.  Then read 
        over this file and become acquainted with its 
        design.

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

#ifndef _WIN32
#include <unistd.h>
#else
void usleep(unsigned int nanosec)
{
    Sleep(nanosec / 1000);
}
#endif


// *************** GLOBAL VARIABLES *************************


const float PI = 3.14159;

// --------------- USER INTERFACE VARIABLES -----------------

// Window settings
int windowID;               // Glut window ID (for display)
GLUI *glui;                 // Glui window (for controls)
int Win[2];                 // window (x,y) size


// ---------------- ANIMATION VARIABLES ---------------------

// Animation settings
int animate_mode = 0;       // 0 = no anim, 1 = animate
int animation_frame = 0;      // Specify current frame of animation

// Joint parameters
const float JOINT_MIN = -45.0f;
const float JOINT_MAX =  45.0f;
const float MAX_BEAK_OPEN = 10;
const float HEAD_MAX_ROT = 10;

float joint_rot = 0.0f;
float beak_trans = 0.0f;
float head_rot = 0.0f;
float fin_rot = 0.0f;
float left_leg_upper_rot = 0.0f;
float left_leg_lower_rot = 0.0f;
float right_leg_upper_rot = 0.0f;
float right_leg_lower_rot = 0.0f;
float penguin_horizontal_trans = 0.0f;
float penguin_vertical_trans = 0.0f;

//////////////////////////////////////////////////////
// TODO: Add additional joint parameters here
//////////////////////////////////////////////////////



// ***********  FUNCTION HEADER DECLARATIONS ****************


// Initialization functions
void initGlut(char* winName);
void initGlui();
void initGl();


// Callbacks for handling events in glut
void myReshape(int w, int h);
void animate();
void display(void);

// Callback for handling events in glui
void GLUI_Control(int id);


// Functions to help draw the object
void drawSquare(float size);
void drawShape(float width, float height, float delta, float slant_percentage);
void drawTorso(const float height, const float width);
void drawHead(const float height, const float width);
void shear(float shear_x, float shear_y);
void drawCircle(float cx, float cy, float r, int num_segments, bool filled);
void drawBeak(const float height, const float width);
void drawFin(const float height, const float width);
void jointAt(const float x, const float y, const float &variable);
void drawTrapazoid(const float height, const float width, const float upper_width_percentage);
// Return the current system clock (in seconds)
double getTime();


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
        printf("Using 300x200 window by default...\n");
        Win[0] = 1000;
        Win[1] = 750;
    } else {
        Win[0] = atoi(argv[1]);
        Win[1] = atoi(argv[2]);
    }


    // Initialize glut, glui, and opengl
    glutInit(&argc, argv);
    initGlut(argv[0]);
    initGlui();
    initGl();

    // Invoke the standard GLUT main event loop
    glutMainLoop();

    return 0;         // never reached
}


// Initialize glut and create a window with the specified caption 
void initGlut(char* winName)
{
    // Set video mode: double-buffered, color, depth-buffered
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Create window
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Win[0],Win[1]);
    windowID = glutCreateWindow(winName);

    // Setup callback functions to handle events
    glutReshapeFunc(myReshape); // Call myReshape whenever window resized
    glutDisplayFunc(display);   // Call display whenever new frame needed 
}


// Quit button handler.  Called when the "quit" button is pressed.
void quitButton(int)
{
  exit(0);
}

// Animate button handler.  Called when the "animate" checkbox is pressed.
void animateButton(int)
{
  // synchronize variables that GLUT uses
  glui->sync_live();

  animation_frame = 0;
  if(animate_mode == 1) {
    // start animation
    GLUI_Master.set_glutIdleFunc(animate);
  } else {
    // stop animation
    GLUI_Master.set_glutIdleFunc(NULL);
  }
}

// Initialize GLUI and the user interface
void initGlui()
{
    GLUI_Master.set_glutIdleFunc(NULL);

    // Create GLUI window
    glui = GLUI_Master.create_glui("Glui Window", 0, Win[0]+10, 0);

    // Create a control to specify the rotation of the joint
    GLUI_Spinner *joint_spinner
        = glui->add_spinner("Joint", GLUI_SPINNER_FLOAT, &joint_rot);
    joint_spinner->set_speed(0.1);
    joint_spinner->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);


    #define SPINNER(name, variable, min, max) { \
        GLUI_Spinner *spinner = glui->add_spinner(#name, GLUI_SPINNER_FLOAT, &variable);\
        spinner->set_speed(0.1);\
        spinner->set_float_limits(min, max, GLUI_LIMIT_CLAMP);\
    }
    SPINNER(beak, beak_trans, 0, MAX_BEAK_OPEN)
    SPINNER(head, head_rot, -HEAD_MAX_ROT, HEAD_MAX_ROT);
    SPINNER(fin, fin_rot, JOINT_MIN, JOINT_MAX);
    SPINNER(left_leg_lower, left_leg_lower_rot, JOINT_MIN, JOINT_MAX);
    SPINNER(left_leg_upp, left_leg_upper_rot, JOINT_MIN, JOINT_MAX);
    SPINNER(right_leg_lower, right_leg_lower_rot, JOINT_MIN, JOINT_MAX);
    SPINNER(right_leg_upp, right_leg_upper_rot, JOINT_MIN, JOINT_MAX);


    ///////////////////////////////////////////////////////////
    // TODO: 
    //   Add controls for additional joints here
    ///////////////////////////////////////////////////////////

    // Add button to specify animation mode 
    glui->add_separator();
    glui->add_checkbox("Animate", &animate_mode, 0, animateButton);

    // Add "Quit" button
    glui->add_separator();
    glui->add_button("Quit", 0, quitButton);

    // Set the main window to be the "active" window
    glui->set_main_gfx_window(windowID);
}


// Performs most of the OpenGL intialization
void initGl(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);
}




// Callback idle function for animating the scene
void animate()
{
    // Update geometry
    const double joint_rot_speed = 0.1;
    double joint_rot_t = (sin(animation_frame*joint_rot_speed) + 1.0) / 2.0;
    double cos_wave = (cos(animation_frame*joint_rot_speed) + 1.0) / 2.0;
    double head_wave = (sin(animation_frame*joint_rot_speed/2) + 1.0) / 2.0;
    double walk_wave = (cos(animation_frame*joint_rot_speed/4) + 1.0) / 2.0;

    joint_rot = joint_rot_t * JOINT_MIN + (1 - joint_rot_t) * JOINT_MAX;
    
    ///////////////////////////////////////////////////////////
    // TODO:
    //   Modify this function animate the character's joints
    //   Note: Nothing should be drawn in this function!  OpenGL drawing
    //   should only happen in the display() callback.
    ///////////////////////////////////////////////////////////
    beak_trans = joint_rot_t * JOINT_MIN + (1 - joint_rot_t) * JOINT_MAX;
    head_rot = head_wave * -HEAD_MAX_ROT + (1 - head_wave) * HEAD_MAX_ROT;
    fin_rot = joint_rot_t * JOINT_MIN + (1 - joint_rot_t) * JOINT_MAX;
    left_leg_upper_rot = joint_rot_t * JOINT_MIN + (1 - joint_rot_t) * JOINT_MAX;
    left_leg_lower_rot = joint_rot_t * 0 + (1 - joint_rot_t) * JOINT_MAX;
    right_leg_upper_rot = joint_rot_t * JOINT_MIN + (1 - joint_rot_t) * JOINT_MAX;
    right_leg_lower_rot = joint_rot_t * 0 + (1 - joint_rot_t) * JOINT_MAX;
    penguin_horizontal_trans = walk_wave * JOINT_MIN * 5 + (1 - walk_wave) * JOINT_MAX * 5;

    // Update user interface
    glui->sync_live();

    // Tell glut window to update itself.  This will cause the display()
    // callback to be called, which renders the object (once you've written
    // the callback).
    glutSetWindow(windowID);
    glutPostRedisplay();

    // increment the frame number.
    animation_frame++;

    // Wait 50 ms between frames (20 frames per second)
    usleep(50000);
}


// Handles the window being resized by updating the viewport
// and projection matrices
void myReshape(int w, int h)
{
    // Setup projection matrix for new window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-w/2, w/2, -h/2, h/2);

    // Update OpenGL viewport and internal variables
    glViewport(0,0, w,h);
    Win[0] = w;
    Win[1] = h;
}



// display callback
//
// This gets called by the event handler to draw
// the scene, so this is where you need to build
// your scene -- make your changes and additions here.
// All rendering happens in this function.  For Assignment 1,
// updates to geometry should happen in the "animate" function.
void display(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);

    // OK, now clear the screen with the background colour
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

    // Setup the model-view transformation matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    ///////////////////////////////////////////////////////////
    // TODO:
    //   Modify this function draw the scene
    //   This should include function calls to pieces that
    //   apply the appropriate transformation matrice and
    //   render the individual body parts.
    ///////////////////////////////////////////////////////////
    const float TORSO_HEIGHT = 520;
    const float TORSO_WIDTH = 350;
    const float HEAD_HEIGHT = 130;
    const float HEAD_WIDTH = 160;
    const float BEAK_HEIGHT = 40;
    const float BEAK_WIDTH  = 110;
    const float FIN_HEIGHT = 200;
    const float FIN_WIDTH = 100;
    const float UPPER_LEG_HEIGHT = 140;
    const float UPPER_LEG_WIDTH = 50;
    const float LOWER_LEG_LENGTH = 160;
    const float LOWER_LEG_THINKNESS = 30; 

    glPushMatrix();
        glTranslatef(penguin_horizontal_trans, penguin_vertical_trans, 0);
        drawTorso(TORSO_HEIGHT, TORSO_WIDTH);
        glPushMatrix();
            glPushMatrix();
                glTranslatef(TORSO_WIDTH * 0.05, TORSO_HEIGHT * 0.1, 0);
                jointAt(0, -FIN_HEIGHT * 0.4, fin_rot);
                drawFin(FIN_HEIGHT, FIN_WIDTH);
            glPopMatrix();
            glPushMatrix();
                glTranslatef(TORSO_WIDTH * 0.18, -TORSO_HEIGHT * 0.5, 0);
                glRotatef(10, 0, 0, 1);
                jointAt(0, - UPPER_LEG_HEIGHT * 0.4, right_leg_upper_rot);
                drawTrapazoid(UPPER_LEG_HEIGHT, UPPER_LEG_WIDTH, 0.8);
                    glPushMatrix();
                        glTranslatef(-(LOWER_LEG_LENGTH - UPPER_LEG_WIDTH) / 2, -(UPPER_LEG_HEIGHT - LOWER_LEG_THINKNESS) / 2 + 5, 0);
                        jointAt(-LOWER_LEG_LENGTH * 0.4, 0, right_leg_lower_rot);
                        glScalef(LOWER_LEG_LENGTH, LOWER_LEG_THINKNESS, 1);
                        drawSquare(1);
                    glPopMatrix();
            glPopMatrix();
            glPushMatrix();
                glTranslatef(-TORSO_WIDTH * 0.25, -TORSO_HEIGHT * 0.45, 0);
                glRotatef(-15, 0, 0, 1);
                jointAt(0, - UPPER_LEG_HEIGHT * 0.4, left_leg_upper_rot);
                drawTrapazoid(UPPER_LEG_HEIGHT, UPPER_LEG_WIDTH, 0.8);
                    glPushMatrix();
                        glTranslatef(-(LOWER_LEG_LENGTH - UPPER_LEG_WIDTH) / 2, -(UPPER_LEG_HEIGHT - LOWER_LEG_THINKNESS) / 2 + 5, 0);
                        jointAt(-LOWER_LEG_LENGTH * 0.4, 0, left_leg_lower_rot);
                        glScalef(LOWER_LEG_LENGTH, LOWER_LEG_THINKNESS, 1);
                        drawSquare(1);
                    glPopMatrix();
            glPopMatrix();
            glPushMatrix();
                glTranslatef(0, TORSO_HEIGHT * 0.57, 0); // move to top of torso
                jointAt(0, HEAD_HEIGHT * 0.4, head_rot);
                drawHead(HEAD_HEIGHT, HEAD_WIDTH);
                glTranslatef(- TORSO_WIDTH / 3, 0, 0);
                drawBeak(BEAK_HEIGHT, BEAK_WIDTH);
                glPushMatrix();
                    glTranslatef(0, -BEAK_HEIGHT * 0.6 - beak_trans, 0);
                    glScalef(BEAK_WIDTH, 10, 1);
                    drawSquare(1);
                glPopMatrix();
            glPopMatrix();
        glPopMatrix();
    glPopMatrix();

 //    // Draw our hinged object
 //    const float BODY_WIDTH = 30.0f;
 //    const float BODY_LENGTH = 50.0f;
 //    const float ARM_LENGTH = 50.0f;
 //    const float ARM_WIDTH = 10.0f;

 //    // Push the current transformation matrix on the stack
 //    glPushMatrix();
        
 //        // Draw the 'body'
 //        glPushMatrix();
 //            // Scale square to size of body
 //            glScalef(BODY_WIDTH, BODY_LENGTH, 1.0);

 //            // Set the colour to green
 //            glColor3f(0.0, 1.0, 0.0);

 //            // Draw the square for the body
 //            drawSquare(1.0);
 //        glPopMatrix();
 
 //        // Draw the 'arm'

 //        // Move the arm to the joint hinge
 //        glTranslatef(0.0, -BODY_LENGTH/2 + ARM_WIDTH, 0.0);

 //        // Rotate along the hinge
 //        glRotatef(joint_rot, 0.0, 0.0, 1.0);

 //        // Scale the size of the arm
 //        glScalef(ARM_WIDTH, ARM_LENGTH, 1.0);

 //        // Move to center location of arm, under previous rotation
 //        glTranslatef(0.0, -0.5, 0.0);

 //        // Draw the square for the arm
 //        glColor3f(1.0, 0.0, 0.0);
 //        drawSquare(1.0);

 //    // Retrieve the previous state of the transformation stack
 //    glPopMatrix();


    // Execute any GL functions that are in the queue just to be safe
    glFlush();

    // Now, show the frame buffer that we just drew into.
    // (this prevents flickering).
    glutSwapBuffers();
}


// Draw a square of the specified size, centered at the current location
void drawSquare(float width)
{
    // Draw the square
    glBegin(GL_LINE_LOOP);
    glVertex2d(-width/2, -width/2);
    glVertex2d(width/2, -width/2);
    glVertex2d(width/2, width/2);
    glVertex2d(-width/2, width/2);
    glEnd();
}

void drawShape(float width, float height, float delta, float slant_percentage) {
    const float h_width = width / 2;
    const float h_height = height / 2;
    // Draw the square
    glBegin(GL_LINE_LOOP);
        glVertex2d(h_width, -h_height);
        glVertex2d(h_width - delta, h_height);
        glVertex2d(-h_width, h_height - slant_percentage * height);
        glVertex2d(-h_width + delta, -h_height);
    glEnd();   
}

void drawTorso(const float height, const float width) {
    glPushMatrix();
        glTranslatef(((0.65 - 0.35) * width) / 2, -height / 2, 0); // translate the object's center to the origin
         glBegin(GL_LINE_LOOP);
            // draw the head, with the origin being placed at the lowest vertex, at the right leg
            glVertex2d(0, 0);
            glVertex2d(0.35 * width, 0.21 * height);
            glVertex2d(0.05 * width, height);
            glVertex2d(-0.34 * width, height);
            glVertex2d(-0.65 * width, 0.25 * height);
            glVertex2d(-0.29 * width, 0.02 * height);
        glEnd();
    glPopMatrix();
}

void drawHead(const float height, const float width) {
    glPushMatrix();
        glTranslatef((-(0.63 - 0.37) / 2) * width, -height / 2, 0); // translate the object's center to the origin
        glBegin(GL_LINE_LOOP);
            // draw the head, with the origin being placed at the head's bottom, right under its highest point
            glVertex2d(0.63 * width, 0);
            glVertex2d(0.5 * width, 0.75 * height);
            glVertex2d(0, height);
            glVertex2d(-0.25 * width, 0.85 * height);
            glVertex2d(-0.37 * width, 0);
        glEnd();
        drawCircle(-width * 0.1, 0.8 * height, 10, 100, false); // outer eye
        drawCircle(-width * 0.11, 0.8 * height, 7, 100, true); // outer eye
    glPopMatrix();
}

void drawBeak(const float height, const float width) {
    drawShape(width, height, 2, 0.6);
}

void drawFin(const float height, const float width) {
    glPushMatrix();
        glRotatef(90, 0, 0, 1);
        drawShape(height, width, 2, 0.6);
    glPopMatrix();
}

void shear(float shear_x, float shear_y) {
    float shear[] = { 
       1, shear_y, 0, 0, 
       shear_x, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1 };
    glMultMatrixf(shear);
}

void drawCircle(float cx, float cy, float r, int num_segments, bool filled) 
{ 
    GLenum mode = filled ? GL_POLYGON : GL_LINE_LOOP;
    glBegin(mode); 
    for(int i = 0; i < num_segments; i++) 
    { 
        float theta = 2.0f * 3.1415926f * float(i) / float(num_segments);//get the current angle 

        float x = r * cosf(theta);//calculate the x component 
        float y = r * sinf(theta);//calculate the y component 

        glVertex2f(x + cx, y + cy);//output vertex 

    } 
    glEnd(); 
}

void jointAt(const float x, const float y, const float &variable) {
    glTranslatef(-x, -y, 0); // move origin back 
    glRotatef(variable, 0, 0, 1); // rotate around joint
    drawCircle(0, 0, 5, 100, false); // draw joint
    glTranslatef(x, y, 0); // move origin to joint
}

void drawTrapazoid(const float height, const float width, const float upper_width_percentage) {
    const float h_width = width / 2;
    const float upper_h_width = (width * upper_width_percentage) / 2;
    const float h_height = height / 2;
    glBegin(GL_LINE_LOOP);
        glVertex2d(upper_h_width, h_height);
        glVertex2d(-upper_h_width, h_height);
        glVertex2d(-h_width, -h_height);
        glVertex2d(h_width, -h_height);
    glEnd();
}