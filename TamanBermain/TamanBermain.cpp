// terrain.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include <glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include "imageloader.h"
#include "vec3f.h"
#endif

static int spin = 0 , jungkat = 0, ayun = 0;
float angle = 0;
using namespace std;
GLuint texture[2];
float lastx, lasty;
GLint stencilBits;
/* Deklarasi untuk kamera pembangunan dan pengeditan
static int camposz = 150;
static int camposx = 0;
static int camposy = 50;
static int posz = 0;
static int posx = 0;
static int posy = 50;

*/
static int viewx = 50;
static int viewy = 100;
static int viewz = 200;
float rot = 0;


//train 2D
//class untuk terain 2D
class Terrain {
private:
	int w; //Width
	int l; //Length
	float** hs; //Heights
	Vec3f** normals;
	bool computedNormals; //Whether normals is up-to-date
public:
	Terrain(int w2, int l2) {
		w = w2;
		l = l2;

		hs = new float*[l];
		for (int i = 0; i < l; i++) {
			hs[i] = new float[w];
		}

		normals = new Vec3f*[l];
		for (int i = 0; i < l; i++) {
			normals[i] = new Vec3f[w];
		}

		computedNormals = false;
	}

	~Terrain() {
		for (int i = 0; i < l; i++) {
			delete[] hs[i];
		}
		delete[] hs;

		for (int i = 0; i < l; i++) {
			delete[] normals[i];
		}
		delete[] normals;
	}

	int width() {
		return w;
	}

	int length() {
		return l;
	}

	//Sets the height at (x, z) to y
	void setHeight(int x, int z, float y) {
		hs[z][x] = y;
		computedNormals = false;
	}

	//Returns the height at (x, z)
	float getHeight(int x, int z) {
		return hs[z][x];
	}

	//Computes the normals, if they haven't been computed yet
	void computeNormals() {
		if (computedNormals) {
			return;
		}

		//Compute the rough version of the normals
		Vec3f** normals2 = new Vec3f*[l];
		for (int i = 0; i < l; i++) {
			normals2[i] = new Vec3f[w];
		}

		for (int z = 0; z < l; z++) {
			for (int x = 0; x < w; x++) {
				Vec3f sum(0.0f, 0.0f, 0.0f);

				Vec3f out;
				if (z > 0) {
					out = Vec3f(0.0f, hs[z - 1][x] - hs[z][x], -1.0f);
				}
				Vec3f in;
				if (z < l - 1) {
					in = Vec3f(0.0f, hs[z + 1][x] - hs[z][x], 1.0f);
				}
				Vec3f left;
				if (x > 0) {
					left = Vec3f(-1.0f, hs[z][x - 1] - hs[z][x], 0.0f);
				}
				Vec3f right;
				if (x < w - 1) {
					right = Vec3f(1.0f, hs[z][x + 1] - hs[z][x], 0.0f);
				}

				if (x > 0 && z > 0) {
					sum += out.cross(left).normalize();
				}
				if (x > 0 && z < l - 1) {
					sum += left.cross(in).normalize();
				}
				if (x < w - 1 && z < l - 1) {
					sum += in.cross(right).normalize();
				}
				if (x < w - 1 && z > 0) {
					sum += right.cross(out).normalize();
				}

				normals2[z][x] = sum;
			}
		}

		//Smooth out the normals
		const float FALLOUT_RATIO = 0.5f;
		for (int z = 0; z < l; z++) {
			for (int x = 0; x < w; x++) {
				Vec3f sum = normals2[z][x];

				if (x > 0) {
					sum += normals2[z][x - 1] * FALLOUT_RATIO;
				}
				if (x < w - 1) {
					sum += normals2[z][x + 1] * FALLOUT_RATIO;
				}
				if (z > 0) {
					sum += normals2[z - 1][x] * FALLOUT_RATIO;
				}
				if (z < l - 1) {
					sum += normals2[z + 1][x] * FALLOUT_RATIO;
				}

				if (sum.magnitude() == 0) {
					sum = Vec3f(0.0f, 1.0f, 0.0f);
				}
				normals[z][x] = sum;
			}
		}

		for (int i = 0; i < l; i++) {
			delete[] normals2[i];
		}
		delete[] normals2;

		computedNormals = true;
	}

	//Returns the normal at (x, z)
	Vec3f getNormal(int x, int z) {
		if (!computedNormals) {
			computeNormals();
		}
		return normals[z][x];
	}
};
//end class


