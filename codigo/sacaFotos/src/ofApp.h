#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"

class ofApp : public ofBaseApp{

	public:
        void setup();
        void update();
        void draw();
        void exit();

		void keyPressed(int key);

    
        ofxKinect kinect;
    
        ofxCvColorImage colorImg;
    
        ofxCvGrayscaleImage grayImage; // grayscale depth image

        int angle;
    
		int width, height;
        bool bSaveSequence;
        bool sSaveSequence;
        int frame1;
        int frame2;
};
