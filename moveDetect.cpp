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

enum {
    //Camera settings
	WIDTH = 640,
	HEIGHT = 480,
	TAILLE_BLOC = WIDTH * HEIGHT,

    //Movement detection parameters
    FOREGROUND = 255,
    BACKGROUND = 0,
    TRESHOLD = 18,
    NOISEFILTER = 3
};

struct Tampons {
	unsigned char data[2][TAILLE_BLOC];
};

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

/*
* Process all suported options on the command line and inits the camera. 
* Currently defaults to black and white (only suported algo right now)
*/
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
    else
      Camera.setFormat(raspicam::RASPICAM_FORMAT_GRAY);
    

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
    outFile.write ( (char*) data,Camera.getImageBufferSize() );
}


/*
*For testing purpose, reads a file into the same data structure as if the camera took the photo
*/
void loadImage(string filePath, unsigned char data[]){
    int  i=0, numrows = 0, numcols = 0, maxColor=0;
    stringstream ss;
    string inputLine = "";
    ifstream infile(filePath, ios::binary);
    
    //Version line
    getline(infile , inputLine);
    cout << endl << inputLine;

    ss << infile.rdbuf();
    ss >> numcols >> numrows >> maxColor;
    cout << numcols << " columns and " << numrows << " rows and " << maxColor << " maxColor value" << endl;

    //ss.ignore();
    for(i = 0; i < numcols * numrows; ++i){
        ss.read((char*)&data[i], 1);
    }
}

/*
* Inspiration : http://codeding.com/articles/motion-detection-algorithm
* Detects foreground object and pass the results into a erosion filter
*/
void detectMovement(unsigned char image1[], unsigned char image2[], raspicam::RaspiCam &Camera){
    int n = (NOISEFILTER - 1) /2;
    unsigned int marked = 0;

    Tampons output;
    //first-pass: difference and threshold filter (mark the pixels that are changed between two frames)
    for (unsigned int x = 0; x < WIDTH * HEIGHT; x++){
        unsigned int diff = abs(image1[x] - image2[x] );
        //cout << image1[x]  << " et " << image2[x];
        output.data[0][x] = (diff >= TRESHOLD ? FOREGROUND : BACKGROUND);
    }

     //second-pass: erosion filter (remove noise i.e. ignore minor differences between two frames)
    for ( int x = 0; x < WIDTH; x++){
        for(int y = 0; y < WIDTH * HEIGHT; y+= WIDTH){
            marked = 0;

            //i = ranger actuel -n, va jusqua la ranger + n
            for (int i = x - n; i < x + n; i++){
            //j == colonne actuel -n vas sjusqua colonne+n
	            for (int j = y - n*WIDTH; j < y + n*WIDTH; j+= WIDTH){
                    if(i < 0 || j < 0 || i > WIDTH || j > HEIGHT * WIDTH ){
                        continue;
                    }else{
                    
                        marked += (output.data[0][i+j] == FOREGROUND ? 1 : 0);
                    } 
                }
            }
            
            if(marked >= NOISEFILTER){
               //i = ranger actuel -n, va jusqua la ranger + n
                for (int i = x - n; i < x + n; i++){
                //j == colonne actuel -n vas sjusqua colonne+n
                    for (int j = y - n*WIDTH; j < y + n*WIDTH; j+= WIDTH){
                            output.data[1][i+j] = FOREGROUND;
                    }
                }
            }
        }
    }
    
    std::stringstream step1, step2;
    step1<<"step1.pgm";
    step2<<"step2.pgm";
    saveImage ( step1.str(), output.data[0], Camera );
    saveImage ( step2.str(), output.data[1], Camera );

}

/*
* Currently: saves some photos in the main loop and runs detectMovement on 2 previously saved pictured 
*/
int main ( int argc,char **argv ) {
    Timer timer;
    raspicam::RaspiCam Camera;
    string formatFile = ".pgm";
    bool speedOnly = false;
    unsigned int nFramesCaptured = 100;
    Tampons tampons;

    //Camera initialisation
    speedOnly = processCommandLine ( argc,argv,Camera );
    nFramesCaptured=getParamVal("-nframes",argc,argv,100);
    if ( Camera.getFormat()==raspicam::RASPICAM_FORMAT_RGB ){
        formatFile = ".ppm";
    }
   
    
   //Capture loop
   if ( Camera.open() ) {
        cout<<"Connected to camera ="<<Camera.getId() <<" bufs="<<Camera.getImageBufferSize( )<<endl;
        size_t i=0;
        timer.start();

        do{
            Camera.grab();
            //Camera.retrieve ( data );
            Camera.retrieve ( tampons.data[0] );
            if ( !speedOnly ) {
                    if ( i%5==0 ) 
                        cout<<"\r capturing ..."<<i<<"/"<<nFramesCaptured<<std::flush;
                    //save image every 30 images
                    if ( i%30==0 && i!=0  && nFramesCaptured>0 ) { 
                        std::stringstream fn;
                        fn<<"image"<<i<<formatFile;
                        saveImage ( fn.str(), reinterpret_cast<unsigned char *> (&tampons.data[0]) ,Camera );
                        cout<<"Saving "<<fn.str()<<endl;
                    }
            }
        }while(++i<nFramesCaptured || nFramesCaptured==0);

        timer.end();
        cout<< timer.getSecs()<< " seconds for "<< nFramesCaptured<< "  frames : FPS " << ( ( float ) ( nFramesCaptured ) / timer.getSecs() ) <<endl;
        Camera.release();
    }else{
        cout<<"Error opening camera, skipping capture loop"<<endl;
    }


    //Test detection on 2 saved files
    loadImage("sampleMove/me1.pgm", tampons.data[0]);
    loadImage("sampleMove/me2.pgm", tampons.data[1]);
    detectMovement(tampons.data[0], tampons.data[1], Camera);
    cout <<"Closing normaly"<<endl;
}