void initRendering() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
}
//Loads a terrain from a heightmap.  The heights of the terrain range from
//-height / 2 to height / 2.
//load terain di procedure inisialisasi
Terrain* loadTerrain(const char* filename, float height) {
	Image* image = loadBMP(filename);
	Terrain* t = new Terrain(image->width, image->height);
	for (int y = 0; y < image->height; y++) {
		for (int x = 0; x < image->width; x++) {
			unsigned char color = (unsigned char) image->pixels[3 * (y
					* image->width + x)];
			float h = height * ((color / 255.0f) - 0.5f);
			t->setHeight(x, y, h);
		}
	}

	delete image;
	t->computeNormals();
	return t;
}

float _angle = 60.0f;
//buat tipe data terain
Terrain* _terrain;
Terrain* _terrainTanah;
Terrain* _terrainAir;

const GLfloat light_ambient[] = { 0.0f, 0.3f, 0.1f, 1.0f };
const GLfloat light_diffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 1.0f, 1.0f, 1.0f, 1.0f };

const GLfloat light_ambient2[] = { 0.3f, 0.3f, 0.3f, 0.0f };
const GLfloat light_diffuse2[] = { 1.0f, 1.0f, 0.6f, 0.0f };

const GLfloat mat_ambient[] = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

void cleanup() {
	delete _terrain;
	delete _terrainTanah;
}

//untuk di display
void drawSceneTanah(Terrain *terrain, GLfloat r, GLfloat g, GLfloat b) {
	//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/*
	 glMatrixMode(GL_MODELVIEW);
	 glLoadIdentity();
	 glTranslatef(0.0f, 0.0f, -10.0f);
	 glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
	 glRotatef(-_angle, 0.0f, 1.0f, 0.0f);

	 GLfloat ambientColor[] = {0.4f, 0.4f, 0.4f, 1.0f};
	 glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

	 GLfloat lightColor0[] = {0.6f, 0.6f, 0.6f, 1.0f};
	 GLfloat lightPos0[] = {-0.5f, 0.8f, 0.1f, 0.0f};
	 glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
	 glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
	 */
	float scale = 300.0f / max(terrain->width() - 1, terrain->length() - 1);
	glScalef(scale, scale, scale);
	glTranslatef(-(float) (terrain->width() - 1) / 2, 0.0f,
			-(float) (terrain->length() - 1) / 2);

	glColor3f(r, g, b);
	for (int z = 0; z < terrain->length() - 1; z++) {
		//Makes OpenGL draw a triangle at every three consecutive vertices
		glBegin(GL_TRIANGLE_STRIP);
		for (int x = 0; x < terrain->width(); x++) {
			Vec3f normal = terrain->getNormal(x, z);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, terrain->getHeight(x, z), z);
			normal = terrain->getNormal(x, z + 1);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, terrain->getHeight(x, z + 1), z + 1);
		}
		glEnd();
	}

}
void alas()
{
glPushMatrix();
glBegin(GL_POLYGON);
glTexCoord2f(0.0, 0.0);
glVertex3f(1, 10, 0.0);
glTexCoord2f(1.0, 0.0);
glVertex3f(10, 10, 0.0);
glTexCoord2f(1.0, 1.0);
glVertex3f(10, 20, 0.0);
glTexCoord2f(0.0, 1.0);
glVertex3f(1, 20, 0.0);
glEnd();

glPopMatrix();
}
void jungkit() {
    //papan
glColor3f(0.5, 0.5, 1);
	glPushMatrix();
	glRotatef(jungkat, 0.0, 0.0, 1.0);
	glPushMatrix();		
	glScalef(5.5, 0.1, 1.0);	
	glutSolidCube(1.0);
	glPopMatrix();	

glColor3f(0, 0, 1);
	//Pegangan 1
	glPushMatrix();
	glScalef(0.1, 0.1, 1.0);
	glTranslatef(-17, 4, 0);
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glScalef(0.1, 0.5, 0.2);
	glTranslatef(-17, 0.3, 0);
	glutSolidCube(1.0);
	glPopMatrix();


	//Pegangan 2
	glPushMatrix();
	glScalef(0.1, 0.1, 1.0);
	glTranslatef(17, 4, 0);
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glScalef(0.1, 0.5, 0.2);
	glTranslatef(17, 0.3, 0);
	glutSolidCube(1.0);
	glPopMatrix();

	glPopMatrix();

	//papan tahanan
	glPushMatrix();
	glScalef(0.2, 1, 1.0);
	glTranslatef(0, -0.5, 0);
	glutSolidCube(1.0);
	glPopMatrix();

}

