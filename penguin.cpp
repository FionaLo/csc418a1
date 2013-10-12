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

// define some colors
#define GREY 0.5, 0.5, 0.5
#define ORANGE 1.0, 0.5, 0.0
#define BLUE 0.0, 0.0, 1.0
#define RED 1.0, 0.0, 0.0
#define BLACK 0.0, 0.0, 0.0
#define WHITE 1.0, 1.0, 1.0

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

// scale the penguin
// change these if you increase the window size from the default for a larger penguin
float penguin_y_scale_factor = 0.2f;
float penguin_x_scale_factor = 0.2f;

// Sizing variables
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
// an estimate on the penguin height
const float PENGUIN_HEIGHT = (TORSO_HEIGHT + HEAD_HEIGHT + UPPER_LEG_HEIGHT + LOWER_LEG_LENGTH) * penguin_y_scale_factor; 
// an estimate on penguin width
const float PENGUIN_WIDTH = (TORSO_WIDTH + BEAK_WIDTH) * penguin_x_scale_factor;

// Joint parameters
const float HEAD_MAX_ROT = 10;
const float UPPER_LEG_MAX_ROT = 30;
const float LOWER_LEG_MAX_ROT = 20;
const float FIN_MIN_ROT = -45;
const float FIN_MAX_ROT = 45;

float head_rot = 0.0f;
float fin_rot = 0.0f;
float left_leg_upper_rot = 0.0f;
float left_leg_lower_rot = 0.0f;
float right_leg_upper_rot = 0.0f;
float right_leg_lower_rot = 0.0f;

// Translation parameters
float penguin_horizontal_trans = 0.0f;
float penguin_vertical_trans = 0.0f;
float beak_trans = 0.0f;

