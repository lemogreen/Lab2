#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <GL/glut.h>
#include <sstream>
#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

class Vector3f
{
public:
	float x, y, z;

	Vector3f() {};							//конструктор по умолчанию
	Vector3f(float _x, float _y, float _z)	//конструктор
	{
		x = _x;
		y = _y;
		z = _z;
	}

	//Переопределение оператора +
	Vector3f operator + (Vector3f _Vector)
	{
		return Vector3f(_Vector.x + x, _Vector.y + y, _Vector.z + z);
	}
	//Переопределение оператора -
	Vector3f operator - (Vector3f _Vector)
	{
		return Vector3f(x - _Vector.x, y - _Vector.y, z - _Vector.z);
	}

	//Переопределение оператора *
	Vector3f operator * (float num)
	{
		return Vector3f(x * num, y * num, z * num);
	}

	//Переопределение оператора /
	Vector3f operator / (float num)
	{
		return Vector3f(x / num, y / num, z / num);
	}
};


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

int light_sample = 1;
bool isTexturingEnabled = true;
bool isSkeletonViewEnabled = true;
bool isNormalsShowing = false;
bool isPerspectiveViewEnabled = true;

//Вычисление нормали двух векторов
Vector3f Cross(Vector3f _Vec1, Vector3f _Vec2)
{
	Vector3f _Normal;

	//вычисление векторного произведения
	_Normal.x = ((_Vec1.y * _Vec2.z) - (_Vec1.z * _Vec2.y));
	_Normal.y = ((_Vec1.z * _Vec2.x) - (_Vec1.x * _Vec2.z));
	_Normal.z = ((_Vec1.x * _Vec2.y) - (_Vec1.y * _Vec2.x));

	return _Normal;
}

//Вычисление нормы вектора
float Norm(Vector3f _Vec)
{
	return (float)sqrt((_Vec.x * _Vec.x) + (_Vec.y * _Vec.y) + (_Vec.z * _Vec.z));
}

//Нормализация вектора
Vector3f Normalize(Vector3f _Vec)
{
	//Вычислить норму вектора
	float norm = Norm(_Vec);

	//нормализовать вектор
	_Vec = _Vec / norm;

	return _Vec;
}

//Вычисление скалярного произведения
float scalar(Vector3f _Vec1, Vector3f _Vec2)
{
	return _Vec1.x * _Vec2.x + _Vec1.y * _Vec2.y + _Vec1.z * _Vec2.z;
}

//Вектор между двумя точками
Vector3f Vector(Vector3f _Point1, Vector3f _Point2)
{
	Vector3f _Vector;

	_Vector.x = _Point1.x - _Point2.x;
	_Vector.y = _Point1.y - _Point2.y;
	_Vector.z = _Point1.z - _Point2.z;
	return _Vector;
}

//Вычисление нормали полигона
Vector3f Normal(Vector3f _1, Vector3f _2, Vector3f _3)
{
	Vector3f _Vector1 = Vector(_2, _1);
	Vector3f _Vector2 = Vector(_3, _2);
	Vector3f _Normal = Cross(_Vector1, _Vector2);
	_Normal = Normalize(_Normal);
	return _Normal;
}

