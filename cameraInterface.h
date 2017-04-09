#include "src/raspicam.h"
#include "utils.h"
#include "timer.h"
#include <ctime>
#include <cstdlib>
#include "rpiPWM1.h"

enum {
    //Camera settings
	WIDTH = 640,
	HEIGHT = 480,
	TAILLE_BLOC = WIDTH * HEIGHT,
    MIDDLE = WIDTH / 2,


    //Movement detection parameters
    MAXBLOB=3,
    NBTAMPON=2,
    FOREGROUND = 255,
    BACKGROUND = 0,
    TRESHOLD = 18,
    NOISEFILTER = 3,
};

enum DIRECTION{
    STILL = 1,
    LEFT=2,
    RIGHT=3

};

struct Tampons {
	unsigned char data[NBTAMPON][TAILLE_BLOC];
};

struct Blob {
	unsigned int topX=0, topY=0, moyX=0 ;
};

class CameraInterface{
    private:
        Utils utils;
        raspicam::RaspiCam Camera;
        Timer timer;
		rpiPWM1 servoMoteur;
		float currentAngle; // from -90 to 90

        unsigned int nFramesCaptured;

        Tampons tampons;
		Tampons blackWhite;
        size_t currentTamponIndex = 0;

        Blob blobs[MAXBLOB];
        size_t currentblobIndex = 0;
		
		bool firstFrame = true;
		
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
            

            if ( utils.findParam ( "-yuv",argc,argv ) !=-1 ) {
            this->Camera.setFormat(raspicam::RASPICAM_FORMAT_YUV420);
 
            }

            this->Camera.setAWB_RB(utils.getParamVal("-awb_b",argc,argv ,1), utils.getParamVal("-awb_g",argc,argv ,1));
            nFramesCaptured  = utils.getParamVal("-nframes",argc,argv,100);
        }
        //La camera semble besoin d'avoir un certain nombre de grab/retreive avant darriver a un bon niveau dexposition
        void settle(raspicam::RaspiCam &Camera){
			servoMoteur.setFrequency(50.0);
			servoMoteur.setCounts(1024);
			servoMoteur.setDutyCycle(7.5);
			servoMoteur.setMode(piPWM1::MSMODE);
			usleep(500 000); //let the camera to be place to it's neutral position 1/2 second
			currentAngle = 0.0f;
      		for(int i=0; i<100; i++){
        		this->Camera.grab();
        	}
        }

