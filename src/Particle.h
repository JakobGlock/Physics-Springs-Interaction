//
//  Particle.h
//
//  Created by Jakob Glock on 05/03/2017.
//
//

#ifndef Particle_h
#define Particle_h

/* Includes */
#include "ofMain.h"
#include "Spring.h"

/* This is a particle class, which has various functions for playing around with some physics.
 * All my physics is based on newtonian physics. This class also includes a Spring class, that
 * is where I calculate my spring force.
*/

class Particle{
public:
    /* Constructor */
    Particle(ofVec2f _pos, ofColor _col, float _radius);

    /* Update, draw, etc. */
    void update();
    void draw();
    void resetPosition();
    void edges();
    bool isDead();
    bool isOffScreen();
    
    /* Physics */
    void addForce(ofVec2f f);
    void dampenForce();
    void resetForce();
    void resetVelocity();
    void addCvForce(ofVec2f f);
    void repulsionParticle(Particle *p, float _scl);
    void calcSpring();
    
    /* Getters and setters */
    ofVec2f getPosition();
    ofVec2f getOrigin();
    bool getIsFree();
    
    /* Create a spring object */
    Spring mySpring;
    
    /* Variables */
    ofVec2f pos, vel, frc, origin, lastPos;
    float radius, drawOffset;
    int life, maxLife, maxLifeOffset;
    ofColor col;
    bool doPhysics, isFree;
};

#endif /* Particle_h */
