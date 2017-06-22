//////////////////////////////////////////////////////////
// Physics, Springs and Interaction                     //
// -By Jakob Glock                                      //
// -2017                                                //
//                                                      //
// -Created for the module Workshops in Creative Coding //
//  Term Two @ Goldsmiths University.                   //
//                                                      //
// -This program uses Optical Flow as a control source  //
//  allowing the user to interact with a physics based  //
//  system through a webcam                             //
//                                                      //
// -Depending on your computer, this program might run  //
//  better if built in release, try both                //
//                                                      //
// -This program uses the addon ofxOpenCV               //
//                                                      //
//////////////////////////////////////////////////////////

#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    /* General setup */
    ofSetWindowShape(960, 720);
    ofSetVerticalSync(true);
    ofBackground(0);
    
    /* Set the mesh draw mode to points */
    sceneMesh.setMode(OF_PRIMITIVE_POINTS);

    /* Set the size of the points */
    glPointSize(3);
    
    /* Variables for drawing a grid */
    float gridSize = 120;
    float xStep = ofGetWidth() / gridSize;
    float yStep = ofGetHeight() / gridSize;
    float offSetX = xStep / 2;
    float offSetY = yStep / 2;
    float radius = 1;
    
    /* Nested loop for creating my objects in a grid */
    for(int i=0; i<gridSize; i++){
        for(int j=0; j<gridSize; j++){
            /* Vector to store points and then calculate the positions */
            ofVec2f p;
            p.x = xStep * i + offSetX;
            p.y = yStep * j + offSetY;

            /* Add an element to the particles vector, we use new when its a vector of pointers */
            myParticles.push_back(new Particle(p, ofColor(255), radius));

            /* Get the position of that particle and add it to the mesh and add a color */
            sceneMesh.addVertex(myParticles[i * gridSize + j]->getPosition());
            sceneMesh.addColor(ofColor(0, 0, 0, 255));
        }
    }

    /* Calculate 50% of the total number of particles */
    resetPercent = myParticles.size() * 0.5;
    
    /* Allocate some space for my fbo and clear it of junk, this is to draw my scene in */
    scene.allocate(ofGetWidth(), ofGetHeight(), GL_RGB);
    scene.begin();
    ofClear(0,0,0);
    scene.end();

    /* Allocating space for ofPixels object, this has to be the same size as the optical Flow image,
     * seems to throw a warning if I don't allocate space and set the channels first
    */
    camPix.allocate(ofGetWidth()*0.25, ofGetHeight()*0.25, OF_PIXELS_RGB);
    
    /* By default readFlowField and resetParticles is set to false */
    readFlowField = false;
    resetParticles = false;

    /* Start my custom thread */
    thread.startThread();

}