void CreateLight()
{
	GLfloat ambience[4] = { 0.1f, 0.1f, 0.1f, 1.0 };	//Цвет мирового света
	GLfloat material_diffuse[] = { 0.15, 0.15, 0.15, 1 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
	glEnable(GL_COLOR_MATERIAL);
	if (light_sample == 1)
	{
		//направленный источник света
		//находится в бесконечности и свет от него распространяется в заданном направлении
		GLfloat light0_diffuse[] = { 0.4, 0.7, 0.2 };

		GLfloat light0_direction[] = { 0.0, 0.0, 1.0, 0.0 };
		glEnable(GL_LIGHT0);
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambience);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
		glLightfv(GL_LIGHT0, GL_POSITION, light0_direction);
	}
	if (light_sample == 2)
	{
		//точечный источник света
		//убывание интенсивности с расстоянием
		//отключено (по умолчанию)
		GLfloat light1_diffuse[] = { 0.4, 0.7, 0.2 };

		GLfloat light1_position[] = { 0.0, 0.0, 1.0, 1.0 };
		glEnable(GL_LIGHT1);
		glLightfv(GL_LIGHT1, GL_AMBIENT, ambience);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
		glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
	}
	if (light_sample == 3)
	{
		//точечный источник света
		//убывание интенсивности с расстоянием
		GLfloat light2_diffuse[] = { 0.4, 0.7, 0.2 };
		GLfloat light2_position[] = { 1.0, 1.0, 1.0, 1.0 };
		glEnable(GL_LIGHT2);
		glLightfv(GL_LIGHT2, GL_AMBIENT, ambience);
		glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_diffuse);
		glLightfv(GL_LIGHT2, GL_POSITION, light2_position);
		glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 0);
		glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.04);
		glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.04);
	}
	if (light_sample == 4)
	{
		//прожектор
		//убывание интенсивности с расстоянием
		//отключено (по умолчанию)
		GLfloat light3_diffuse[] = { 0.4, 0.7, 0.2 };
		GLfloat light3_position[] = { 0.0, 0.0, 1.0, 1.0 };
		GLfloat light3_spot_direction[] = { 0.0, 0.0, -1.0 };
		glEnable(GL_LIGHT3);
		glLightfv(GL_LIGHT3, GL_AMBIENT, ambience);
		glLightfv(GL_LIGHT3, GL_DIFFUSE, light3_diffuse);
		glLightfv(GL_LIGHT3, GL_POSITION, light3_position);
		glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 50);
		glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, light3_spot_direction);
	}
	if (light_sample == 5)
	{
		//прожектор
		//включен рассчет убывания интенсивности для прожектора
		GLfloat light4_diffuse[] = { 0.4, 0.7, 0.2 };
		GLfloat light4_position[] = { 0.0, 0.0, 1.0, 1.0 };
		GLfloat light4_spot_direction[] = { 0.0, 0.0, -1.0 };
		glEnable(GL_LIGHT4);
		glLightfv(GL_LIGHT4, GL_AMBIENT, ambience);
		glLightfv(GL_LIGHT4, GL_DIFFUSE, light4_diffuse);
		glLightfv(GL_LIGHT4, GL_POSITION, light4_position);
		glLightf(GL_LIGHT4, GL_SPOT_CUTOFF, 50);
		glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, light4_spot_direction);
		glLightf(GL_LIGHT4, GL_SPOT_EXPONENT, 15.0);
	}
}

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

GLuint LoadTexture(const char* filename)
{
	GLuint texture;
	int width, height;
	unsigned char* data;

	FILE* file;
	file = fopen(filename, "rb");

	if (file == NULL) return 0;
	width = 1024;
	height = 512;
	data = (unsigned char*)malloc(width * height * 3);
	//int size = fseek(file,);
	fread(data, width * height * 3, 1, file);
	fclose(file);

	for (int i = 0; i < width * height; ++i)
	{
		int index = i * 3;
		unsigned char B, R;
		B = data[index];
		R = data[index + 2];

		data[index] = R;
		data[index + 2] = B;
	}

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
	free(data);

	return texture;
}

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
	}
}

