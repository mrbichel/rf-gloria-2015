//
//  Gloria
//
// Vibeke Bertelsen 2014
//

#include "CurlyFur.h"

void CurlyFur::setup(){
    
    myfbo.allocate(OUTWIDTH/10, OUTHEIGHT/10, GL_RGBA);
    
    plength = 10;
    density = 50;
    totalCount = 1000;
    fade = 5;
    
}

void CurlyFur::update(){
    
    //ofSeedRandom();
    
    for (int i=0; i<particles.size(); i++) {
        particles[i].update();
    }
}

void CurlyFur::draw(){;
    float lineWidth = 5;
    
    ofEnableAlphaBlending();
    
    //Draw particles
    for (int i=0; i<particles.size(); i++) {
        Particle &p = particles[i];
        p.draw(lineWidth);
    }
    
    for (int i=0; i<density; i++) {
        createParticle();
    }
    
    while (particles.size()>totalCount) {
        particles.erase(particles.begin());
    }
    
    //Fade out the old stuff
    ofSetColor(0,0,0,fade);
    ofFill();
    ofDrawRectangle(0,0,OUTWIDTH,OUTHEIGHT);
    
    ofDisableAlphaBlending();
    
    myfbo.begin();
    ofSetColor(255, 255, 255, 255);
    syphonIn->draw(0, 0, OUTWIDTH/10,OUTHEIGHT/10);
    myfbo.end();
    
    //myfbo.draw(0,0);
}


void CurlyFur::createParticle() {
    
    
    float randomX = ofRandom(1.0);
    float randomY = ofRandom(1.0);
    
    ofPixels pixels;
    Particle p;
    p.setup(plength);
    p.pos.set(OUTWIDTH * randomX, OUTHEIGHT * randomY);
    
    myfbo.readToPixels(pixels);
    p.color = pixels.getColor(p.pos.x/10,p.pos.y/10);
    
    particles.push_back(p);
}
