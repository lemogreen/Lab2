#include <windows.h>
#include <GL/glut.h>
#include <sstream>
#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

void glColorHex(string _hex)
{
	if (!(_hex.length() == 6 || (_hex.length() == 7 && _hex[0] == '#')))
		throw new exception("Illegal color format");

	int start = (_hex[0] == '#') ? 1 : 0;
	float color[3] = { 0, 0, 0 };
	for (int i = 0; i < 3; i++)
	{
		unsigned int x;
		stringstream ss;
		ss << std::hex << _hex.substr(start + i * 2, 2);
		ss >> x;
		if (x > 255)
			throw new exception("Too big color hex");
		color[i] = (float)x / 255;
	}
	glColor3f(color[0], color[1], color[2]);
}

struct type_point
{
	GLint x, y, z;
	type_point() { x = 0; y = 0; z = 0; }
	type_point(GLint _x, GLint _y, GLint _z) { x = _x; y = _y; z = _z; }
};

struct type_triangle
{
	type_point points[3];
};

struct ratio
{
	int percent;
	float kx, ky, kz;
	ratio(int _percent, float _kx, float _ky, float _kz) { percent = _percent; kx = _kx; ky = _ky; kz = _kz; }
};

type_point spectator = type_point(300, 200, 200);
float angle_x = 0;
float angle_y = 0;
int speed = 10;
int angle_speed = 1;

vector<type_triangle> model = {};
type_triangle triangle;
vector<type_point> tempTrajectory = {};
vector<ratio> tempRatios = {};
vector<type_point> trajectory = {};
vector<ratio> ratios = {};

bool karkas = true;

void ReadModel(string triangle_path, string trajectory_path, string params_path)
{
	ifstream triangle_in, trajectory_in, params_in;
	triangle_in.open(triangle_path);

	type_triangle start_triangle;
	for (int i = 0; i < 3; i++)
	{
		triangle_in >> start_triangle.points[i].x >> start_triangle.points[i].y >> start_triangle.points[i].z;
	}
	model.push_back(start_triangle);
	triangle = start_triangle;

	trajectory_in.open(trajectory_path);

	string line;
	int x, y, z;
	while (getline(trajectory_in, line))
	{
		sscanf_s(line.c_str(), "%d %d %d", &x, &y, &z);
		tempTrajectory.push_back(type_point(x, y, z));
	}

	int percent;
	float kx, ky, kz;
	params_in.open(params_path);
	while (getline(params_in, line))
	{
		sscanf_s(line.c_str(), "%d %f %f %f", &percent, &kx, &ky, &kz);
		tempRatios.push_back(ratio(percent, kx, ky, kz));
	}
}

