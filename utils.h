/*Gabriel Gagné
Danny Groleau
*/
#include <iostream>
#include "src/raspicam.h"
#include <fstream>
#include <sstream>


class Utils{
    private:
    std::stringstream ss;
public:
    Utils(){}

    int findParam ( std::string param,int argc,char **argv ) {
        int idx=-1;
        for ( int i=0; i<argc && idx==-1; i++ )
            if ( std::string ( argv[i] ) ==param ) idx=i;
        return idx;
    }

    float getParamVal ( std::string param,int argc,char **argv,float defvalue=-1 ) {
        int idx=-1;
        for ( int i=0; i<argc && idx==-1; i++ )
            if ( std::string ( argv[i] ) ==param ) idx=i;
        if ( idx==-1 ) return defvalue;
        else return atof ( argv[  idx+1] );
    }

    void  saveImagePGM ( std::string filepath,unsigned char *data,raspicam::RaspiCam &Camera ) {
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
    void loadImagePGM(std::string filePath, unsigned char data[]){
        int  i=0, numrows = 0, numcols = 0, maxColor=0;
        std::string inputLine = "";
        std::ifstream infile(filePath, std::ios::binary);
        
        //Version line
        getline(infile , inputLine);
        std::cout << std::endl << inputLine;

        ss << infile.rdbuf();
        ss >> numcols >> numrows >> maxColor;
        std::cout << numcols << " columns and " << numrows << " rows and " << maxColor << " maxColor value" << std::endl;

        //ss.ignore();
        for(i = 0; i < numcols * numrows; ++i){
            ss.read((char*)&data[i], 1);
        }
    }

};
