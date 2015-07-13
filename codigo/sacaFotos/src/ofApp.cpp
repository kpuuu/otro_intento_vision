#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    width = 720;
    height = 480;
    
    frame1 = 0;// init frame value to zero
    frame2=0;
    bSaveSequence = false;// this one will allow you to save just when you want to and not all the time that the app is running.
    sSaveSequence =false;
    angle = 20;
    kinect.setRegistration(true);
    kinect.init(true);
    kinect.open();
    kinect.setCameraTiltAngle(angle);
    
    //colorImg.allocate(kinect.width,kinect.height); //NO necesitamos color :o!
    grayImage.allocate(kinect.width,kinect.height);
    

}


//--------------------------------------------------------------
void ofApp::update()
{
    kinect.update();
    
    if (kinect.isFrameNew())
    {
        //colorImg.setFromPixels(kinect.getPixels(), 640, 480);
        grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width,kinect.height);
    }
   
    
    if(bSaveSequence){
        //ofxCvGrayscaleImage imagenGris;
        //imagenGris = colorImg;
        ofImage img;
        
        img.setFromPixels(kinect.getPixels(), kinect.getWidth(), kinect.getHeight(), OF_IMAGE_GRAYSCALE);
        bSaveSequence=false;
        img.saveImage("../../opencv/images/"+ofToString(frame1)+".png"); // change the ".jpg" for ".png" if you want a png sequence.
        frame1++;
    }

    if(sSaveSequence)
    {
        ofImage img2;
        img2.setFromPixels(kinect.getDepthPixels(), kinect.getWidth(), kinect.getHeight(), OF_IMAGE_GRAYSCALE);
        sSaveSequence=false;
        img2.saveImage(ofToString(frame0)+"_profundida.png"); // change the ".jpg" for ".png" if you want a png sequence.
        frame2++;
    }
}

//--------------------------------------------------------------
void ofApp::draw()
{
    kinect.draw(0, 0, width, height);
    kinect.drawDepth(350, 0, width, height);
    stringstream reportStream;
    reportStream <<"Presione las flechas para subir o bajar la camara"<<endl<<"Presione f para tener pantalla completa"<<endl<<"Presione s para guardar la imagen"<<endl<<"Presione d para guardar los mapas de profundidad"<<endl<<"Presione esc para salir"<< endl;
    ofDrawBitmapString(reportStream.str(), 20, 652);
}


//--------------------------------------------------------------
void ofApp::keyPressed  (int key)
{
    switch (key)
    {
        case OF_KEY_UP:
            angle++;
            if (angle > 30)
                angle = 30;
            kinect.setCameraTiltAngle(angle);
            break;
            
        case OF_KEY_DOWN:
            angle--;
            if (angle < -30)
                angle = -30;
            kinect.setCameraTiltAngle(angle);
            break;
            
        case 'f':
            ofToggleFullscreen();
            
        case 's': // this is the space bar. you can change it to any key you want to.
            bSaveSequence =true;

        case 'd':
            sSaveSequence=true;
    }  
}

//--------------------------------------------------------------
void ofApp::exit() {
    kinect.setCameraTiltAngle(0); // zero the tilt on exit
    kinect.close();
}

