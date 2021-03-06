#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxDmx.h"
#include "ofxOsc.h"

#include "Lamp.h"

#define OSCSENDPORT 8000

#define OSCCLIENTONE "Rumba.local"
#define OSCCLIENTTWO "HalfdanJ-iPad.local"


class testApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    
    void loadXml();
    
    ofxXmlSettings XML;
    ofTrueTypeFont TTF;
    
    vector<Lamp> lamps;
    
    
    ofxOscSender * clientOne;
    ofxOscSender * clientTwo;
    
    vector<ofxOscSender *> clients;
    
    ofxDmx dmx;

    bool masterEnable = false;
    
    string message;
    
    
    ofEasyCam cam; // add mouse controls for camera movement
    
    ofxOscReceiver * osc;
    

};