void pohon(){
	glColor3f(0.8, 0.5, 0.2);
	//<<<<<<<<<<<<<<<<<<<< Batang >>>>>>>>>>>>>>>>>>>>>>>
	glPushMatrix();
	glScalef(0.2, 2, 0.2);
	glutSolidSphere(1.0, 20, 16);
	glPopMatrix();
	//<<<<<<<<<<<<<<<<<<<< end Batang >>>>>>>>>>>>>>>>>>>>>>>

	glColor3f(0.0, 1.0, 0.0);
	//<<<<<<<<<<<<<<<<<<<< Daun >>>>>>>>>>>>>>>>>>>>>>>
	glPushMatrix();	
	glScalef(1, 1, 1.0);
	glTranslatef(0, 1, 0);
	glRotatef(270, 1, 0, 0);
	glutSolidCone(1,3,10,1);
	glPopMatrix();

	glPushMatrix();
	glScalef(1, 1, 1.0);
	glTranslatef(0, 2, 0);
	glRotatef(270, 1, 0, 0);
	glutSolidCone(1,3,10,1);	
	glPopMatrix();

	glPushMatrix();
	glScalef(1, 1, 1.0);
	glTranslatef(0, 3, 0);
	glRotatef(270, 1, 0, 0);
	glutSolidCone(1,3,10,1);	
	glPopMatrix();
	//<<<<<<<<<<<<<<<<<<<< end Daun >>>>>>>>>>>>>>>>>>>>>>>


}
void ayunan(){
	//ayunan 1
	glColor3f(1, 1, 1);
	glPushMatrix();
	glTranslatef(0, 2.5, 0);
	glRotatef(ayun, 0.0, 0.0, 1.0);
	
	glPushMatrix();	
	glTranslatef(0, -1.7, 1);
	glScalef(0.02, 3.5, 0.02);
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, -1.7, 2);
	glScalef(0.02, 3.5, 0.02);	
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, -3.4, 1.5);
	glScalef(0.7, 0.1, 1);	
	glutSolidCube(1.0);
	glPopMatrix();
	glPopMatrix();

	//ayunan 2
	glColor3f(1, 1, 1);
	glPushMatrix();
	glTranslatef(0, 2.5, 0);
	glRotatef(ayun, 0.0, 0.0, 1.0);
	
	glPushMatrix();	
	glTranslatef(0, -1.7, 3);
	glScalef(0.02, 3.5, 0.02);
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, -1.7, 4);
	glScalef(0.02, 3.5, 0.02);	
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, -3.4, 3.5);
	glScalef(0.7, 0.1, 1);	
	glutSolidCube(1.0);
	glPopMatrix();
	glPopMatrix();


	//<<<<<<<<<<<<<<<<<<<< Tiang>>>>>>>>>>>>>>>>>>>>>>>
	//glRotatef(RotateX, 0.0, 1.0, 0);
	
	glColor3f(1, 0.8, 1);
	glPushMatrix();
	glTranslatef(0, 2.6, 2.2);
	glScalef(0.1, 0.1, 4.4);
	glutSolidCube(1.0);
	glPopMatrix();

	//tiang 1
	glPushMatrix();
	glRotatef(20, 0.0, 0.0, 1.0);
	glTranslatef(0.9, 0, 0);	
	glScalef(0.1, 5, 0.1);	
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glRotatef(340, 0.0, 0.0, 1.0);
	glTranslatef(-0.9, 0, 0);	
	glScalef(0.1, 5, 0.1);	
	glutSolidCube(1.0);
	glPopMatrix();

	//tiang 2
	glPushMatrix();
	glRotatef(20, 0.0, 0.0, 1.0);
	glTranslatef(0.9, 0, 4.5);	
	glScalef(0.1, 5, 0.1);	
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glRotatef(340, 0.0, 0.0, 1.0);
	glTranslatef(-0.9, 0, 4.5);	
	glScalef(0.1, 5, 0.1);	
	glutSolidCube(1.0);
	glPopMatrix();
	//<<<<<<<<<<<<<<<<<<<< end Tiang>>>>>>>>>>>>>>>>>>>>>>>
}
void pagar()
{
	//pagar bawah
	glColor3f(0.5, 0.0, 0.0);
	glPushMatrix();
	glTranslatef(0, 0, 0);
	glScalef(19.9, 0.2, 0.03);	
	glutSolidCube(1.0);
	glPopMatrix();

	//pagar ataas
	glPushMatrix();
	glTranslatef(0, 0.3, 0);
	glScalef(19.9, 0.2, 0.03);	
	glutSolidCube(1.0);
	glPopMatrix();

	//tiang tengah
	glPushMatrix();
	glScalef(0.2, 1, 0.1);
	glTranslatef(0, 0.0, 0);
	glutSolidCube(1.0);
	glPopMatrix();

	//tiang kanan
	glPushMatrix();
	glTranslatef(10, 0.0, 0);
	glScalef(0.2, 1, 0.2);	
	glutSolidCube(1.0);
	glPopMatrix();

	//tiang kiri
	glPushMatrix();
	glTranslatef(-10, 0.0, 0);
	glScalef(0.2, 1, 0.2);	
	glutSolidCube(1.0);
	glPopMatrix();
}

