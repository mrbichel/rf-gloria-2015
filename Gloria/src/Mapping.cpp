//
//  Mapping.cpp
//  Gloria
//
//  Created by Johan Bichel Lindegaard on 27/06/14.
//
//

#include "Mapping.h"

void Corner::addTriangleReference(InputTriangle* triangle) {
    bool found = false;
    for(int i=0;i<triangles.size();i++) {
        if(triangle == triangles[i]) {
            found = true;
        }
    }
    if(!found) {
        triangles.push_back(triangle);
    }
}

void Corner::createDivisionCorners(){
    for(int i=0;i<joinedCorners.size();i++){
        if(joinedCorners[i]->division.find(this) != joinedCorners[i]->division.end()){
            division[joinedCorners[i]] = joinedCorners[i]->division[this];
        } else {
            Corner * divisionCorner = new Corner;
            division[joinedCorners[i]] = divisionCorner;
            joinedCorners[i]->division[this] = divisionCorner;
            
            ofVec3f c2 = joinedCorners[i]->pos;
            ofVec3f c1 = pos;
            divisionCorner->anchorRatio = ofRandom(0.3,0.7);

            divisionCorner->pos = (c1 - c2)*divisionCorner->anchorRatio + c2;
            divisionCorner->origPos = (c1 - c2)*divisionCorner->anchorRatio + c2;
            divisionCorner->randomSeed = ofVec3f(ofRandom(-1,1),ofRandom(-1,1),ofRandom(-1,1));
            
            divisionCorner->joinedCorners.push_back(this);
            divisionCorner->joinedCorners.push_back(joinedCorners[i]);
            
            divisionCorner->anchor1 = this;
            divisionCorner->anchor2 = joinedCorners[i];
        }
        
    }
}

InputTriangle::InputTriangle() {
}

InputTriangle::~InputTriangle(){
    for(int i=0;i<3;i++){
        delete corners[i];
    }
}

ofVec3f InputTriangle::getCenter(){
    ofVec3f center = corners[0]->pos;
    center += corners[1]->pos;
    center += corners[2]->pos;
    center /= 3;
    return center;
}

void InputFixture::debugDraw() {
    
}

void Mapping::loadFixtures(string _xmlfile, string _svgfile) {
    
    fixtureXmlFilename = _xmlfile;
    fixtureSvgFilename = _svgfile;
    
    generateFixtures();
    
    // we dont really use the xml anyway

}


void Mapping::load(string _xmlfile, string _svgfile) {
    selectedCorner = NULL;
    selectedCornerId = 0;
    
    xmlFilename = _xmlfile;
    svgFilename = _svgfile;
    
    if(settings.loadFile(xmlFilename)) {
    } else {
      ofLogFatalError("Mapping")<<"Failed to load xml settings.";
    }
    
    generate();
    generateMask();
    
    if(settings.tagExists("corners")){
        settings.pushTag("corners");
        int numCornerTags = settings.getNumTags("corner");
        if(numCornerTags > 0){
            // populate depth on corners from xml
            for(int i = 0; i < numCornerTags; i++){
                
                settings.pushTag("corner", i);
                int uid = settings.getValue("uid", 0);
                for(int c = 0; c< corners.size(); c++) {
                    if(corners[c]->uid == uid) {
                        corners[c]->pos.z = settings.getValue("z", 0);
                    }
                }
                settings.popTag();
            }
        }
        settings.popTag();
    }
    
    updateMeshes();
    
    selectedCorner = corners[selectedCornerId];
}

void Mapping::save() {
    
    settings.clear();
    
    settings.addTag("corners");
    settings.pushTag("corners"); {
        for(int i=0; i< corners.size(); i++) {
        
            settings.addTag("corner");
            settings.pushTag("corner", i); {
                
                settings.addValue("uid", corners[i]->uid);
                settings.addValue("x", corners[i]->pos.x);
                settings.addValue("y", corners[i]->pos.y);
                settings.addValue("z", corners[i]->pos.z);
                
                settings.addTag("inTriangles");
                settings.pushTag("inTriangles"); {
                    for(int t=0; t< corners[i]->triangles.size(); t++) {
                        settings.addValue("uid", corners[i]->triangles[t]->uid);
                    }
                } settings.popTag();
                
                settings.addTag("joinedCorners");
                settings.pushTag("joinedCorners"); {
                    for(int j=0; j< corners[i]->joinedCorners.size(); j++) {
                        settings.addValue("uid", corners[i]->joinedCorners[j]->uid);
                    }
                } settings.popTag();
            
            } settings.popTag();
     
        }
    } settings.popTag();
    
    settings.addTag("triangles");
    settings.pushTag("triangles"); {
        
        for(int i=0; i< triangles.size(); i++) {
            settings.addTag("triangle");
            settings.pushTag("triangle", i); {
                settings.addValue("uid", triangles[i]->uid);
            }settings.popTag();
        }
        
    } settings.popTag();
    
    settings.saveFile(xmlFilename);
}