void GenerateModel()
{
	double wholeDistance = 0;
	for (int i = 1; i < tempTrajectory.size(); i++)
	{
		wholeDistance += sqrt(
			(tempTrajectory[i].x - tempTrajectory[i - 1].x) * (tempTrajectory[i].x - tempTrajectory[i - 1].x) +
			(tempTrajectory[i].y - tempTrajectory[i - 1].y) * (tempTrajectory[i].y - tempTrajectory[i - 1].y) +
			(tempTrajectory[i].z - tempTrajectory[i - 1].z) * (tempTrajectory[i].z - tempTrajectory[i - 1].z)
		);
	}

	trajectory.push_back(tempTrajectory[0]);
	ratios.push_back(tempRatios[0]);

	int i = 1;
	int j = 1;
	int percentsPassed = 0;
	double passedDistance = 0;
	while (percentsPassed < 100)
	{
		double deltaX, deltaY, deltaZ;
		deltaX = tempTrajectory[i].x - trajectory.back().x;
		deltaY = tempTrajectory[i].y - trajectory.back().y;
		deltaZ = tempTrajectory[i].z - trajectory.back().z;
		passedDistance += sqrt(
			deltaX * deltaX +
			deltaY * deltaY +
			deltaZ * deltaZ
		);
		percentsPassed = passedDistance / wholeDistance * 100;

		while (percentsPassed > tempRatios[j].percent)
		{
			double k = (double)(tempRatios[j].percent - ratios.back().percent)
				/ (percentsPassed - ratios.back().percent);
			trajectory.push_back(type_point(
				trajectory.back().x + deltaX * k,
				trajectory.back().y + deltaY * k,
				trajectory.back().z + deltaZ * k
			));
			ratios.push_back(tempRatios[j]);
			j++;
		}

		trajectory.push_back(tempTrajectory[i]);
		float kx, ky, kz;
		kx = ratios.back().kx
			+ (tempRatios[j].kx - ratios.back().kx)
			* (percentsPassed - ratios.back().percent)
			/ (tempRatios[j].percent - ratios.back().percent);
		ky = ratios.back().ky
			+ (tempRatios[j].ky - ratios.back().ky)
			* (percentsPassed - ratios.back().percent)
			/ (tempRatios[j].percent - ratios.back().percent);
		kz = ratios.back().kz
			+ (tempRatios[j].kz - ratios.back().kz)
			* (percentsPassed - ratios.back().percent)
			/ (tempRatios[j].percent - ratios.back().percent);
		ratios.push_back(ratio(percentsPassed, kx, ky, kz));
		i++;

		/*if (percentsPassed < tempRatios[j].percent)
		{

		}
		else
		{

		}*/
	}
}

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
void Display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers

	glMatrixMode(GL_MODELVIEW);     // To operate on model-view matrix

	// Render a color-cube consisting of 6 quads with different colors
	glLoadIdentity();                 // Reset the model-view matrix

	gluLookAt(
		spectator.x, spectator.y, spectator.z,
		0, 0, 0,
		0, 100, 0
	);

	// Координатные оси XYZ
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

	glTranslatef(
		trajectory[0].x,
		trajectory[0].y,
		trajectory[0].z
	);
	glBegin((karkas) ? GL_LINE_LOOP : GL_TRIANGLES);
	glColorHex("#91edea");
	glVertex3f(triangle.points[0].x * ratios[0].kx, triangle.points[0].y * ratios[0].ky, triangle.points[0].z);
	glVertex3f(triangle.points[1].x * ratios[0].kx, triangle.points[1].y * ratios[0].ky, triangle.points[1].z);
	glVertex3f(triangle.points[2].x * ratios[0].kx, triangle.points[2].y * ratios[0].ky, triangle.points[2].z);
	glEnd();

	for (int i = 1; i < trajectory.size(); i++)
	{
		type_point delta = type_point(
			trajectory[i].x - trajectory[i - 1].x,
			trajectory[i].y - trajectory[i - 1].y,
			trajectory[i].z - trajectory[i - 1].z
		);

		glBegin((karkas) ? GL_LINE_LOOP : GL_QUADS);
		glVertex3f(triangle.points[0].x * ratios[i - 1].kx, triangle.points[0].y * ratios[i - 1].ky, triangle.points[0].z);
		glVertex3f(triangle.points[1].x * ratios[i - 1].kx, triangle.points[1].y * ratios[i - 1].ky, triangle.points[1].z);

		glVertex3f(
			triangle.points[1].x * ratios[i].kx + delta.x,
			triangle.points[1].y * ratios[i].ky + delta.y,
			triangle.points[1].z + delta.z
		);
		glVertex3f(
			triangle.points[0].x * ratios[i].kx + delta.x,
			triangle.points[0].y * ratios[i].ky + delta.y,
			triangle.points[0].z + delta.z
		);
		glEnd();

		glBegin((karkas) ? GL_LINE_LOOP : GL_QUADS);
		glVertex3f(triangle.points[0].x * ratios[i - 1].kx, triangle.points[0].y * ratios[i - 1].ky, triangle.points[0].z);
		glVertex3f(triangle.points[2].x * ratios[i - 1].kx, triangle.points[2].y * ratios[i - 1].ky, triangle.points[2].z);
		glVertex3f(
			triangle.points[2].x * ratios[i].kx + delta.x,
			triangle.points[2].y * ratios[i].ky + delta.y,
			triangle.points[2].z + delta.z
		);
		glVertex3f(
			triangle.points[0].x * ratios[i].kx + delta.x,
			triangle.points[0].y * ratios[i].ky + delta.y,
			triangle.points[0].z + delta.z
		);
		glEnd();

		glBegin((karkas) ? GL_LINE_LOOP : GL_QUADS);
		glVertex3f(triangle.points[1].x * ratios[i - 1].kx, triangle.points[1].y * ratios[i - 1].ky, triangle.points[1].z);
		glVertex3f(triangle.points[2].x * ratios[i - 1].kx, triangle.points[2].y * ratios[i - 1].ky, triangle.points[2].z);
		glVertex3f(
			triangle.points[2].x * ratios[i].kx + delta.x,
			triangle.points[2].y * ratios[i].ky + delta.y,
			triangle.points[2].z + delta.z
		);
		glVertex3f(
			triangle.points[1].x * ratios[i].kx + delta.x,
			triangle.points[1].y * ratios[i].ky + delta.y,
			triangle.points[1].z + delta.z
		);
		glEnd();

		glTranslatef(delta.x, delta.y, delta.z);
	}

	glBegin((karkas) ? GL_LINE_LOOP : GL_TRIANGLES);
	glVertex3f(triangle.points[0].x, triangle.points[0].y, triangle.points[0].z);
	glVertex3f(triangle.points[1].x, triangle.points[1].y, triangle.points[1].z);
	glVertex3f(triangle.points[2].x, triangle.points[2].y, triangle.points[2].z);
	glEnd();

	glutSwapBuffers();  // Swap the front and back frame buffers (double buffering)
}

/* Handler for window re-size event. Called back when the window first appears and
   whenever the window is re-sized with its new width and height */
void Reshape(GLsizei width, GLsizei height) {  // GLsizei for non-negative integer
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

	if (key == 'k') karkas = !karkas;

	cout << key << endl;
	glutPostRedisplay();
}

/* Main function: GLUT runs as a console application starting at main() */
int main(int argc, char** argv) {
	glutInit(&argc, argv);            // Initialize GLUT
	glutInitDisplayMode(GLUT_DOUBLE); // Enable double buffered mode
	glutInitWindowSize(640, 480);   // Set the window's initial width & height
	glutInitWindowPosition(200, 100); // Position the window's initial top-left corner
	glutCreateWindow("lab2");          // Create window with the given title
	glutDisplayFunc(Display);       // Register callback handler for window re-paint event
	glutReshapeFunc(Reshape);       // Register callback handler for window re-size event
	glutKeyboardFunc(Keyboard);
	initGL();                       // Our own OpenGL initialization

	ReadModel(
		"triangle.txt",
		"trajectory.txt",
		"params.txt"
	);
	GenerateModel();

	glutMainLoop();                 // Enter the infinite event-processing loop
	return 0;
}