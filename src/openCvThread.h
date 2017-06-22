//
//  openCvThread.h
//
//  Created by Jakob Glock on 15/03/2017.
//
//

/* -This is a seperate thread to the main one, this calculates the optical flow
 *  using a seperate thread and is more effiecent as it will not get in the way
 *  of drawing my scene but freeing up cpu in the main thread.
 *
 * -Optical Flow with minor adjustments, taken from the 'Camera Controller' example
 *  from 'Term 2 of Workshops in Creative Coding'.
 */

#pragma once

/* Includes */
#include "ofMain.h"
#include "ofThread.h"
#include "ofxOpenCV.h"

/* Set namespace to cv */
using namespace cv;

/* This class inherits from ofThread */
class openCvThread : public ofThread {
    
public:
    
    bool drawn;
    
    /* Create a video grabber */
    ofVideoGrabber cam;
    ofPixels camImage;
    
    bool isNew, calculatedFlow;
    float decimate; // Decimate is global
    
    ofxCvColorImage currentColor;		//First and second original images
    ofxCvGrayscaleImage gray1, gray2;	//Decimated grayscaled images
    ofxCvFloatImage flowX, flowY;		//Resulted optical flow in x and y axes
    
    //--------------------------------------------------------------
    openCvThread() {
        
        drawn = false;
        
        /* Variables for width, height and decimate */
        int camW = ofGetWidth();
        int camH = ofGetHeight();
        decimate = 0.25;
        
        /* Setup webcam and dont use texture */
        //cam.setDesiredFrameRate(60);
        cam.initGrabber(camW, camH);
        cam.setUseTexture(false);
        
        /* By default the flow has not been calcuated */
        calculatedFlow = false;
        
        /* Seperate threads to the main one cannot use OpenGl, so we disable the use of textures which will turn off all GL calls */
        currentColor.setUseTexture(false);
        
        /* Then we allocate space, if you do not allocate space, the program will crash */
        currentColor.allocate(camW, camH);
        
        /* We allocate the right amount of space, so we know these will be smaller so we use the decimate varibale */
        gray1.setUseTexture(false);
        gray1.allocate(camW * decimate, camH * decimate);
        gray2.setUseTexture(false);
        gray2.allocate(camW * decimate, camH * decimate);
        flowX.setUseTexture(false);
        flowX.allocate(camW * decimate, camH * decimate);
        flowY.setUseTexture(false);
        flowY.allocate(camW * decimate, camH * decimate);
    }
    
    //--------------------------------------------------------------
    void threadedFunction() {
        while(isThreadRunning()) {
            
            /* Update the webcam pixels */
            cam.update();
            
            if(cam.isFrameNew() && isNew && drawn)
            {
                if ( gray1.bAllocated ) {
                    gray2 = gray1;
                    calculatedFlow = true;
                }
                
                //Convert to ofxCv images
                currentColor.setFromPixels(cam.getPixels());
                
                /* Flip the image */
                currentColor.mirror(false, true);
                
                /* Save the webcam image in an ofPixels so I can access it outside the thread and draw it */
                camImage = currentColor.getPixels();
                
                ofxCvColorImage imageDecimated1;
                
                /* Do not use a texture */
                imageDecimated1.setUseTexture(false);
                
                imageDecimated1.allocate(currentColor.width * decimate, currentColor.height * decimate);
                imageDecimated1.scaleIntoMe(currentColor, CV_INTER_AREA);             //High-quality resize
                gray1 = imageDecimated1;
                
                if (gray2.bAllocated){
                    Mat img1(gray1.getCvImage());  //Create OpenCV images
                    Mat img2(gray2.getCvImage());
                    Mat flow;                        //Image for flow
                    //Computing optical flow (visit https://goo.gl/jm1Vfr for explanation of parameters)
                    calcOpticalFlowFarneback(img1, img2, flow, 0.7, 3, 11, 5, 5, 1.1, 0);
                    //Split flow into separate images
                    vector<Mat> flowPlanes;
                    split(flow, flowPlanes);
                    //Copy float planes to ofxCv images flowX and flowY
                    IplImage iplX(flowPlanes[0]);
                    flowX = &iplX;
                    IplImage iplY(flowPlanes[1]);
                    flowY = &iplY;
                }
            }
        }
    }
};
