#include <fstream>
#include <cstdlib>
#include <iostream>
#include "src/raspicam_still.h"
#include <chrono>
#include <thread>

using namespace std;

raspicam::RaspiCam_Still Camera;
//Returns the value of a param. If not present, returns the defvalue
float getParamVal ( string id,int argc,char **argv,float defvalue ) {
    for ( int i=0; i<argc; i++ )
        if ( id== argv[i] )
            return atof ( argv[i+1] );
    return defvalue;
}


//prints program command line usage
void usage() {
    cout<<"-w val : sets image width (2592 default)"<<endl;
    cout<<"-h val : sets image height (1944 default)"<<endl;
     cout<<"-iso val: set iso [100,800] (400 default)"<<endl;
}


int main ( int argc, char *argv[] ) {
    usage();

    int width = getParamVal ( "-w",argc,argv,2592 );
    int height =getParamVal ( "-h",argc,argv,1944 );
    int iso=getParamVal ( "-iso",argc,argv,400);


    cout << "Initializing ..."<<width<<"x"<<height<<endl;
    Camera.setWidth ( width );
    Camera.setHeight ( height );
    Camera.setISO(iso);
    Camera.setEncoding ( raspicam::RASPICAM_ENCODING_BMP );
    Camera.open();
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    
    cout<<"capture"<<endl;
    unsigned int length = Camera.getImageBufferSize(); // Header + Image Data + Padding
    unsigned char * data = new unsigned char[length];
      if ( !Camera.grab_retrieve(data, length) ) {
        cerr<<"Error in grab"<<endl;
        return -1;
    }

    cout<<"saving picture.bmp"<<endl;
    ofstream file ( "picture.bmp",ios::binary );
    file.write ( ( char* ) data,   length );
    return 0;
}