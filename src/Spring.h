//
//  Spring.h
//
//  Created by Jakob Glock on 05/03/2017.
//
//

#ifndef SPRING_H
#define SPRING_H

/* Includes */
#include "ofMain.h"

/* This class is a basic spring class, it does all the calculations needed to determine the force
 * of a spring
*/

class Spring{
public:
    /* Constructor */
    Spring(ofVec2f _a, ofVec2f _p);
    
    /* Update, draw, etc. */
    ofVec2f update(ofVec2f _p);
    void draw();
    void calcCurrentLength();

    /* Getters and Setters */
    void setDoSpring(bool _doSpring);
    bool getDoSpring();

    /* Variables */
    ofVec2f anchor, weight;
    float len;
    bool doSpring;
};

#endif // SPRING_H
