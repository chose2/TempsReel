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
using namespace std;



/*
* Currently: saves some photos in the main loop and runs detectMovement on 2 previously saved pictured 
*/
int main ( int argc,char **argv ) {
    Timer timer;
    CameraInterface camInterface(argc,argv);

    rpiPWM1 pwm(50.0, 1024, 7.5, rpiPWM1::MSMODE);
    usleep(2000000); //let the camera to be place to it's neutral position
	
    //Capture loop
    camInterface.loop();
    
    //Test detection on 2 saved files
    cout <<"Closing normaly"<<endl;
}

