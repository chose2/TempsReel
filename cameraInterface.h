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
    MAXBLOB=5,
    NBTAMPON=2,
    FOREGROUND = 255,
    BACKGROUND = 0,
    TRESHOLD = 18,
    NOISEFILTER = 3,
    MINSIZEBLOB = 50
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
	unsigned int topX=0, topY=0, bottomX=0, bottom=0;
};

class CameraInterface{
    private:
        Utils utils;
        raspicam::RaspiCam Camera;
        Timer timer;

        unsigned int nFramesCaptured;

        Tampons tampons;
        size_t currentTamponIndex = 0;

        Blob blobs[MAXBLOB];
        size_t currentblobIndex = 0;


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
    

    CameraInterface(int argc,char **argv){
        this->processCommandLine(argc, argv);
    }


    void loop(){
        sleep(3);
        if ( this->Camera.open() ) {
                std::cout <<"Connected to this->camera ="<<this->Camera.getId() <<" bufs=" << this->Camera.getImageBufferSize( )<<std::endl;
                size_t i=0;
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
                    
                   /*if ( i%5==0 ) 
                        std::cout <<"\r capturing ..."<<i<<"/"<<nFramesCaptured<<std::flush;
                    //save image every 30 images
                    if ( i%30==0 && i!=0  && nFramesCaptured>0 ) { 
                        std::stringstream fn, fn2;
                        fn<<"imageStartPoint"<<i<<".pgm";
                        utils.saveImagePGM ( fn.str(), reinterpret_cast<unsigned char *> (&tampons.data[currentTamponIndex == 0 ? NBTAMPON-1 : currentTamponIndex - 1]) ,this->Camera );
                        std::cout <<"Saving "<<fn.str()<<std::endl;
                      
                        fn2<<"imageEndPoint"<<i<<".pgm";
                        utils.saveImagePGM ( fn2.str(), reinterpret_cast<unsigned char *> (&tampons.data[currentTamponIndex]) ,this->Camera );
                        std::cout <<"Saving "<<fn2.str()<<std::endl;
                    }*/

                    currentTamponIndex = (currentTamponIndex == NBTAMPON -1 ? 0 : currentTamponIndex + 1);

                
                }while(++i<nFramesCaptured || nFramesCaptured==0);

                timer.end();
                std::cout << timer.getSecs()<< " seconds for "<< nFramesCaptured<< "  frames : FPS " << ( ( float ) ( nFramesCaptured ) / timer.getSecs() ) <<std::endl;
                this->Camera.release();
        }else{
            std::cout <<"Error opening this->camera, skipping capture loop"<<std::endl;
        }

    }

     /*
     *Detection dun groupe de MINSIZEBLOB de large minimum. De haut en bas vers la droite
     */
    void detectBlob(unsigned char mouvement[]){
        std::stringstream  step2;
        step2<<"movement2.pgm";
        int blobBorder;
        bool blobdetected = false;
        int index=0;

        /*old --  top  to right to bottom
        for (unsigned int x = 0; x < WIDTH * HEIGHT; x++){  
        index = x;
        */
        for ( int x = 0; x < WIDTH && !blobdetected; x++){
            for(int y = 0; y < WIDTH * HEIGHT && !blobdetected; y+= WIDTH){
                index = x+y;
                if(mouvement[index] == FOREGROUND){
                    //Si trop proche du bord de l'image ignore.
                    if(index % WIDTH < 45 ||  index % WIDTH > WIDTH - MINSIZEBLOB){
                        continue;
                    }
                        
                    
                    
                    //vefifie que c'est pas un pixel tout seul
                    int limit = index + MINSIZEBLOB;
                    for(blobBorder = index; blobBorder < WIDTH * HEIGHT && blobBorder < limit ; ++blobBorder){
                        if(mouvement[blobBorder] != FOREGROUND )
                            break;
                    }
                    //Si le blob était de taille suffirance, save
                    if(blobBorder - index >= MINSIZEBLOB){
                        blobs[currentblobIndex].topX = index % WIDTH;
                        blobs[currentblobIndex].topY = index / WIDTH;
                        //std::cout << std::endl << "blob detected :"  << (int) currentblobIndex <<  std::cout << " at point  :" << index % WIDTH  <<  ", " << index / WIDTH << " de size " << blobBorder - index << std::endl;
                        blobdetected = true;
                        break;
                    }
                }
            }
        }
        
        //Si on vien detecter un nouveau blob
        if(blobdetected){
            //Si on a rempli notre tableau de blob, analyse, sinon continue de remplir le tableau de blob
            if(currentblobIndex == MAXBLOB - 1){
                //compareBlob();
                std::cout << "Blob full, Current blob :"  << (int) currentblobIndex  << " previous point  :" << blobs[0].topX << " current "<< blobs[currentblobIndex].topX << std::endl;
                if(blobs[MAXBLOB - 1].topX == blobs[0].topX){
                    std::cout << " BLOB IS STILL " << std::endl;
                }else if(blobs[MAXBLOB -1].topX < blobs[0].topX){
                    std::cout << " BLOB IS GOING LEFT " << std::endl;
                }else{
                    std::cout << " BLOB IS GOING right " << std::endl;
                }
                utils.saveImagePGM ( step2.str(), mouvement, this->Camera );

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

        Tampons output;
        //first-pass: difference and threshold filter (mark the pixels that are changed between two frames)
        for (unsigned int x = 0; x < WIDTH * HEIGHT; x++){
            unsigned int diff = abs(image1[x] - image2[x] );
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
                    totalMarked += marked;
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

 
        if(totalMarked !=0){
            detectBlob(output.data[1]);
        }
       

    }
/*  to use when final
    void compareBlob(){
        if(blobs[MAXBLOB -1].topX == blobs[0].topX){
            std::cout << " BLOB IS STILL " << std::endl;
        }else if(blobs[MAXBLOB -1].topX < blobs[0].topX){
            std::cout << " BLOB IS GOING LEFT " << std::endl;
        }else{
            std::cout << " BLOB IS GOING right " << std::endl;
        }
    }*/


};

       /* Anaylse les 2 image de moi
       utils.loadImagePGM("sampleMove/me1.pgm", tampons.data[0]);
        utils.loadImagePGM("sampleMove/me2.pgm", tampons.data[1]);
        detectMovement(tampons.data[0], tampons.data[1], Camera);*/