void lampu()
{
glColor3f(1,1,1);
   //lampu
   glPushMatrix();
   glTranslatef(0,18,0);
   glScalef(5,5,5);
   glutSolidSphere(1.0, 20, 16);
   glPopMatrix();
   //batang
 
   glPushMatrix();
   glTranslatef(0,7,0);
   glScalef(0.8,14,0.8);
   glutSolidSphere(1.0, 20, 16);
   glPopMatrix();
}




void display(void) {
	glClearStencil(0); //clear the stencil buffer
	glClearDepth(1.0f);
	glClearColor(0.0, 0.6, 0.8, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); //clear the buffers
	glLoadIdentity();
	// untuk kamera pembangunan dan pengeditan gluLookAt(camposx, camposy, camposz, posx, posy, posz, 0.0, 1.0, 0);
	gluLookAt(viewx, viewy, viewz, 0.0, 0.0, 5.0, 0.0, 1.0, 0.0);

		
	
	
	// lampu 1
	glPushMatrix();
	glTranslatef(110,0,-18);
	lampu();
	glPopMatrix();

	//lampu 2
	glPushMatrix();
	glTranslatef(110,0, 14);
	lampu();
	glPopMatrix();
	
	//Jungkat Jungkit
	glPushMatrix();	
	glRotatef(330, 0.0, 1.0, 0.0);
	glTranslatef(40, 7, -60);
	glScalef(10, 10, 10);
	jungkit();
	glPopMatrix();

	//ayunan
	glPushMatrix();	
	glTranslatef(-60, 8, 15);
	glScalef(10, 10, 10);
	glRotatef(357, 0, 1, 0);
	ayunan();
	glPopMatrix();

	

	//pagar samping depan
	glPushMatrix();
	glTranslatef(3, 0, 70);
	glScalef(10, 10, 10);
	pagar();
	glPopMatrix();

	//pagar samping belakang
	glPushMatrix();
	glTranslatef(3, 0, -70);
	glScalef(10, 10, 10);
	pagar();
	glPopMatrix();

	//pagar belakang
	glPushMatrix();
	glTranslatef(-97, 0, 0);
	glScalef(10, 10, 7);
	glRotatef(90, 0, 1, 0);
	pagar();
	glPopMatrix();

	//pagar depan depan
	glPushMatrix();
	glTranslatef(103, 0, -45);
	glScalef(10, 10, 2.6);
	glRotatef(90, 0, 1, 0);
	pagar();
	glPopMatrix();

	//pagar depan belakang
	glPushMatrix();
	glTranslatef(103, 0, 45);
	glScalef(10, 10, 2.6);
	glRotatef(90, 0, 1, 0);
	pagar();
	glPopMatrix();

	



	//<<<<<<<<<<<<<<<<<<<< pohon >>>>>>>>>>>>>>>>>>>>>>>
	glPushMatrix();
	glColor3f(1, 0, 0);
	glTranslatef(-110, 0, 85);
	glScalef(10, 10, 10);
	pohon();
	glPopMatrix();

	glPushMatrix();
	glColor3f(1, 0, 0);
	glTranslatef(110, 0, -80);
	glScalef(10, 10, 10);	
	pohon();
	glPopMatrix();

	glPushMatrix();
	glColor3f(1, 0, 0);
	glTranslatef(-80, 0, -40);
	glScalef(17, 14, 17);	
	pohon();
	glPopMatrix();


	//<<<<<<<<<<<<<<<<<<<< end pohon >>>>>>>>>>>>>>>>>>>>>>
	glPushMatrix();	
	//glBindTexture(GL_TEXTURE_3D, texture[0]);
	drawSceneTanah(_terrain, 0.3f, 0.9f, 0.0f);
	glPopMatrix();

	glPushMatrix();
	//glBindTexture(GL_TEXTURE_3D, texture[0]);
	drawSceneTanah(_terrainTanah,0.8, 0.5, 0.2);//tanah
	glPopMatrix();

	glPushMatrix();
	//glBindTexture(GL_TEXTURE_3D, texture[0]);
	drawSceneTanah(_terrainAir, 0.0f, 0.2f, 0.5f);//air
	glPopMatrix();


	glutSwapBuffers();
	glFlush();
	rot++;
	angle++;

}