void calculateNormals(int i) {
	type_point delta = type_point(
		trajectory[i].x - trajectory[i - 1].x,
		trajectory[i].y - trajectory[i - 1].y,
		trajectory[i].z - trajectory[i - 1].z
	);

	Vector3f vec1 = Vector3f(triangle.points[0].x * ratios[i - 1].kx, triangle.points[0].y * ratios[i - 1].ky, triangle.points[0].z);
	Vector3f vec2 = Vector3f(triangle.points[1].x * ratios[i - 1].kx, triangle.points[1].y * ratios[i - 1].ky, triangle.points[1].z);
	Vector3f vec3 = Vector3f(
		triangle.points[1].x * ratios[i].kx + delta.x,
		triangle.points[1].y * ratios[i].ky + delta.y,
		triangle.points[1].z + delta.z
	);
	Vector3f norm = Normal(vec3, vec2, vec1);

	type_point center;
	center.x = (triangle.points[0].x * ratios[i - 1].kx + triangle.points[1].x * ratios[i - 1].kx
		+ triangle.points[1].x * ratios[i].kx + delta.x + triangle.points[0].x * ratios[i].kx + delta.x) / 4;
	center.y = (triangle.points[0].y * ratios[i - 1].ky + triangle.points[1].y * ratios[i - 1].ky
		+ triangle.points[1].y * ratios[i].ky + delta.y + triangle.points[0].y * ratios[i].ky + delta.y) / 4;
	center.z = (triangle.points[0].z * ratios[i - 1].kz + triangle.points[1].z * ratios[i - 1].kz
		+ triangle.points[1].z * ratios[i].kz + delta.z + triangle.points[0].z * ratios[i].kz + delta.z) / 4;

	glBegin(GL_LINES);
	glVertex3f(center.x, center.y, center.z);
	glVertex3f(center.x + norm.x * 10, center.y + norm.y * 10, center.z + norm.z * 10);
	glEnd();

	vec1 = Vector3f(triangle.points[0].x * ratios[i - 1].kx, triangle.points[0].y * ratios[i - 1].ky, triangle.points[0].z);
	vec2 = Vector3f(triangle.points[2].x * ratios[i - 1].kx, triangle.points[2].y * ratios[i - 1].ky, triangle.points[2].z);
	vec3 = Vector3f(
		triangle.points[2].x * ratios[i].kx + delta.x,
		triangle.points[2].y * ratios[i].ky + delta.y,
		triangle.points[2].z + delta.z
	);
	norm = Normal(vec1, vec2, vec3);

	center.x = (triangle.points[0].x * ratios[i - 1].kx + triangle.points[2].x * ratios[i - 1].kx
		+ triangle.points[2].x * ratios[i].kx + delta.x + triangle.points[0].x * ratios[i].kx + delta.x) / 4;
	center.y = (triangle.points[0].y * ratios[i - 1].ky + triangle.points[2].y * ratios[i - 1].ky
		+ triangle.points[2].y * ratios[i].ky + delta.y + triangle.points[0].y * ratios[i].ky + delta.y) / 4;
	center.z = (triangle.points[0].z * ratios[i - 1].kz + triangle.points[2].z * ratios[i - 1].kz
		+ triangle.points[2].z * ratios[i].kz + delta.z + triangle.points[0].z * ratios[i].kz + delta.z) / 4;

	glBegin(GL_LINES);
	glVertex3f(center.x, center.y, center.z);
	glVertex3f(center.x + norm.x * 10, center.y + norm.y * 10, center.z + norm.z * 10);
	glEnd();

	vec1 = Vector3f(triangle.points[2].x * ratios[i - 1].kx, triangle.points[2].y * ratios[i - 1].ky, triangle.points[2].z);
	vec2 = Vector3f(triangle.points[1].x * ratios[i - 1].kx, triangle.points[1].y * ratios[i - 1].ky, triangle.points[1].z);
	vec3 = Vector3f(
		triangle.points[1].x * ratios[i].kx + delta.x,
		triangle.points[1].y * ratios[i].ky + delta.y,
		triangle.points[1].z + delta.z
	);
	norm = Normal(vec1, vec2, vec3);

	center.x = (triangle.points[2].x * ratios[i - 1].kx + triangle.points[1].x * ratios[i - 1].kx
		+ triangle.points[1].x * ratios[i].kx + delta.x + triangle.points[2].x * ratios[i].kx + delta.x) / 4;
	center.y = (triangle.points[2].y * ratios[i - 1].ky + triangle.points[1].y * ratios[i - 1].ky
		+ triangle.points[1].y * ratios[i].ky + delta.y + triangle.points[2].y * ratios[i].ky + delta.y) / 4;
	center.z = (triangle.points[2].z * ratios[i - 1].kz + triangle.points[1].z * ratios[i - 1].kz
		+ triangle.points[1].z * ratios[i].kz + delta.z + triangle.points[2].z * ratios[i].kz + delta.z) / 4;

	glBegin(GL_LINES);
	glVertex3f(center.x, center.y, center.z);
	glVertex3f(center.x + norm.x * 10, center.y + norm.y * 10, center.z + norm.z * 10);
	glEnd();
}

/* Initialize OpenGL Graphics */
void initGL() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
	glClearDepth(1.0f);                   // Set background depth to farthest
	glEnable(GL_DEPTH_TEST);   // Enable depth testing for z-culling
	LoadTexture("1.bmp");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	//умножение
	glDepthFunc(GL_LEQUAL);    // Set the type of depth-test
	glShadeModel(GL_SMOOTH);   // Enable smooth shading
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Nice perspective corrections
}

/* Handler for window-repaint event. Called back when the window first appears and
   whenever the window needs to be re-painted. */
