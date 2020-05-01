//Jeff Chastine
#include "GL/glut.h" 
#include <vector> 

using namespace std;

GLint Width = 512, Height = 512; 
GLubyte ColorR = 255, ColorG = 255, ColorB = 255;

struct type_point
{
	GLint x, y;
	type_point(GLint _x, GLint _y) { x = _x; y = _y; }
};

void changeViewPort(int w, int h)
{
	glViewport(0, 0, w, h);
}

vector<type_point> Points;
/* Функция вывода на экран */

void Display(void)
{
	glClearColor(0.5, 0.5, 0.5, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3ub(ColorR, ColorG, ColorB);
	glPointSize(6); 
	glEnable(GL_POINT_SMOOTH);
	glBegin(GL_POINTS);
	for (int i = 0; i < Points.size(); i++)
		glVertex2i(Points[i].x, Points[i].y);
	glEnd();
	glFinish();
}

/* Функция изменения размеров окна */
void Reshape(GLint w, GLint h)
{
	Width = w; Height = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/* Функция обработки сообщений от клавиатуры */
void Keyboard(unsigned char key, int x, int y) {
	int i, n = Points.size();
	/* Изменение RGB-компонент цвета точек */
	if (key == 'r') ColorR += 5;
	if (key == 'g') ColorG += 5;
	if (key == 'b') ColorB += 5;
	/* Изменение XY-кординат точек */
	if (key == 'w') for (i = 0; i < n; i++) Points[i].y += 9;
	if (key == 's') for (i = 0; i < n; i++) Points[i].y -= 9;
	if (key == 'a') for (i = 0; i < n; i++) Points[i].x -= 9;
	if (key == 'd') for (i = 0; i < n; i++) Points[i].x += 9;
	glutPostRedisplay();
}

/* Функция обработки сообщения от мыши */
void Mouse(int button, int state, int x, int y)
{
	/* клавиша была нажата, но не отпущена */
	if (state != GLUT_DOWN) return;
	/* новая точка по левому клику */
	if (button == GLUT_LEFT_BUTTON)
	{
		type_point p(x, Height - y);
		Points.push_back(p);
	}
	/* удаление последней точки по правому клику */
	if (button == GLUT_RIGHT_BUTTON) Points.pop_back();
	glutPostRedisplay();
}
/* Головная программа */
void main(int argc, char* argv[])

{
	glutInit(&argc, argv);
	// Set up some memory buffers for our display
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	// Set the window size
	glutInitWindowSize(Width, Height);
	glutCreateWindow("Рисуем точки");
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMainLoop();
}