public:
    bool UseHelperWindow = true;

    CameraInterface(int argc,char **argv){
        this->processCommandLine(argc, argv);
  
    }

	bool init()	{
		if ( this->Camera.open() ) {
        	settle(Camera);	
			std::cout <<"Connected to this->camera ="<<this->Camera.getId() <<" bufs=" << this->Camera.getImageBufferSize( )<<std::endl;			
			return true;
        }else{
            std::cout <<"Error opening this->camera, skipping capture loop"<<std::endl;
			return false;
        }
	}

	void ShowMeWhatYouGot(unsigned char *outData){		
		for(int i = 0; i < TAILLE_BLOC; ++i){
			outData[i] = blackWhite.data[1][i];
		}
	}
	
	void release()	{
		this->Camera.release();
	}
	
    void loop(){
     	size_t i=0;  
        if ( this->Camera.open() ) {
        	settle(Camera);	
                std::cout <<"Connected to this->camera ="<<this->Camera.getId() <<" bufs=" << this->Camera.getImageBufferSize( )<<std::endl;
                timer.start();
                do{
                    this->Camera.grab();
                    //Met l'image caupter dans le tampon actuel
                    this->Camera.retrieve ( tampons.data[currentTamponIndex] );

                    if(i != 0){
                        //detect le movement dans lancient shot et l'actuel
                         detectMovement(tampons.data[currentTamponIndex == 0 ? NBTAMPON-1 : currentTamponIndex - 1], 
                                        tampons.data[currentTamponIndex], 
                                        Camera);
                    }
                    currentTamponIndex = (currentTamponIndex == NBTAMPON -1 ? 0 : currentTamponIndex + 1);

                }while(++i<nFramesCaptured || nFramesCaptured==0);

                timer.end();
                std::cout << timer.getSecs()<< " seconds for "<< nFramesCaptured<< "  frames : FPS " << ( ( float ) ( nFramesCaptured ) / timer.getSecs() ) <<std::endl;
                this->Camera.release();		
        }else{
            std::cout <<"Error opening this->camera, skipping capture loop"<<std::endl;
        }

    }
	//minimaliste loop
	void runLoop() {
		this->Camera.grab();
		this->Camera.retrieve ( tampons.data[currentTamponIndex] );

		if(!firstFrame){
			//detect le movement dans lancient shot et l'actuel
			 detectMovement(tampons.data[currentTamponIndex == 0 ? NBTAMPON-1 : currentTamponIndex - 1], 
							tampons.data[currentTamponIndex], 
							Camera);
		}
		firstFrame = false;
		currentTamponIndex = (currentTamponIndex == NBTAMPON -1 ? 0 : currentTamponIndex + 1);
	}
	
     /*
     *Detection dun groupe de MINSIZEBLOB de large minimum. De haut en bas vers la droite
     */
    void detectBlob(unsigned char mouvement[]){
        long int totalX = 0;
        int totalMarked = 0; 
        for ( int x = 0; x < WIDTH; x++){
            for(int y = 0; y < WIDTH * HEIGHT; y+= WIDTH){
                if(mouvement[x+y] == FOREGROUND){
                    totalX += x; 
                    totalMarked++;
                }
            }
        }
        //Si une detection de mouvement prenant au moins 1% de limage (0.01 * 640*480) et 80%
        if(totalMarked >= 3072 && totalMarked < 245 760){
            blobs[currentblobIndex].moyX = totalX / totalMarked;
            std::cout << "BLOB SAVED AT  " << currentblobIndex << " SIZE OF " << totalMarked << " MOY OF " << blobs[currentblobIndex].moyX << std::endl;
            if(currentblobIndex == MAXBLOB - 1){
                //compareBlob();
                if(blobs[MAXBLOB - 1].moyX > blobs[0].moyX){
                    std::cout << " BLOB IS RIGHT " << " adjust " <<  (MIDDLE + blobs[MAXBLOB -1].moyX ) << std::endl;
                }else if(blobs[MAXBLOB -1].moyX < blobs[0].moyX){
                    std::cout << " BLOB IS GOING LEFT " << " adjust minus " <<  (MIDDLE - blobs[MAXBLOB -1].moyX)  << std::endl;     
                }else{
                    std::cout << " BLOB IS GOING STILL " << std::endl;
                }
                //Reset tableau de blob
                currentblobIndex=0;
            }else{
                currentblobIndex++;
            }
        }
    }
    /*
    * Inspiration : http://codeding.com/articles/motion-detection-algorithm
    * Detects foreground object and pass the results into a erosion filter
    */
    void detectMovement(unsigned char image1[], unsigned char image2[], raspicam::RaspiCam &Camera){
        int n = (NOISEFILTER - 1) /2;
        unsigned int marked = 0;
        unsigned int totalMarked = 0;        



        //Tampons output;
        //first-pass: difference and threshold filter (mark the pixels that are changed between two frames)
        for (unsigned int x = 0; x < WIDTH * HEIGHT; x++){
            //Profite du loop pour reset la partie erosion filter
            blackWhite.data[1][x] = BACKGROUND;
            unsigned int diff = abs(image1[x] - image2[x] );
            blackWhite.data[0][x] = (diff >= TRESHOLD ? FOREGROUND : BACKGROUND);
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
                            marked += (blackWhite.data[0][i+j] == FOREGROUND ? 1 : 0);
                        } 
                    }
                }

                if(marked >= NOISEFILTER){
                    totalMarked += marked;
                    //i = ranger actuel -n, va jusqua la ranger + n
                    for (int i = x - n; i < x + n; i++){
                    //j == colonne actuel -n vas sjusqua colonne+n
                        for (int j = y - n*WIDTH; j < y + n*WIDTH; j+= WIDTH){
                                blackWhite.data[1][i+j] = FOREGROUND;
                        }
                    }
                }
            }
        }

 
        if(totalMarked !=0){
            //std::cout << " BLOB detected Marked: " << totalMarked << std::endl;
            detectBlob(blackWhite.data[1]);
        }
    }
};