void Mapping::generateFixtures() {
    
    svg.load(fixtureSvgFilename);
    
    int svgHeight = svg.getHeight();
    int svgWidth  = svg.getWidth();
    
    int maxFixtureSize   = 1000000;
    int numFixtures      = 0;
    int numTriangles     = 0;
    
    for (int i = 0; i < svg.getNumPath(); i++){
        ofPath p = svg.getPathAt(i);
        
        // svg defaults to non zero winding which doesn't look so good as contours
        //p.setPolyWindingMode(OF_POLY_WINDING_ODD);
        const vector<ofPolyline>& lines = p.getOutline();
        
        for(int j=0;j<(int)lines.size();j++){
            
            if(lines[j].getArea() < maxFixtureSize) {
             
                InputFixture * fixture = new InputFixture;
                
                fixture->uid = numFixtures;
                fixture->polyline = lines[j];
                fixture->centroid = lines[j].getCentroid2D();
                
                fixture->rect = lines[j].getBoundingBox();
                
                fixtures.push_back(fixture);
                ++numFixtures;
                
            }
        }
    }    
}

void Mapping::generate() {
    
    // load the worldpoints
    
    /*int worldPointTags = XML.getNumTags("WORLDPOINT:PT");
     
     if(worldPointTags > 0){
     
     for(int i = 0; i < worldPointTags; i++){
     int x = XML.getValue("PT:X", 0, i);
     int y = XML.getValue("PT:Y", 0, i);
     int z = XML.getValue("PT:Z", 0, i);
     
     WorldPoint * wp = new WorldPoint();
     wp->pos.set(ofVec3f(x,y,z));
     mapping->worldPoints.push_back(wp);
     }
     
     }*/
    
    // load the svg (transforms should be flattened before)
    // todo - try to load this data from xml files
    
    svg.load(svgFilename);
    
    int numTriangles      = 0;
    int numCorners        = 0;
    int maxTriangleSize   = 1000000;
    float cornerThreshold = 10;
    
    int svgHeight = svg.getHeight();
    int svgWidth  = svg.getWidth();
    
    for (int i = 0; i < svg.getNumPath(); i++){
		ofPath p = svg.getPathAt(i);
        
		// svg defaults to non zero winding which doesn't look so good as contours
		//p.setPolyWindingMode(OF_POLY_WINDING_ODD);
		const vector<ofPolyline>& lines = p.getOutline();
        
		for(int j=0;j<(int)lines.size();j++){
            
			if(lines[j].getArea() < maxTriangleSize) {
                
                InputTriangle * triangle = new InputTriangle;
                
                triangle->uid = numTriangles;
                triangle->polyline = lines[j];
                triangle->centroid = lines[j].getCentroid2D();
                
                //cout<<"Creating triangle..."<<endl;
                // For each of 3 vertices in triangle create a corner pointer
                for(int vi=0; vi<3; vi++) {
                    
                    bool set = false;
                    ofVec2f vert = triangle->polyline.getVertices()[vi];
                    
                    // if we are scaling the output differently than the mapping
                    vert = vert / ofVec2f(svgWidth/OUTWIDTH, svgHeight/OUTHEIGHT);
                    
                    
                    
                    
                    // Loop through all corners in all triangels and set a pointer if the corner already exists
                    for(int ti=0; ti<triangles.size(); ti++) {
                        
                        for(int cti = 0; cti < 3; cti++) {
                            
                            if (vert.distance(triangles[ti]->corners[cti]->pos) < cornerThreshold) {
                                //cout<<"Setting corner: "<<vert.x<<", "<<vert.y<<endl;
                                triangle->corners[vi] = triangles[ti]->corners[cti];
                                triangle->corners[vi]->addTriangleReference(triangle);
                                set = true;
                            }
                        }
                    }
                    
                    if(!set) {
                        //cout<<"Creating corner: "<<vert.x<<", "<<vert.y<<endl;
                        
                        triangle->corners[vi] = new Corner;
                        triangle->corners[vi]->uid = numCorners;
                        ++numCorners;
                        triangle->corners[vi]->pos = vert;
                        
                        corners.push_back(triangle->corners[vi]);
                        triangle->corners[vi]->addTriangleReference(triangle);
                    }
                }
                
                triangle->color.set(ofRandom(100,255));
                
                // add as a mesh - maybe add a normal pointing out for light effects
                for(int c=0; c<3; c++) {
                    triangle->mesh.addVertex(triangle->corners[c]->pos);
                    triangle->mesh.addTexCoord(triangle->corners[c]->pos);
                }
                triangle->mesh.addTriangle(0, 1, 2);
                
                triangles.push_back(triangle);
                ++numTriangles;
            }
        }
	}
    
    for(int i=0;i<corners.size();i++){
        cout<<corners[i]->pos.x<<endl;
        for(int u=0 ; u<corners[i]->triangles.size() ; u++){
            for(int j=0;j<3;j++){
                if(corners[i]->triangles[u]->corners[j] != corners[i]){
                    bool alreadyAdded = false;
                    for(int k=0;k<corners[i]->joinedCorners.size();k++){
                        if(corners[i]->joinedCorners[k] == corners[i]->triangles[u]->corners[j]){
                            alreadyAdded = true;
                        }
                    }
                    if(!alreadyAdded){
                        corners[i]->joinedCorners.push_back(corners[i]->triangles[u]->corners[j]);
                    }
                }
            }
        }
    }
    
    ofLogNotice("Mapping")<<"Created: "<<triangles.size()<<" triangles with "<<corners.size()<<" unique corners"<<endl;
}


