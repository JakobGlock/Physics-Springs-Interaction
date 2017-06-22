#pragma once

/* Includes */
#include "ofMain.h"
#include "openCvThread.h"
#include "Particle.h"

class ofApp : public ofBaseApp{

public:

    /* General functions */
    void setup();
    void update();
    void draw();
    void exit();
    
    /* Create an instance of my thread which does the OpenCV calculations */
    openCvThread thread;
    
    /* Fbo to draw my scene to and ofPixels to store webcam data */
    ofFbo scene;
    ofPixels camPix;
    
    /* A mesh to draw my points, this is way faster than using 'ofDrawCircle()' */
    ofMesh sceneMesh;
    
    //Optical flow
    float *flowXPixels;
    float *flowYPixels;
    int w, h;
    
    /* Vector of pointers */
    vector<Particle*> myParticles;

    /* Boolean to tell my program when to read the optical flow */
    bool readFlowField, resetParticles;
    int resetPercent;

};
