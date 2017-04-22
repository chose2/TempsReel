/*Gabriel Gagné
Danny Groleau
*/
/*
    Usage:
    [-help shows this help]
    [-yuv sets yuv420 color mode]
    [-w width] [-h height] \n[-br brightness_val(0,100)]\n[-sh  sharpness_val (-100 to 100)]
    [-co contrast_val (-100 to 100)]
    [-sa saturation_val (-100 to 100)]
    [-iso ISO_val  (100 to 800)]
    [-vs turns on video stabilisation]\n[-ec exposure_compensation_value(-10,10)]
    [-ss shutter_speed (value in microsecs (max 330000)]
    [-ec exposure_compensation_value(-10,10)]
    [-nframes val: number of frames captured (100 default). 0 == Infinite lopp]
    [-awb_r val:(0,8):set the value for the red component of white balance
    [-awb_g val:(0,8):set the value for the green component of white balance
*/
#include <iostream>
#include "cameraInterface.h"
#include <GL/glut.h>
#include <cstdlib>
using namespace std;

int window(0);                          // Glut window identifier

CameraInterface *globalCam;

unsigned char data[4][TAILLE_BLOC];

void DrawGLScene()
{
    globalCam->runLoop();
    globalCam->ShowMeWhatYouGot(0,data[0]);
    globalCam->ShowMeWhatYouGot(1,data[1]);
    globalCam->ShowMeWhatYouGotDetected(0,data[2]);
    globalCam->ShowMeWhatYouGotDetected(1,data[3]);
	
    GLuint tex[4];
    glGenTextures(1, &tex[0]);
    glGenTextures(1, &tex[1]);
    glGenTextures(1, &tex[2]);
    glGenTextures(1, &tex[3]);
    glBindTexture(GL_TEXTURE_2D, tex[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, WIDTH, HEIGHT, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data[0]);
    
    glBindTexture(GL_TEXTURE_2D, tex[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, WIDTH, HEIGHT, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data[1]);
    
    glBindTexture(GL_TEXTURE_2D, tex[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, WIDTH, HEIGHT, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data[2]);
    
    glBindTexture(GL_TEXTURE_2D, tex[3]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, WIDTH, HEIGHT, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data[3]);
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBindTexture(GL_TEXTURE_2D, tex[0]);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2i(0, 0); glVertex2i(0, 0);
    glTexCoord2i(0, 1); glVertex2i(0, HEIGHT / 2);
    glTexCoord2i(1, 1); glVertex2i(WIDTH / 2, HEIGHT / 2);
    glTexCoord2i(1, 0); glVertex2i(WIDTH / 2, 0);
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D, tex[1]);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2i(0, 0); glVertex2i(WIDTH / 2, 0);
    glTexCoord2i(0, 1); glVertex2i(WIDTH / 2, HEIGHT / 2);
    glTexCoord2i(1, 1); glVertex2i(WIDTH, HEIGHT / 2);
    glTexCoord2i(1, 0); glVertex2i(WIDTH, 0);
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D, tex[2]);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2i(0, 0); glVertex2i(0, HEIGHT / 2);
    glTexCoord2i(0, 1); glVertex2i(0, HEIGHT);
    glTexCoord2i(1, 1); glVertex2i(WIDTH / 2, HEIGHT);
    glTexCoord2i(1, 0); glVertex2i(WIDTH / 2, HEIGHT / 2);
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D, tex[3]);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2i(0, 0); glVertex2i(WIDTH / 2, HEIGHT / 2);
    glTexCoord2i(0, 1); glVertex2i(WIDTH / 2, HEIGHT);
    glTexCoord2i(1, 1); glVertex2i(WIDTH, HEIGHT);
    glTexCoord2i(1, 0); glVertex2i(WIDTH, HEIGHT / 2);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);    
	
    glEnd();

    glutSwapBuffers();
}

void idleGLScene()
{
    glutPostRedisplay();
}

int main ( int argc,char **argv ) {
    CameraInterface camInterface(argc,argv);
	
	if(camInterface.UseHelperWindow){
		cout <<"Demarre fenetre de debug"<<endl;
		if(camInterface.init())	{
			globalCam = &camInterface;
			glutInit(&argc, argv);
			glutInitDisplayMode(GLUT_RGBA);
			glutInitWindowSize(WIDTH, HEIGHT);
			glutInitWindowPosition(0, 0);

			window = glutCreateWindow("Camera Espionne Russe");
			glClearColor(0.45f, 0.45f, 0.45f, 0.0f);

			glEnable(GL_ALPHA_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glAlphaFunc(GL_GREATER, 0.0f);

			glMatrixMode(GL_PROJECTION); 			
			glOrtho(0, WIDTH, 0, HEIGHT, -1, 1);
			glMatrixMode(GL_MODELVIEW);

			glutDisplayFunc(&DrawGLScene);
			glutIdleFunc(&idleGLScene);

			glutMainLoop();	
			
			camInterface.release();
		}
	}else{
		//Capture loop
		camInterface.loop();
	}
    
    //Test detection on 2 saved files
    cout <<"Closing normaly"<<endl;
}