//--------------------------------------------------------------
void ofApp::update(){

    ////////////////////////////////////////////////////////////
    // Seperate Thread Start

    /* Lock */
    thread.lock();
    
    /* Set isNew to true */
    thread.isNew = true;
    
    /* If the flow has been caculated in the thread */
    if (thread.calculatedFlow)
    {
        /* Set some variables in the the main thread from my thread */
        w = thread.gray1.width; // Width
        h = thread.gray1.height; // Height
        
        /* Get the optical flow from my thread */
        flowXPixels = thread.flowX.getPixelsAsFloats();
        flowYPixels = thread.flowY.getPixelsAsFloats();
        
        /* Store the webcam image in an ofPixels */
        camPix = thread.camImage;
        
        /* Set readFlowField to true */
        readFlowField = true;

    }
    
    /* Unlock */
    thread.unlock();

    // Seperate Thread End
    ////////////////////////////////////////////////////////////
    
    

    /* Variable to store how many particles are disconnected from there spring */
    int freeParticleCount = 0;
  
    /* Only read from flowPixels arrays if they have something added to them, basically im waiting 
     * for the other thread to calculate something otherwise im reading from an empty array and that
     * is not good!
     */
    if(readFlowField)
    {
        ////////////////////////////////////////////////////////////
        // Update Particles Start

        /* Loop over the particles array */
        for(int i=0; i<myParticles.size(); i++){
            
            /* Reset the force each frame */
            myParticles[i]->resetForce();
            
            /* Scale the particle positions to equal the optical flow dimensions, very important! */
            ofVec2f p = myParticles[i]->getPosition() * 0.25;
            
            /* Variable for storing the force to apply */
            ofVec2f f;

            /* Set the default force to nothing */
            f.set(0,0);
            
            /* Convert the particle position to an int so we can use it to read from an array */
            int fieldPosX = (int)p.x;
            int fieldPosY = (int)p.y;
            
            /* This is for safety, just to make sure we don't step outside the array at any point */
            fieldPosX = MAX(0, MIN(fieldPosX, w-1));
            fieldPosY = MAX(0, MIN(fieldPosY, h-1));
            
            /* Get the position in the array */
            int pos = fieldPosY * w + fieldPosX;

            /* Read the value from the arrays and reverse the direction */
            f.set(flowXPixels[pos] * -1, flowYPixels[pos] * -1);
            
            /* Add a global force to the particles, eg. Gravity */
            myParticles[i]->addForce(ofVec2f(0, 0.004));

            /* Add the force from the optical flow */
            myParticles[i]->addCvForce(f);
            
            /* Dampen the force */
            myParticles[i]->dampenForce();
            
            /* Update the particle */
            myParticles[i]->update();

            /* Here I am counting how many particles are free from the spring */
            if(myParticles[i]->getIsFree())
            {
                freeParticleCount++;

                /* If the value is over a certian percentage then it sets a varibale to true */
                if(freeParticleCount > resetPercent)
                {
                    resetParticles = true;
                }
                else if(freeParticleCount == 0)
                {
                    /* Pick a random reset percentage, this adds slight variation */
                    resetPercent = myParticles.size() * ofRandom(0.3, 0.7);

                    /* If the total number is zero it sets that variable to false */
                    resetParticles = false;
                }
            }
        }

        /* Loop through the particles again, so that I can call every particles resetPosition function,
         * not just the ones above freeParticleCount
         */

        for(int i=0; i<myParticles.size(); i++){

            /* If the particle is currently free from its spring and the global reset to true then
             * run the reset function
             */

            if(myParticles[i]->getIsFree() == true && resetParticles == true)
            {
                myParticles[i]->resetPosition();
            }
        }

        // Update Particles End
        ////////////////////////////////////////////////////////////



        ////////////////////////////////////////////////////////////
        // Update Mesh Start
        
        /* Loop over the mesh */
        for(int i=0; i<sceneMesh.getNumVertices(); i++){
            
            /* Get the vertex at a specific index */
            ofVec3f p = sceneMesh.getVertex(i);
            
            /* Update that vertex with the corresponding position from myParticles */
            p = myParticles[i]->getPosition();
            
            /* Set the vertex at the current index with the new position */
            sceneMesh.setVertex(i, p);
            
            /* Get the origin point of the particle */
            ofVec2f o = myParticles[i]->getOrigin();
            
            /* Get the color of the pixel relative to the origin point from the webcam image that is store in an ofPixels */
            ofColor col = camPix.getColor((int)o.x, (int)o.y);
            
            /* Update the color in the mesh for this vertex */
            sceneMesh.setColor(i, col);
        }

        // Update Mesh End
        ////////////////////////////////////////////////////////////

    }
    
    ////////////////////////////////////////////////////////////
    // Scene Fbo Start
    
    /* Draw to an Fbo */
    scene.begin();
    
    /* Clear the fbo each frame */
    ofClear(0, 0, 0);
    
    /* Draw the mesh into the fbo */
    sceneMesh.draw();
    
    /* Close the fbo */
    scene.end();
    
    // Scene Fbo End
    ////////////////////////////////////////////////////////////
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    /* Set a color and draw the scene fbo */
    ofSetColor(255, 255, 255);
    scene.draw(0,0);

//    /* For debugging FrameRate and Amount of Particles */
//    ofSetColor(255, 0, 0);
//    ofDrawBitmapString("FrameRate: " + ofToString(ofGetFrameRate()), 10, 10);
//    ofDrawBitmapString("NumParticles: " + ofToString(myParticles.size()), 10, 20);
    
    /* Once drawn then calculate the next frame of the optical flow */
    thread.lock();
    thread.drawn=true;
    thread.unlock();

}

//--------------------------------------------------------------
// Stop the thread when exiting the application
void ofApp::exit(){
    thread.stopThread();
}
