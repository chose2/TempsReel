#include "src/raspicam.h"
#include "utils.h"
#include "timer.h"
#include <ctime>
#include <cstdlib>


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


class CameraInterface{
    private:
        raspicam::RaspiCam Camera;
        Timer timer;
        unsigned int nFramesCaptured;
        Tampons tampons;

        
        /*
        * Process all suported options on the command line and inits the camera. 
        * Currently defaults to black and white (only suported algo right now)
        */
        void processCommandLine ( int argc,char **argv) {
            this->Camera.setWidth ( WIDTH );
            this->Camera.setHeight (HEIGHT );
            this->Camera.setBrightness ( utils.getParamVal ( "-br",argc,argv,50 ) );
            this->Camera.setSharpness ( utils.getParamVal ( "-sh",argc,argv,0 ) );
            this->Camera.setContrast ( utils.getParamVal ( "-co",argc,argv,0 ) );
            this->Camera.setSaturation ( utils.getParamVal ( "-sa",argc,argv,0 ) );
            this->Camera.setShutterSpeed( utils.getParamVal ( "-ss",argc,argv,0 ) );
            this->Camera.setISO ( utils.getParamVal ( "-iso",argc,argv ,400 ) );
            
            if ( utils.findParam ( "-vs",argc,argv ) !=-1 )
                this->Camera.setVideoStabilization ( true );
            
            this->Camera.setExposureCompensation ( utils.getParamVal ( "-ec",argc,argv ,0 ) );

            this->Camera.setFormat(raspicam::RASPICAM_FORMAT_GRAY);
            

            if ( utils.findParam ( "-yuv",argc,argv ) !=-1 ) 
            this->Camera.setFormat(raspicam::RASPICAM_FORMAT_YUV420);
            
            this->Camera.setAWB_RB(utils.getParamVal("-awb_b",argc,argv ,1), utils.getParamVal("-awb_g",argc,argv ,1));
            nFramesCaptured  = utils.getParamVal("-nframes",argc,argv,100);
        }

public:
    Utils utils;

    CameraInterface(int argc,char **argv){
        this->processCommandLine(argc, argv);
    }

    void loop(){
        if ( this->Camera.open() ) {
                std::cout <<"Connected to this->camera ="<<this->Camera.getId() <<" bufs=" << this->Camera.getImageBufferSize( )<<std::endl;
                size_t i=0;
                timer.start();
                do{
                    this->Camera.grab();
                    //this->Camera.retrieve ( data );
                    this->Camera.retrieve ( tampons.data[0] );
                    if ( i%5==0 ) 
                        std::cout <<"\r capturing ..."<<i<<"/"<<nFramesCaptured<<std::flush;
                    //save image every 30 images
                    if ( i%30==0 && i!=0  && nFramesCaptured>0 ) { 
                        std::stringstream fn;
                        fn<<"image"<<i<<".pgm";
                        utils.saveImagePGM ( fn.str(), reinterpret_cast<unsigned char *> (&tampons.data[0]) ,this->Camera );
                        std::cout <<"Saving "<<fn.str()<<std::endl;
                    }
                }while(++i<nFramesCaptured || nFramesCaptured==0);

                timer.end();
                std::cout << timer.getSecs()<< " seconds for "<< nFramesCaptured<< "  frames : FPS " << ( ( float ) ( nFramesCaptured ) / timer.getSecs() ) <<std::endl;
                this->Camera.release();
        }else{
            std::cout <<"Error opening this->camera, skipping capture loop"<<std::endl;
        }
        utils.loadImagePGM("sampleMove/me1.pgm", tampons.data[0]);
        utils.loadImagePGM("sampleMove/me2.pgm", tampons.data[1]);
        detectMovement(tampons.data[0], tampons.data[1], Camera);
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
            //std::cout  << image1[x]  << " et " << image2[x];
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
        utils.saveImagePGM ( step1.str(), output.data[0], this->Camera );
        utils.saveImagePGM ( step2.str(), output.data[1], this->Camera );

    }


};