void Mapping::updateMeshes() {

    for(int i=0; i<triangles.size(); i++) {
        for(int c=0; c<3; c++) {
            triangles[i]->mesh.setVertex(c, triangles[i]->corners[c]->pos);
        }
    }
}

void Mapping::exit() {
    save();
}

void Mapping::drawGuide() {
}

void Mapping::debugDraw() {
    
    ofPushStyle();
    
    for(int i =0; i<triangles.size();i++) {
        //triangles[i]->debugDraw();
    }
    
    for(int i=0; i < corners.size(); i++) {
    }
    
    ofFill();
    ofSetColor(255, 0, 0);
    if(selectedCorner) {
        ofDrawCircle(selectedCorner->pos.x, selectedCorner->pos.y, 40);
    }
    
    ofPopStyle();

    
}

void InputTriangle::debugDraw() {
    ofSetColor(255, 255, 255, 40);
    ofDrawRectangle(polyline.getBoundingBox());
    
    //path.draw();
    //vector<ofPolyline>& lines = path.getOutline();
    //for(int j=0;j<(int)lines.size();j++){
    
    ofSetColor(255,255,255,200);
    polyline.draw();
    
    ofSetColor(255, 255, 255, 255);
    ofDrawBitmapString(ofToString(uid), centroid);
    
    ofSetColor(255, 0, 0, 60);
    
    for(int i=0; i<polyline.getVertices().size(); i++) {
        ofDrawCircle(polyline.getVertices()[i].x, polyline.getVertices()[i].y, 20);
    }
    
}


void Mapping::generateMask() {
    
    outputMask.allocate(OUTWIDTH, OUTHEIGHT);
    
    
    outputMask.begin(); {
        
        ofClear(0,0,0,0);
        ofFill();
        ofSetColor(0,0,0,255);
        for(int i=0; i< triangles.size(); i++) {
            triangles[i]->mesh.draw();
        }
        
    } outputMask.end();
    
}

void Mapping::drawMask() {
    // I guess we need to use a shader forr this - TODO
    outputMask.draw(0,0,outputMask.getWidth(), outputMask.getHeight());
}

void Mapping::nextCorner() {
    ++selectedCornerId;
    
    if(selectedCornerId > corners.size()-1) selectedCornerId = 0;
    cout<<selectedCornerId<<endl;
    selectedCorner = corners[selectedCornerId];
}


void Mapping::prevCorner() {
    
    --selectedCornerId;
    if(selectedCornerId < 0) selectedCornerId = corners.size()-1;
    cout<<selectedCornerId<<endl;
    selectedCorner = corners[selectedCornerId];
    
}

void Mapping::selectCornerAt(int _x, int _y, int r) {
    // todo translate to screen coordinates allow selection
    
    /*for(int i=0; i < corners.size(); i++) {
        if(corners[i]->pos.x - _x)
    }*/
}




