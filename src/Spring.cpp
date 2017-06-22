//
//  Spring.cpp
//
//  Created by Jakob Glock on 05/03/2017.
//
//

#include "Spring.h"

//--------------------------------------------------------------
Spring::Spring(ofVec2f _a, ofVec2f _p){
    /* Set default values for variables */
    anchor.set(_a.x, _a.y);
    weight.set(_p.x, _p.y);
    len = 0;
    doSpring = true;
}

//--------------------------------------------------------------
/* This function returns a vector of the calculated force */
ofVec2f Spring::update(ofVec2f _p){
    
    /* Update the current position of the end of the spring */
    weight.set(_p.x, _p.y);
    
    /* get the different between the anchor point of the spring and the other end */
    ofVec2f newForce = weight - anchor;
    
    /* Get the length of that vector */
    float d = newForce.length();
    
    /* Calculate the difference between that and the length of the spring */
    float stretched = d - len;
    
    /* Normalize the newForce vector */
    newForce.normalize();
    
    /* Multiply that vector by the amount stretched, also reverse it and scale it */
    newForce *= (-0.01 * stretched);
    
    /* Return the newForce */
    return newForce;
}

//--------------------------------------------------------------
/* This function returns the current distance from the anchor to the end of the spring */
void Spring::calcCurrentLength(){
    float currentLength = ofDist(weight.x, weight.y, anchor.x, anchor.y);

    if(currentLength > 125)
    {
        doSpring = false;
    }
}

//--------------------------------------------------------------
void Spring::setDoSpring(bool _doSpring){
    /* Sets doSpring boolean */
    doSpring = _doSpring;
}

//--------------------------------------------------------------
bool Spring::getDoSpring(){
    /* Returns doSpring boolean */
    return doSpring;
}

//--------------------------------------------------------------
/* Draw the spring, for debugging */
void Spring::draw(){
    ofSetColor(255);
    ofDrawLine(anchor, weight);
}
