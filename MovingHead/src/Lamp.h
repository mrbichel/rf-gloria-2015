//
//  Lamp.h
//  emptyExample
//
//  Created by Jonas Jongejan on 29/06/13.
//
//

#ifndef __emptyExample__Lamp__
#define __emptyExample__Lamp__

#include "ofMain.h"

class Lamp {
    
public:
    ofVec3f position;
    ofVec3f dest;
    
    int dmxPanMin;
    int dmxPanMax;
    
    int dmxTiltMin;
    int dmxTiltMax;
    
    float panMin;
    float panMax;

    float tiltMin;
    float tiltMax;
    

    
    int dmxAddress;
    
    void pointAt(ofVec3f dest);
    void setRotation(float pan, float tilt);
    
    float getTilt();
    float getPan();
    
    float getDmxTilt();
    float getDmxPan();
    

    float tilt;
    float pan;
};



#endif /* defined(__emptyExample__Lamp__) */