const float MAX_BEAK_OPEN = 10;
// can't set till main when you know the window size
float PENGUIN_MAX_TRANS;


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
void drawBeakFinShape(float width, float height, float slant_percentage);
void drawTorso(const float height, const float width);
void drawHead(const float height, const float width);
void drawCircle(float cx, float cy, float r, int num_segments, bool filled);
void drawBeak(const float height, const float width);
void drawFin(const float height, const float width);
void jointAt(const float x, const float y, const float &variable);
void drawTrapazoid(const float height, const float width, const float upper_width_percentage);
void drawLeg(const float &upper_rot_variable, const float &lower_rot_variable);
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
        Win[0] = 300;
        Win[1] = 200;
    } else {
        Win[0] = atoi(argv[1]);
        Win[1] = atoi(argv[2]);
    }

    // set this here, becasue it requires knowing the window size. 
    // unfortunately can't resize later because of spinner
    PENGUIN_MAX_TRANS = Win[0] / 2 + PENGUIN_WIDTH;

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

    // define a macro for easily creating spinners
    #define SPINNER(name, variable, min, max) { \
        GLUI_Spinner *spinner = glui->add_spinner(#name, GLUI_SPINNER_FLOAT, &variable);\
        spinner->set_speed(0.1);\
        spinner->set_float_limits(min, max, GLUI_LIMIT_CLAMP);\
    }
    SPINNER(beak, beak_trans, 0, MAX_BEAK_OPEN)
    SPINNER(head, head_rot, -HEAD_MAX_ROT, HEAD_MAX_ROT);
    SPINNER(fin, fin_rot, FIN_MIN_ROT, FIN_MAX_ROT);
    SPINNER(left_leg_lower, left_leg_lower_rot, -LOWER_LEG_MAX_ROT, LOWER_LEG_MAX_ROT);
    SPINNER(left_leg_upp, left_leg_upper_rot, -UPPER_LEG_MAX_ROT, UPPER_LEG_MAX_ROT);
    SPINNER(right_leg_lower, right_leg_lower_rot, -LOWER_LEG_MAX_ROT, LOWER_LEG_MAX_ROT);
    SPINNER(right_leg_upp, right_leg_upper_rot, -UPPER_LEG_MAX_ROT, UPPER_LEG_MAX_ROT);
    SPINNER(horiz, penguin_horizontal_trans, -PENGUIN_MAX_TRANS, PENGUIN_MAX_TRANS);
    #undef SPINNER

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
    // get some sin and cos waves to use of varying speeds
    const double joint_rot_speed = 0.1;
    const double sin_wave = (sin(animation_frame*joint_rot_speed) + 1.0) / 2.0;
    const double slow_sin_wave = (sin(animation_frame*joint_rot_speed/2) + 1.0) / 2.0;
    const double cos_wave = (cos(animation_frame*joint_rot_speed) + 1.0) / 2.0;
    
    // a static boolean to remember if the penguin is jumping between invocations of the function
    static bool jumping = false;

    // using the above waves, compute some joint rotations
    head_rot = slow_sin_wave * -HEAD_MAX_ROT + (1 - slow_sin_wave) * HEAD_MAX_ROT;
    fin_rot = sin_wave * FIN_MIN_ROT + (1 - sin_wave) * FIN_MAX_ROT;
    left_leg_upper_rot = cos_wave * -UPPER_LEG_MAX_ROT + (1 - cos_wave) * UPPER_LEG_MAX_ROT;
    left_leg_lower_rot = cos_wave * -LOWER_LEG_MAX_ROT + (1 - cos_wave) * LOWER_LEG_MAX_ROT;
    right_leg_upper_rot = sin_wave * -UPPER_LEG_MAX_ROT + (1 - sin_wave) * UPPER_LEG_MAX_ROT;
    right_leg_lower_rot = sin_wave * -LOWER_LEG_MAX_ROT + (1 - sin_wave) * LOWER_LEG_MAX_ROT;

    // compute how much to open the beak
    beak_trans = (1 - slow_sin_wave) * MAX_BEAK_OPEN;

    // compute how much to walk, and remember to walk in the direction we are facing
    penguin_horizontal_trans += copysign(5, -penguin_x_scale_factor);

    // check for offscreen
    if (abs(penguin_horizontal_trans) - PENGUIN_WIDTH / 2 > Win[0] / 2) { 
        // if we weren't jumping, start jumping. If we were, stop
        jumping = !jumping;
        penguin_vertical_trans = 0;
        // turn around
        penguin_x_scale_factor = -penguin_x_scale_factor;
    }
    if (jumping) {
        penguin_vertical_trans = (1 - cos_wave) * Win[1] / 8;
    }

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

    glPushMatrix();
        // translate the entire penguin
        glTranslatef(penguin_horizontal_trans, penguin_vertical_trans, 0); 
        // scale the penguin
        glScalef(penguin_x_scale_factor, penguin_y_scale_factor, 1);
        drawTorso(TORSO_HEIGHT, TORSO_WIDTH);
        // draw the fin
        glPushMatrix();
            // move fin to top right of torso
            glTranslatef(TORSO_WIDTH * 0.05, TORSO_HEIGHT * 0.1, 0);
            // create joint between torso and fin
            jointAt(0, -FIN_HEIGHT * 0.4, fin_rot);
            drawFin(FIN_HEIGHT, FIN_WIDTH);
        glPopMatrix();
        // draw the left leg
        glPushMatrix();
            // move the leg to the bottom left of the torso
            glTranslatef(TORSO_WIDTH * 0.18, -TORSO_HEIGHT * 0.5, 0);
            // set some constant rotation offset
            glRotatef(10, 0, 0, 1);
            drawLeg(right_leg_upper_rot, right_leg_lower_rot);
        glPopMatrix();
        // draw the right leg
        glPushMatrix();
            // move the leg to the bottom right of the torso
            glTranslatef(-TORSO_WIDTH * 0.25, -TORSO_HEIGHT * 0.45, 0);
            // set some constant rotation offset
            glRotatef(-15, 0, 0, 1);
            drawLeg(left_leg_upper_rot, left_leg_lower_rot);
        glPopMatrix();
        // draw the head
        glPushMatrix();
            // move head to top of torso
            glTranslatef(0, TORSO_HEIGHT * 0.57, 0); 
            // create joint between head and torso
            jointAt(0, HEAD_HEIGHT * 0.4, head_rot);
            drawHead(HEAD_HEIGHT, HEAD_WIDTH);
            glPushMatrix();
                // move beak to proper place on head
                glTranslatef(-TORSO_WIDTH / 3, 0, 0);
                drawBeak(BEAK_HEIGHT, BEAK_WIDTH);
            glPopMatrix();
        glPopMatrix();
    glPopMatrix();

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

// draw the shape used for both the beak and fin, centered at the current location. 
// it is drawn in the orientation used for the beak
// slant percentange controls the slope of the top of the beak / side of fin
void drawBeakFinShape(float width, float height, float slant_percentage) {
    const float h_width = width / 2;
    const float h_height = height / 2;
    glBegin(GL_LINE_LOOP);
        glVertex2d(h_width, -h_height);
        glVertex2d(h_width, h_height);
        glVertex2d(-h_width, h_height - slant_percentage * height);
        glVertex2d(-h_width, -h_height);
    glEnd();   
}

