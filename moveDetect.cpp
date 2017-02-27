#define WIDTH 1280
#define HEIGHT 960
#define BUFFERSIZE (HEIGHT*WIDTH)


/*
    Usage:
    [-help shows this help]
    [-col sets color mode]
    [-test_speed use for test speed and no images will be saved
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
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <sys/timeb.h>
#include "src/raspicam.h"
#include "timer.h"
using namespace std;

int findParam ( string param,int argc,char **argv ) {
    int idx=-1;
    for ( int i=0; i<argc && idx==-1; i++ )
        if ( string ( argv[i] ) ==param ) idx=i;
    return idx;
}

float getParamVal ( string param,int argc,char **argv,float defvalue=-1 ) {
    int idx=-1;
    for ( int i=0; i<argc && idx==-1; i++ )
        if ( string ( argv[i] ) ==param ) idx=i;
    if ( idx==-1 ) return defvalue;
    else return atof ( argv[  idx+1] );
}


bool processCommandLine ( int argc,char **argv,raspicam::RaspiCam &Camera ) {
    bool speedtest =false;
    Camera.setWidth ( WIDTH );
    Camera.setHeight (HEIGHT );
    Camera.setBrightness ( getParamVal ( "-br",argc,argv,50 ) );
    Camera.setSharpness ( getParamVal ( "-sh",argc,argv,0 ) );
    Camera.setContrast ( getParamVal ( "-co",argc,argv,0 ) );
    Camera.setSaturation ( getParamVal ( "-sa",argc,argv,0 ) );
    Camera.setShutterSpeed( getParamVal ( "-ss",argc,argv,0 ) );
    Camera.setISO ( getParamVal ( "-iso",argc,argv ,400 ) );
    
    if ( findParam ( "-vs",argc,argv ) !=-1 )
        Camera.setVideoStabilization ( true );
    
    Camera.setExposureCompensation ( getParamVal ( "-ec",argc,argv ,0 ) );

    if ( findParam ( "-col",argc,argv ) !=-1 )
      Camera.setFormat(raspicam::RASPICAM_FORMAT_RGB);

    if ( findParam ( "-yuv",argc,argv ) !=-1 ) 
      Camera.setFormat(raspicam::RASPICAM_FORMAT_YUV420);

    if ( findParam ( "-test_speed",argc,argv ) !=-1 )
        speedtest=true;

    Camera.setAWB_RB(getParamVal("-awb_b",argc,argv ,1), getParamVal("-awb_g",argc,argv ,1));
    return speedtest;
}







void saveImage ( string filepath,unsigned char *data,raspicam::RaspiCam &Camera ) {
    std::ofstream outFile ( filepath.c_str(),std::ios::binary );
    if ( Camera.getFormat()==raspicam::RASPICAM_FORMAT_BGR ||  Camera.getFormat()==raspicam::RASPICAM_FORMAT_RGB ) {
        outFile<<"P6\n";
    } else if ( Camera.getFormat()==raspicam::RASPICAM_FORMAT_GRAY ) {
        outFile<<"P5\n";
    } else if ( Camera.getFormat()==raspicam::RASPICAM_FORMAT_YUV420 ) { //made up format
        outFile<<"P7\n";
    }
    outFile<<Camera.getWidth() <<" "<<Camera.getHeight() <<" 255\n";
    outFile.write ( ( char* ) data,Camera.getImageBufferSize() );
}


int main ( int argc,char **argv ) {
    Timer timer;
    raspicam::RaspiCam Camera;
    string formatFile = ".pgm";
    bool speedOnly = false;
    unsigned int nFramesCaptured = 100;
    speedOnly = processCommandLine ( argc,argv,Camera );
    nFramesCaptured=getParamVal("-nframes",argc,argv,100);

    if ( Camera.getFormat()==raspicam::RASPICAM_FORMAT_RGB ){
        formatFile = ".ppm";
    }

    //Format gris = width * height, rgb = 3* width * height
    //a voir pour savoir la taille du tableau on compile, set camera prend des floats, voir pour static cast
    //variable preprocesseur
    unsigned char *data=new unsigned char[  Camera.getImageBufferSize( )];

  
    cout<<"Connecting to camera"<<endl;
    
    if ( Camera.open() ) {
        cout<<"Connected to camera ="<<Camera.getId() <<" bufs="<<Camera.getImageBufferSize( )<<endl;
        size_t i=0;
        timer.start();
        do{
            Camera.grab();
            Camera.retrieve ( data );
            if ( !speedOnly ) {
                    if ( i%5==0 ) 
                        cout<<"\r capturing ..."<<i<<"/"<<nFramesCaptured<<std::flush;
                    //save image every 30 images
                    if ( i%30==0 && i!=0  && nFramesCaptured>0 ) { 
                        std::stringstream fn;
                        fn<<"image"<<i<<formatFile;
                        saveImage ( fn.str(),data,Camera );
                        cout<<"Saving "<<fn.str()<<endl;
                    }
            }
        }while(++i<nFramesCaptured || nFramesCaptured==0);
        timer.end();

        cout<< endl<< timer.getSecs()<< " seconds for "<< nFramesCaptured<< "  frames : FPS " << ( ( float ) ( nFramesCaptured ) / timer.getSecs() ) <<endl;
        
        Camera.release();
    }else{
        cout<<"Error opening camera, skipping capture loop"<<endl;
    }
    cout <<"Closing normaly"<<endl;
}

