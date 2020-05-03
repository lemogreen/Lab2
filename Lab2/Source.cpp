#include <windows.h>
#include <GL/glut.h>
#include <iostream>
#include <cmath>

using namespace std;

struct type_point
{
	GLint x, y, z;
	type_point(GLint _x, GLint _y, GLint _z) { x = _x; y = _y; z = _z; }
};

type_point spectator = type_point(300, 200, 200);
float angle_x = 0;
float angle_y = 0;
int speed = 10;
int angle_speed = 1;

/* Initialize OpenGL Graphics */
void initGL() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
	glClearDepth(1.0f);                   // Set background depth to farthest
	glEnable(GL_DEPTH_TEST);   // Enable depth testing for z-culling
	glDepthFunc(GL_LEQUAL);    // Set the type of depth-test
	glShadeModel(GL_SMOOTH);   // Enable smooth shading
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Nice perspective corrections
}

/* Handler for window-repaint event. Called back when the window first appears and
   whenever the window needs to be re-painted. */
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers

	glMatrixMode(GL_MODELVIEW);     // To operate on model-view matrix

	// Render a color-cube consisting of 6 quads with different colors
	glLoadIdentity();                 // Reset the model-view matrix

	/*type_point view_vector = type_point(

	);*/

	type_point view_vector = type_point(0, 0, -100);

	gluLookAt(
		spectator.x, spectator.y, spectator.z,
		view_vector.x, view_vector.y, view_vector.z,
		0, 100, 0
		);

	glLineWidth(4);
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(50, 0, 0);
	glColor3f(0, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 50, 0);
	glColor3f(0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 50);
	glEnd();
	glLineWidth(1);

	glBegin(GL_LINES);
	glColor3f(187.0 / 256, 0, 182.0 / 256);
	for (int i = -10; i <= 10; i++)
	{
		glVertex3f(50 * i, 0, -500);
		glVertex3f(50 * i, 0, 500);
		glVertex3f(500, 0, 50 * i);
		glVertex3f(-500, 0, 50 * i);
	}
	glEnd();

	glBegin(GL_TRIANGLES);

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(100, 100, -100);
	glVertex3f(200, 100, -100);
	glVertex3f(150, 200, -100);

	glColor3f(0.4f, 0.2f, 0.4f);
	glVertex3f(100, 100, -400);
	glVertex3f(200, 100, -400);
	glVertex3f(150, 200, -400);

	glEnd();

	glBegin(GL_QUADS);

	glColor3f(1.0f, 1.0f, 0.0f);
	glVertex3f(100, 100, -100);
	glVertex3f(150, 200, -100);
	glVertex3f(150, 200, -400);
	glVertex3f(100, 100, -400);

	glColor3f(1.0f, 1.0f, 0.4f);
	glVertex3f(100, 100, -100);
	glVertex3f(200, 100, -100);
	glVertex3f(200, 100, -400);
	glVertex3f(100, 100, -400);

	glColor3f(1.0f, 0.0f, 1.0f);
	glVertex3f(200, 100, -100);
	glVertex3f(200, 100, -400);
	glVertex3f(150, 200, -400);
	glVertex3f(150, 200, -100);

	glEnd();

	glutSwapBuffers();  // Swap the front and back frame buffers (double buffering)
}

/* Handler for window re-size event. Called back when the window first appears and
   whenever the window is re-sized with its new width and height */
void reshape(GLsizei width, GLsizei height) {  // GLsizei for non-negative integer
   // Compute aspect ratio of the new window
	if (height == 0) height = 1;                // To prevent divide by 0
	GLfloat aspect = (GLfloat)width / (GLfloat)height;

	// Set the viewport to cover the new window
	glViewport(0, 0, width, height);

	// Set the aspect ratio of the clipping volume to match the viewport
	glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix
	glLoadIdentity();             // Reset

	gluPerspective(45.0f, aspect, 0.1f, 5000);
	// glOrtho(0, width, 0, height, 0, 5000);

	// Set up orthographic projection view [NEW]
	//if (width >= height) {
		// aspect >= 1, set the height from -1 to 1, with larger width
	//glOrtho(-3.0 * aspect, 3.0 * aspect, -3.0, 3.0, 0.1, 100);
	//}
	//else {
		// aspect < 1, set the width to -1 to 1, with larger height
		//glOrtho(-3.0, 3.0, -3.0 / aspect, 3.0 / aspect, 0.1, 100);
	//}
}

void Keyboard(unsigned char key, int x, int y)
{
	if (key == 'w') spectator.z -= speed;
	if (key == 's') spectator.z += speed;
	if (key == 'a') spectator.x -= speed;
	if (key == 'd') spectator.x += speed;
	if (key == 'q') spectator.y -= speed;
	if (key == 'e') spectator.y += speed;

	if (key == 'o') angle_x += angle_speed;
	if (key == 'l') angle_x += angle_speed;
	if (key == 'k') angle_y += angle_speed;
	if (key == ';') angle_y += angle_speed;

	cout << key << endl;
	glutPostRedisplay();
}

/* Main function: GLUT runs as a console application starting at main() */
int main(int argc, char** argv) {
	glutInit(&argc, argv);            // Initialize GLUT
	glutInitDisplayMode(GLUT_DOUBLE); // Enable double buffered mode
	glutInitWindowSize(640, 480);   // Set the window's initial width & height
	glutInitWindowPosition(200, 100); // Position the window's initial top-left corner
	glutCreateWindow("3� �������!");          // Create window with the given title
	glutDisplayFunc(display);       // Register callback handler for window re-paint event
	glutReshapeFunc(reshape);       // Register callback handler for window re-size event
	glutKeyboardFunc(Keyboard);
	initGL();                       // Our own OpenGL initialization
	glutMainLoop();                 // Enter the infinite event-processing loop
	return 0;
}