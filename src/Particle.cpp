//
//  Particle.cpp
//
//  Created by Jakob Glock on 05/03/2017.
//
//

#include "Particle.h"

//--------------------------------------------------------------
/* Calling spring constructor here and passing in variables */
Particle::Particle(ofVec2f _pos, ofColor _col, float _radius) : mySpring(_pos, _pos){
    /* Set some variables */
    col = _col;
    radius = _radius;
    drawOffset = radius / 2;
    life = 0;
    doPhysics = true;
    isFree = false;
    maxLife = ofRandom(1000, 5000);
    maxLifeOffset = ofRandom(250, 2000);
    
    /* Offset the position by half the size so that the draw point is in the center of the square */
    pos.set(_pos.x, _pos.y);
    origin.set(pos.x, pos.y);
    vel.set(0, 0);
    frc.set(0, 0);
    lastPos.set(pos.x, pos.y);
}

//--------------------------------------------------------------
void Particle::update(){

    if(!mySpring.getDoSpring())
    {
        /* Only check if we have collided with the edges if we are not calculating the spring */
        edges();
    }

    /* Calculate the spring force */
    calcSpring();

    if(doPhysics)
    {
        /* Calculate the physics, basic newtonian physics */
        vel += frc; // Add force to velocity
        pos += vel; // Add velocity to position
    }
}

//--------------------------------------------------------------
void Particle::calcSpring(){

    if(mySpring.getDoSpring())
    {
        /* Calculate a spring force */
        frc += mySpring.update(pos);

        /* Calculate how far the point is away from the origin of the spring */
        mySpring.calcCurrentLength();
    }
    else
    {
        /* Set isFree to true */
        isFree = true;
    }
}

//--------------------------------------------------------------
void Particle::addForce(ofVec2f f){
    /* Add a user defined force */
    frc += f;
}

//--------------------------------------------------------------
void Particle::dampenForce(){
    /* Dampen the force */
    frc.x = frc.x - vel.x * 0.01f;
    frc.y = frc.y - vel.y * 0.01f;
}

//--------------------------------------------------------------
/* Calculates a force from the openCV optical flow velocities that are passed in */
void Particle::addCvForce(ofVec2f f){
    
    /* Vector to store a new force, default no force */
    ofVec2f s;
    s.set(0, 0);
    
    /* Vector to store the incoming force from the optical flow */
    ofVec2f hitForce;
    hitForce.set(f.x, f.y);

    /* Get the length of that vector and limit the length */
    float aLen = ofClamp(hitForce.length(), 0, 0.3);

    if(aLen >= 0.1)
    {
        /* Calculate a force */
        s.x += 0.1 * aLen * hitForce.x;
        s.y += 0.1 * aLen * hitForce.y;
    }
    
    /* Add that new force */
    frc += s;
    
}

//--------------------------------------------------------------
/* This function handles reseting the particles to there original positions */
void Particle::resetPosition(){

    /* Increase life */
    life++;

    if(life < maxLife)
    {
        /* Update the last position */
        lastPos.set(pos.x, pos.y);
    }
    else if(life < maxLife + maxLifeOffset)
    {
        /* Set do physics to false */
        doPhysics = false;

        /* Map life to a value between 0 and 1 so it can be used to lerp between two values */
        float lerpAmt = ofMap(life, maxLife, maxLife + maxLifeOffset, 0, 1, true);

        /* Lerp from the last position saved to the origin point, makes the grid again */
        float lerpX = ofLerp(lastPos.x, origin.x, lerpAmt);
        float lerpY = ofLerp(lastPos.y, origin.y, lerpAmt);

        /* Set the position of the particle to lerped values */
        pos.set(lerpX, lerpY);
    }
    else if(life == maxLife + maxLifeOffset)
    {
        /* Reset the velocity and force */
        resetVelocity();
        resetForce();

        /* Make sure the position is equal to the origin */
        pos.set(origin.x, origin.y);

        /* Set doPhysics back to true */
        doPhysics = true;

        /* Enable calculation of the spring force */
        mySpring.setDoSpring(true);
    }
    else
    {
        /* Set is free back to false */
        isFree = false;
        
        /* Reset life back to zero */
        life = 0;
    }
}

void Particle::resetVelocity(){
    /* Set the velocity to zero */
    vel.set(0, 0);
}

//--------------------------------------------------------------
void Particle::resetForce(){
    /* Set the force to zero */
    frc.set(0, 0);
}

//--------------------------------------------------------------
ofVec2f Particle::getPosition(){
    /* Return the position vector */
    return pos;
}

//--------------------------------------------------------------
ofVec2f Particle::getOrigin(){
    /* Return the origin vector */
    return origin;
}

//--------------------------------------------------------------
bool Particle::getIsFree(){
    /* Return isFree boolean */
    return isFree;
}

//--------------------------------------------------------------
/* Check to see if a particle has collided with the edges of the screen */
void Particle::edges(){
    /* Bouncing off the edges */
    if(pos.x < 0)
    {
        pos.x = 0; //Stops the object from getting caught
        frc.x = frc.x - vel.x * 0.01f; // Apply some dampening when they collide
        vel.x *= -1; // Reverse the velocity
    }
    else if(pos.x > ofGetWidth() - radius)
    {
        pos.x = ofGetWidth() - radius;
        frc.x = frc.x - vel.x * 0.01f;
        vel.x *= -1;
    }
    else if(pos.y < 0)
    {
        pos.y = 0;
        frc.y = frc.y - vel.y * 0.01f;
        vel.y *= -1;
    }

    else if(pos.y > ofGetHeight() - radius)
    {
        pos.y = ofGetHeight() - radius;
        frc.y = frc.y - vel.y * 0.01f;
        vel.y *= -1;
    }

}

/* I didn't end up using these functions below, but Im leaving them in for future development */
//--------------------------------------------------------------
void Particle::repulsionParticle(Particle *p, float _scl){
    
    /* Store the position of the other particle here */
    ofVec2f otherPos = ofVec2f(p->pos.x, p->pos.y);
    
    /* Get the difference between them */
    ofVec2f dif = pos - otherPos;
    
    /* Get the length of the vector */
    float length = dif.length();
    
    /* Check if the length is less than the radius */
    if(length < radius)
    {
        /* Normalize the vector */
        dif.normalize();
        
        /* Add the force to this particle and scale it */
        frc.x += dif.x * _scl;
        frc.y += dif.y * _scl;
        
        /* Add the force to the other particle, but in the other direction, hence the subtract and scale it */
        p->frc.x -= dif.x * _scl;
        p->frc.y -= dif.y * _scl;
    }
}

//--------------------------------------------------------------
/* Draws a particle, for debugging */
void Particle::draw(){
    ofSetColor(255);
    ofDrawCircle(pos.x, pos.y, radius);
}

//--------------------------------------------------------------
bool Particle::isOffScreen(){
    /* If the object is off the screen return true, else return false */
    if(pos.x < 0 || pos.x > ofGetWidth() || pos.y < 0 || pos.y > ofGetHeight())
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------
bool Particle::isDead(){
    /* If the radius is less than zero return true, else return fals */
    if(radius <= 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
