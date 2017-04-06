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
#include "rpiPWM1.h"
#include <GL/glut.h>
using namespace std;

int window(0);                          // Glut window identifier

void DrawGLScene()
{
    glClear(GL_COLOR_BUFFER_BIT);
    /*std::vector<uint8_t> currentRgb;

    glPointSize(2.0f);
    glBegin(GL_POINTS);

    //Real time cam
    currentRgb = DecodeurScene.rgb;
    realTimeDepth = DecodeurScene.depth;

    if (!color) glColor3ub(255, 255, 255);

    if(!currentRgb.empty() && !realTimeDepth.empty())
    {
        float HauteurMax = std::stof(ConfigHelper.GetString("HAUTEUR_MAX"));
        float HauteurMin = std::stof(ConfigHelper.GetString("HAUTEUR_MIN"));
        float HauteurKin = std::stof(ConfigHelper.GetString("HAUTEUR_KINECT"));
        float f = 595.f;
        for (size_t i = 0; i < realTimeDepth.size(); ++i)
        {
            Vector3 vec = Vector3((i%IR_CAMERA_RESOLUTION_X - (IR_CAMERA_RESOLUTION_X-1)/2.f) * realTimeDepth[i] / f,
                                  (-(i/IR_CAMERA_RESOLUTION_X - (IR_CAMERA_RESOLUTION_Y-1)/2.f) * realTimeDepth[i] / f) + HauteurKin,
                                  -realTimeDepth[i]);

            if(vec.y > HauteurMax || vec.y < HauteurMin )
            {
                continue;
            }
            if (color)
                glColor3ub( currentRgb[3*i+0],    // R
                            currentRgb[3*i+1],    // G
                            currentRgb[3*i+2]);   // B

            vec = DecodeurScene.RealCam.matrixToWorld * vec;
            glVertex3f(vec.x, vec.y, vec.z);
        }
    }


    glEnd();*/

    glutSwapBuffers();
}

/*void resizeGLScene(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, (float)width / height, 90.0, 110000.0);

    glMatrixMode(GL_MODELVIEW);
}*/

void idleGLScene()
{
    globalCam->runLoop();
    glutPostRedisplay();
}
CameraInterface *globalCam;
/*
* Currently: saves some photos in the main loop and runs detectMovement on 2 previously saved pictured 
*/
int main ( int argc,char **argv ) {
    Timer timer;
    CameraInterface camInterface(argc,argv);

    rpiPWM1 pwm(50.0, 1024, 7.5, rpiPWM1::MSMODE);
    usleep(2000000); //let the camera to be place to it's neutral position
	
	if(camInterface.UseHelperWindow){
		if(camInterface.init())	{
			globalCam = &camInterface;
			glutInit(&argc, argv);
			glutInitDisplayMode(GLUT_RGB);
			glutInitWindowSize(640, 480);
			glutInitWindowPosition(0, 0);

			window = glutCreateWindow("Camera Espionne Russe");
			glClearColor(0.45f, 0.45f, 0.45f, 0.0f);

			glEnable(GL_ALPHA_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glAlphaFunc(GL_GREATER, 0.0f);

			glMatrixMode(GL_PROJECTION);
			gluPerspective(50.0, 1.0, 900.0, 11000.0);

			glutDisplayFunc(&DrawGLScene);
			glutIdleFunc(&idleGLScene);
			//glutReshapeFunc(&resizeGLScene);

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