// draw the penguin's torso, centered at the current location
void drawTorso(const float height, const float width) {
    glColor3f(WHITE);
    glPushMatrix();
        // translate the object's center to the origin so that other functions are easier to reason about
        glTranslatef(((0.65 - 0.35) * width) / 2, -height / 2, 0);
        glBegin(GL_LINE_LOOP);
            // draw the torso, with the origin being placed at the lowest vertex, the one near the right leg
            // the percentages are based on rough measurements of the assignment image
            glVertex2d(0, 0);
            glVertex2d(0.35 * width, 0.21 * height);
            glVertex2d(0.05 * width, height);
            glVertex2d(-0.34 * width, height);
            glVertex2d(-0.65 * width, 0.25 * height);
            glVertex2d(-0.29 * width, 0.02 * height);
        glEnd();
    glPopMatrix();
}

// draw the penguin's head, centered at the current location
void drawHead(const float height, const float width) {
    glPushMatrix();
        // translate the object's center to the origin so that other functions are easier to reason about
        glTranslatef((-(0.63 - 0.37) / 2) * width, -height / 2, 0); 
        glColor3f(BLUE); 
        glBegin(GL_LINE_LOOP);
            // draw the head, with the origin at the head's bottom, right under its highest point
            // the percentages are based on rough measurements of the assignment image
            glVertex2d(0.63 * width, 0);
            glVertex2d(0.5 * width, 0.75 * height);
            glVertex2d(0, height);
            glVertex2d(-0.25 * width, 0.85 * height);
            glVertex2d(-0.37 * width, 0);
        glEnd();
        // outer eye
        glColor3f(WHITE); 
        drawCircle(-width * 0.1, 0.8 * height, 10, 100, false); 
         // inner eye
        glColor3f(BLACK);
        drawCircle(-width * 0.11, 0.8 * height, 7, 100, true);
    glPopMatrix();
}

// draw the beak, centered at the current location
void drawBeak(const float height, const float width) {
    glColor3f(ORANGE); 
    drawBeakFinShape(width, height, 0.6);
    glPushMatrix();
        glTranslatef(0, -height * 0.6 - beak_trans, 0);
        glScalef(width, 10, 1);
        drawSquare(1);
    glPopMatrix();
}

// draw the fin, centered at the current location
void drawFin(const float height, const float width) {
    glColor3f(GREY); 
    glPushMatrix();
        glRotatef(90, 0, 0, 1);
        drawBeakFinShape(height, width, 0.6);
    glPopMatrix();
}

// draw a circle of radius r, centered at offset cx, cy from the current location
// a circle may be filled or unfilled
// a circle is composed of num_segments line segments
void drawCircle(float cx, float cy, float r, int num_segments, bool filled) 
{ 
    const GLenum mode = filled ? GL_POLYGON : GL_LINE_LOOP;
    glBegin(mode); 
    for(int i = 0; i < num_segments; i++) { 
        float theta = 2.0f * PI * float(i) / float(num_segments);
        float x = r * cosf(theta);
        float y = r * sinf(theta);
        glVertex2f(x + cx, y + cy);
    } 
    glEnd(); 
}

// create a joint at x, y offset from the current location. 
// the joint's rotation will be controlled by the variable argument
void jointAt(const float x, const float y, const float &variable) {
    // keep the current color so we can swap it back later
    float currentColor[4];
    glGetFloatv(GL_CURRENT_COLOR, currentColor);
    glColor3f(RED);
    glTranslatef(-x, -y, 0); // move origin back 
    glRotatef(variable, 0, 0, 1); // rotate around joint
    drawCircle(0, 0, 5, 100, false); // draw joint
    glTranslatef(x, y, 0); // move origin to joint
    // restore current color
    glColor3f(currentColor[0], currentColor[1], currentColor[2]);
}

// draw a trapazoid, centered at the current location. 
// upper_width_percentange controls the relative width of the upper to lower line
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

// draw a leg, centered the current location
// takes in both the upper and lower rotation variables
void drawLeg(const float &upper_rot_variable, const float &lower_rot_variable) {
    glColor3f(GREY); 
    // create the upper leg's joint
    jointAt(0, - UPPER_LEG_HEIGHT * 0.4, upper_rot_variable);
    // draw the upper leg
    drawTrapazoid(UPPER_LEG_HEIGHT, UPPER_LEG_WIDTH, 0.8);
    // draw the lower leg
    glPushMatrix();
        glTranslatef(-(LOWER_LEG_LENGTH - UPPER_LEG_WIDTH) / 2, -(UPPER_LEG_HEIGHT - LOWER_LEG_THINKNESS) / 2 + 5, 0);
        jointAt(-LOWER_LEG_LENGTH * 0.4, 0, lower_rot_variable);
        glScalef(LOWER_LEG_LENGTH, LOWER_LEG_THINKNESS, 1);
        drawSquare(1);
    glPopMatrix();
}