void Display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers
	glEnable(GL_LIGHTING);
	if (isTexturingEnabled) glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_MODELVIEW);     // To operate on model-view matrix

	// Render a color-cube consisting of 6 quads with different colors
	glLoadIdentity();                 // Reset the model-view matrix

	gluLookAt(
		spectator.x, spectator.y, spectator.z,
		0, 50, 0,
		0, 100, 0
	);

	CreateLight();

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

	glBegin((isSkeletonViewEnabled) ? GL_LINE_LOOP : GL_TRIANGLES);
	glColorHex("#91edea");
	glTexCoord2f(0, .1); glVertex3f(triangle.points[0].x * ratios[0].kx, triangle.points[0].y * ratios[0].ky, triangle.points[0].z);
	glTexCoord2f(.05, .1); glVertex3f(triangle.points[1].x * ratios[0].kx, triangle.points[1].y * ratios[0].ky, triangle.points[1].z);
	glTexCoord2f(.1, 0); glVertex3f(triangle.points[2].x * ratios[0].kx, triangle.points[2].y * ratios[0].ky, triangle.points[2].z);
	glEnd();

	if (isNormalsShowing)
	{
		Vector3f vec1 = Vector3f(triangle.points[0].x * ratios[0].kx, triangle.points[0].y * ratios[0].ky, triangle.points[0].z);
		Vector3f vec2 = Vector3f(triangle.points[1].x * ratios[0].kx, triangle.points[1].y * ratios[0].ky, triangle.points[1].z);
		Vector3f vec3 = Vector3f(triangle.points[2].x * ratios[0].kx, triangle.points[2].y * ratios[0].ky, triangle.points[2].z);
		Vector3f norm = Normal(vec1, vec2, vec3);

		type_point temp;
		temp.x = (triangle.points[0].x + triangle.points[1].x + triangle.points[2].x) / 3;
		temp.y = (triangle.points[0].y + triangle.points[1].y + triangle.points[2].y) / 3;
		temp.z = (triangle.points[0].z + triangle.points[1].z + triangle.points[2].z) / 3;

		glBegin(GL_LINES);
		glVertex3f(temp.x, temp.y, temp.z);
		glVertex3f(temp.x + norm.x * 10, temp.y + norm.y * 10, temp.z + norm.z * 10);
		glEnd();
	}

	for (int i = 1; i < trajectory.size(); i++)
	{
		type_point delta = type_point(
			trajectory[i].x - trajectory[i - 1].x,
			trajectory[i].y - trajectory[i - 1].y,
			trajectory[i].z - trajectory[i - 1].z
		);

		glBegin((isSkeletonViewEnabled) ? GL_LINE_LOOP : GL_QUADS);
		glTexCoord2f(0, .1); glVertex3f(triangle.points[0].x * ratios[i - 1].kx, triangle.points[0].y * ratios[i - 1].ky, triangle.points[0].z);
		glTexCoord2f(.1, .1); glVertex3f(triangle.points[1].x * ratios[i - 1].kx, triangle.points[1].y * ratios[i - 1].ky, triangle.points[1].z);

		glTexCoord2f(.1, 0); glVertex3f(
			triangle.points[1].x * ratios[i].kx + delta.x,
			triangle.points[1].y * ratios[i].ky + delta.y,
			triangle.points[1].z + delta.z
		);
		glTexCoord2f(0, 0); glVertex3f(
			triangle.points[0].x * ratios[i].kx + delta.x,
			triangle.points[0].y * ratios[i].ky + delta.y,
			triangle.points[0].z + delta.z
		);
		glEnd();

		glBegin((isSkeletonViewEnabled) ? GL_LINE_LOOP : GL_QUADS);
		glTexCoord2f(0, .1); glVertex3f(triangle.points[0].x * ratios[i - 1].kx, triangle.points[0].y * ratios[i - 1].ky, triangle.points[0].z);
		glTexCoord2f(.1, .1); glVertex3f(triangle.points[2].x * ratios[i - 1].kx, triangle.points[2].y * ratios[i - 1].ky, triangle.points[2].z);
		glTexCoord2f(.1, 0); glVertex3f(
			triangle.points[2].x * ratios[i].kx + delta.x,
			triangle.points[2].y * ratios[i].ky + delta.y,
			triangle.points[2].z + delta.z
		);
		glTexCoord2f(0, 0); glVertex3f(
			triangle.points[0].x * ratios[i].kx + delta.x,
			triangle.points[0].y * ratios[i].ky + delta.y,
			triangle.points[0].z + delta.z
		);
		glEnd();

		glBegin((isSkeletonViewEnabled) ? GL_LINE_LOOP : GL_QUADS);
		glTexCoord2f(0, .1); glVertex3f(triangle.points[1].x * ratios[i - 1].kx, triangle.points[1].y * ratios[i - 1].ky, triangle.points[1].z);
		glTexCoord2f(.1, .1); glVertex3f(triangle.points[2].x * ratios[i - 1].kx, triangle.points[2].y * ratios[i - 1].ky, triangle.points[2].z);
		glTexCoord2f(.1, 0); glVertex3f(
			triangle.points[2].x * ratios[i].kx + delta.x,
			triangle.points[2].y * ratios[i].ky + delta.y,
			triangle.points[2].z + delta.z
		);
		glTexCoord2f(0, 0); glVertex3f(
			triangle.points[1].x * ratios[i].kx + delta.x,
			triangle.points[1].y * ratios[i].ky + delta.y,
			triangle.points[1].z + delta.z
		);
		glEnd();

		if (isNormalsShowing)
			calculateNormals(i);

		glTranslatef(delta.x, delta.y, delta.z);
	}

	if (isNormalsShowing)
	{
		Vector3f vec1 = Vector3f(triangle.points[0].x * ratios[0].kx, triangle.points[0].y * ratios[0].ky, triangle.points[0].z);
		Vector3f vec2 = Vector3f(triangle.points[1].x * ratios[0].kx, triangle.points[1].y * ratios[0].ky, triangle.points[1].z);
		Vector3f vec3 = Vector3f(triangle.points[2].x * ratios[0].kx, triangle.points[2].y * ratios[0].ky, triangle.points[2].z);
		Vector3f norm = Normal(vec1, vec2, vec3);

		type_point temp;
		temp.x = (triangle.points[0].x + triangle.points[1].x + triangle.points[2].x) / 3;
		temp.y = (triangle.points[0].y + triangle.points[1].y + triangle.points[2].y) / 3;
		temp.z = (triangle.points[0].z + triangle.points[1].z + triangle.points[2].z) / 3;

		glBegin(GL_LINES);
		glVertex3f(temp.x, temp.y, temp.z);
		glVertex3f(temp.x + norm.x * 10, temp.y + norm.y * 10, -(temp.z + norm.z * 10));
		glEnd();
	}

	glBegin((isSkeletonViewEnabled) ? GL_LINE_LOOP : GL_TRIANGLES);
	glTexCoord2f(0, .1); glVertex3f(triangle.points[0].x, triangle.points[0].y, triangle.points[0].z);
	glTexCoord2f(.05, .1); glVertex3f(triangle.points[1].x, triangle.points[1].y, triangle.points[1].z);
	glTexCoord2f(.1, 0); glVertex3f(triangle.points[2].x, triangle.points[2].y, triangle.points[2].z);
	glEnd();

	//vec1 = Vector3f(triangle.points[0].x, triangle.points[0].y, triangle.points[0].z);
	//vec2 = Vector3f(triangle.points[1].x, triangle.points[1].y, triangle.points[1].z);
	//vec3 = Vector3f(triangle.points[2].x, triangle.points[2].y, triangle.points[2].z);
	//norm = Normal(vec1, vec2, vec3);

	//glBegin(GL_LINES);
	//glVertex3f(vec1.x, vec1.y, vec1.z);
	//glVertex3f(vec1.x + norm.x * 10, vec1.y + norm.y * 10, vec1.z + norm.z * 10);
	//glEnd();

	//отключить все источники
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHT2);
	glDisable(GL_LIGHT3);
	glDisable(GL_LIGHT4);

	//отключить текстурирование
	glDisable(GL_TEXTURE_2D);

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

	if (isPerspectiveViewEnabled)
		gluPerspective(45.0f, aspect, 0.1f, 5000);
	else
		glOrtho(-width / 2, width / 2, -height / 2, height / 2, -5000, 5000);
}

void Keyboard(unsigned char key, int x, int y)
{
	if (key == 'w') spectator.z -= speed;
	if (key == 's') spectator.z += speed;
	if (key == 'a') spectator.x -= speed;
	if (key == 'd') spectator.x += speed;
	if (key == 'q') spectator.y -= speed;
	if (key == 'e') spectator.y += speed;

	if (key == 'k') isSkeletonViewEnabled = !isSkeletonViewEnabled;
	if (key == 'p')
	{
		isPerspectiveViewEnabled = !isPerspectiveViewEnabled;
		Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	}

	if (key == 't') isTexturingEnabled = !isTexturingEnabled;
	if (key == 'n') isNormalsShowing = !isNormalsShowing;

	if (key == '1') light_sample = 1;
	if (key == '2') light_sample = 2;
	if (key == '3') light_sample = 3;
	if (key == '4') light_sample = 4;
	if (key == '5') light_sample = 5;

	cout << key << endl;
	glutPostRedisplay();
}

/* Main function: GLUT runs as a console application starting at main() */
int main(int argc, char** argv) {
	glutInit(&argc, argv);            // Initialize GLUT
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA); // enable depth buffer and double buffered mode
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