void init(void) {
glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glDepthFunc(GL_LESS);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_CULL_FACE);


	_terrain = loadTerrain("heightmap.bmp", 20);
	_terrainTanah = loadTerrain("heightmapTanah.bmp", 20);
	_terrainAir = loadTerrain("heightmapAir.bmp", 20);

	//binding texture
    
}

static void specialKeyboard(int key, int x, int y) {
	switch (key) {
		case GLUT_KEY_HOME:
		viewy = viewy + 15;
		break;
	case GLUT_KEY_END:
		viewy = viewy-15;
		break;
	case GLUT_KEY_UP:
		viewz = viewz - 15;
		break;
	case GLUT_KEY_DOWN:
		viewz = viewz +15;
		break;

	case GLUT_KEY_RIGHT:
		viewx = viewx + 15;
		break;
	case GLUT_KEY_LEFT:
		viewx = viewx - 15;
		break;

		case GLUT_KEY_F1: {
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	}
		;
		break;
	case GLUT_KEY_F2: {
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient2);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse2);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	}
/* Kamera untuk pembangunan dan pengeditan
	case GLUT_KEY_HOME:	
		camposy = camposy+15;
		posy = posy + 15;	
		break;
	case GLUT_KEY_END:
		camposy = camposy-15;
		posy = posy - 15;	
		break;
	case GLUT_KEY_RIGHT:
		camposx = camposx+15;
		posx = posx + 15;		
		break;
	case GLUT_KEY_LEFT:
		camposx = camposx-15;
		posx = posx - 15;
		break;

	case GLUT_KEY_UP:
		camposz = camposz -15;	
		posz = posz - 15;
		break;
	case GLUT_KEY_DOWN:
		camposz = camposz+15;
		posz = posz + 15;
		break;

	case GLUT_KEY_F1: {
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	}
		;
		break;
	case GLUT_KEY_F2: {
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient2);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse2);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	}
		;
		break;
	default:
		break;
*/		
	}

}

void keyboard(unsigned char key, int x, int y) {	
	
	if (key == 'j') {		
		if (jungkat < 20)
			{
				jungkat = jungkat + 2;
			}
			else if(jungkat == 20)
				{
					jungkat = 380;
				}		
				else if (jungkat <=380 && jungkat >340)
					{
						jungkat = jungkat - 2;
					}
					else if (jungkat ==340)
						{
							jungkat = -20;
						}	
	}

	if (key == 'y') {
		if (ayun < 20)
			{
				ayun = ayun + 2;
			}
			else if(ayun == 20)
				{
					ayun = 380;
				}		
				else if (ayun <=380 && ayun >340)
					{
						ayun = ayun - 2;
					}
					else if (ayun ==340)
						{
							ayun = -20;
						}
		
		
				
	}
	if (key == 'k') 
	{
		spin = spin + 5;
		if (spin == 360)
			{
				spin = 0;
			}
	}
	
	
}

void reshape(int w, int h) {
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (GLfloat) w / (GLfloat) h, 0.1, 1000.0);
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL | GLUT_DEPTH); //add a stencil buffer to the window
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Taman Bermain");
	init();
	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(specialKeyboard);

	glutKeyboardFunc(keyboard);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
	glColorMaterial(GL_FRONT, GL_DIFFUSE);

	glutMainLoop();
	return 0;
}
