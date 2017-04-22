/*Gabriel Gagné
Danny Groleau
*/
#include "src/raspicam.h"
#include "utils.h"
#include <ctime>
#include <cstdlib>
#include "rpiPWM1.h"
#include <chrono>

using namespace std;
using namespace std::chrono;

enum {

    //Movement detection parameters
    MAXBLOB=1,
    NBTAMPON=2,
    FOREGROUND = 255,
    BACKGROUND = 0,
    TRESHOLD = 25,
    NOISEFILTER = 3,
};

//Camera settings
const int WIDTH = 640;
const int HEIGHT = 480;
const int TAILLE_BLOC = WIDTH * HEIGHT;
const int MIDDLE = WIDTH / 2;
const float FOV = 54.0f; //based on doc http://elinux.org/Rpi_Camera_Module#Technical_Parameters_.28v.2_board.29

//servo settings
const int MINPWM = 4;
const int MAXPWM = 11;
const int PWMRANGE = MAXPWM - MINPWM;
const int MINANGLE = -75;
const int MAXANGLE = 75;
const int ANGLERANGE = MAXANGLE - MINANGLE;

//Motion settings
const int MINMOTIONSIZE = TAILLE_BLOC * 0.01f;
const int MAXMOTIONSIZE = TAILLE_BLOC * 0.65f;

struct Tampons {
    unsigned char data[NBTAMPON][TAILLE_BLOC];
};

struct Blob {
    unsigned int moyX=0 ;
};

class CameraInterface{
private:
    Utils utils;
    raspicam::RaspiCam Camera;
    rpiPWM1 servoMoteur;
    float currentAngle;

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
            
        if ( utils.findParam ( "-debug",argc,argv ) !=-1 )
            this->UseHelperWindow = true;
        
        this->Camera.setExposureCompensation ( utils.getParamVal ( "-ec",argc,argv ,0 ) );

        this->Camera.setFormat(raspicam::RASPICAM_FORMAT_GRAY);
        

        if ( utils.findParam ( "-yuv",argc,argv ) !=-1 ) 
            this->Camera.setFormat(raspicam::RASPICAM_FORMAT_YUV420);

        this->Camera.setAWB_RB(utils.getParamVal("-awb_b",argc,argv ,1), utils.getParamVal("-awb_g",argc,argv ,1));
        nFramesCaptured  = utils.getParamVal("-nframes",argc,argv,300);
    }
    
    //La camera semble besoin d'avoir un certain nombre de grab/retreive avant darriver a un bon niveau dexposition
    void settle(raspicam::RaspiCam &Camera){
        servoMoteur.setFrequency(50.0);
        servoMoteur.setCounts(1024);
        servoMoteur.setDutyCycle(7.5);
        servoMoteur.setMode(rpiPWM1::MSMODE);
        usleep(500000); //let the camera to be place to it's neutral position 1/2 second
        currentAngle = 0.0f;
        
        for(int i=0; i<100; i++){
            this->Camera.grab();
        }
    }
    
    float pulseWidthToAngle(float pw){ // return angle from -90 to 90
        return ((pw - MINPWM) / (MAXPWM - MINPWM) * ANGLERANGE) - (ANGLERANGE / 2);
    }
    
    float angleToPulseWidth(float angle){ //return pulse width from 4 to 11
        return (angle + (ANGLERANGE / 2)) / ANGLERANGE * PWMRANGE + MINPWM;
    }
    
    float imagePositionToAngle(int pos){ // return float from -27 to 27
        return ((float)pos / WIDTH * FOV) - (FOV / 2);
    }
    
    void setServoAngle(float angle){            
        if(currentAngle + angle > MINANGLE && currentAngle + angle < MAXANGLE) {
            currentAngle += angle;
            servoMoteur.setDutyCycle(angleToPulseWidth(currentAngle));
            //usleep(60000);//make sure camera don't grab camera movement
        }
    }

public:
    bool UseHelperWindow = false;

    CameraInterface(int argc,char **argv){
        this->processCommandLine(argc, argv); 
    }

    bool init()    {
        if ( this->Camera.open() ) {
            settle(Camera);    
            cout <<"Connected to this->camera ="<<this->Camera.getId() <<" bufs=" << this->Camera.getImageBufferSize( )<<endl;            
            return true;
        }else{
            cout <<"Error opening this->camera, skipping capture loop"<<endl;
            return false;
        }
    }

    void ShowMeWhatYouGot(int index, unsigned char *outData){ 
        copy(tampons.data[index], tampons.data[index] + TAILLE_BLOC, outData);
    }
    void ShowMeWhatYouGotDetected(int index, unsigned char *outData){
        copy(blackWhite.data[index], blackWhite.data[index] + TAILLE_BLOC, outData);
    }
    
    void release()    {
        this->Camera.release();
    }
    
    void loop(){
        size_t i=0;  
        if ( init() ) {
        
            auto pre = high_resolution_clock::now();
            do{
                runLoop();
            }while(++i<nFramesCaptured || nFramesCaptured==0);
            auto post = high_resolution_clock::now();
            
            cout << duration_cast<microseconds>(post - pre).count() << " us for "<< nFramesCaptured<< "  frames : FPS " << ( ( float ) ( nFramesCaptured ) / (duration_cast<microseconds>(post - pre).count() /1000000) ) <<endl;
            this->Camera.release();        
        }
    }
    
    //minimaliste loop
    void runLoop() {
        this->Camera.grab();
        if(firstFrame){
            this->Camera.retrieve ( tampons.data[0] );
            firstFrame = false;
        }else{
            this->Camera.retrieve ( tampons.data[1] );
            firstFrame = true;
            
            detectMovement(tampons.data[0], tampons.data[1], Camera);
        }        
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
        if(totalMarked >= MINMOTIONSIZE && totalMarked < MAXMOTIONSIZE){
            blobs[currentblobIndex].moyX = totalX / totalMarked;
            //cout << "BLOB SAVED AT  " << currentblobIndex << " SIZE OF " << totalMarked << " MOY OF " << blobs[currentblobIndex].moyX << endl;
            if(currentblobIndex == MAXBLOB - 1){
            
                setServoAngle(imagePositionToAngle(blobs[MAXBLOB -1].moyX));

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
            detectBlob(blackWhite.data[1]);
        }
    }
};
