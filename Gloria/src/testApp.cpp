#include "testApp.h"


void testApp::setup() {

    oscSender.setup("HalfdanJ.local", 6745);

    //    ofSetVerticalSync(true);
    ofSetLogLevel(OF_LOG_VERBOSE);

    ofSetFrameRate(60);

    ofSetWindowTitle("Obscure Glorious Control");

    syphonOut.setName("Gloria Generative");
    
    fboOut.allocate(OUTWIDTH, OUTHEIGHT);
    
    //fboOut.allocate(OUTWIDTH, OUTHEIGHT);
    ofEnableSmoothing();
    ofEnableAlphaBlending();

    // load the worldpoints

    XML.loadFile("calibration.xml");

    int worldPointTags = XML.getNumTags("WORLDPOINT:PT");

    if(worldPointTags > 0){

	for(int i = 0; i < worldPointTags; i++){

	    int x = XML.getValue("PT:X", 0, i);
	    int y = XML.getValue("PT:Y", 0, i);
	    int z = XML.getValue("PT:Z", 0, i);

	    WorldPoint * wp = new WorldPoint();
	    wp->pos.set(ofVec3f(x,y,z));
	    mapping.worldPoints.push_back(wp);

	}

    }

    // load the svg (transforms should be flattened before)


    // todo - try to load this data from xml files

    mapping.svg.load("input1.svg");

    int numTriangles = 0;
    int maxTriangleSize = 1000000;
    float cornerThreshold = 20;

    for (int i = 0; i < mapping.svg.getNumPath(); i++){
		ofPath p = mapping.svg.getPathAt(i);
		// svg defaults to non zero winding which doesn't look so good as contours
		//p.setPolyWindingMode(OF_POLY_WINDING_ODD);
		vector<ofPolyline>& lines = p.getOutline();

		for(int j=0;j<(int)lines.size();j++){

			if(lines[j].getArea() < maxTriangleSize) {

		InputTriangle * triangle = new InputTriangle;

		triangle->index = numTriangles;
		triangle->polyline = lines[j];
		triangle->centroid = lines[j].getCentroid2D();

		cout<<"Creating triangle..."<<endl;


		// For each of 3 vertices in triangle create a corner pointer
		for(int vi=0; vi<3; vi++) {

		    bool set = false;
		    ofVec2f vert = triangle->polyline.getVertices()[vi];

		    // Loop through all corners in all triangels and set a pointer if the corner already exists
		    for(int ti=0; ti<mapping.triangles.size(); ti++) {

			for(int cti = 0; cti < 3; cti++) {

			    if (vert.distance(mapping.triangles[ti]->corners[cti]->pos) < cornerThreshold) {
				cout<<"Setting corner: "<<vert.x<<", "<<vert.y<<endl;
				triangle->corners[vi] = mapping.triangles[ti]->corners[cti];

				triangle->corners[vi]->addTriangleReference(triangle);

				set = true;
			    }
			}
		    }

		    if(!set) {
			cout<<"Creating corner: "<<vert.x<<", "<<vert.y<<endl;

			triangle->corners[vi] = new Corner;
			triangle->corners[vi]->pos = vert;

			mapping.corners.push_back(triangle->corners[vi]);

			triangle->corners[vi]->addTriangleReference(triangle);
		    }

		}

		triangle->color.set(ofRandom(100,255));

		// add as a mesh - maybe add a normal pointing out for light effects
		for(int c=0; c<3; c++) {
		    triangle->mesh.addVertex(triangle->corners[c]->pos);
		}
		triangle->mesh.addTriangle(0, 1, 2);


		mapping.triangles.push_back(triangle);

		numTriangles += 1;

	    }
	}
	}


    for(int i=0;i<mapping.corners.size();i++){
	cout<<mapping.corners[i]->pos.x<<endl;
	for(int u=0 ; u<mapping.corners[i]->triangles.size() ; u++){
	    for(int j=0;j<3;j++){
		if(mapping.corners[i]->triangles[u]->corners[j] != mapping.corners[i]){
		    bool alreadyAdded = false;
		    for(int k=0;k<mapping.corners[i]->joinedCorners.size();k++){
			if(mapping.corners[i]->joinedCorners[k] == mapping.corners[i]->triangles[u]->corners[j]){
			    alreadyAdded = true;
			    cout<<"Already added"<<endl;
			}
		    }
		    if(!alreadyAdded){
			mapping.corners[i]->joinedCorners.push_back(mapping.corners[i]->triangles[u]->corners[j]);
		    }
		}
	    }
	}
    }



    cout<<endl<<"Created: "<<mapping.triangles.size()<<" triangles with "<<mapping.corners.size()<<" unique corners"<<endl;

    //setGUI();
    //gui->setDrawBack(true);
    //gui->loadSettings("GUI/guiSettings.xml");


    // effects scenes

 
    // Set up the scenes, all scenes is a subclass of SceneContent, don't call draw, setup and update directly it is taken care of thorugh the scene.
    
    lampWalker.mapping = &mapping;
    scenes.push_back(&lampWalker);
    
    quickTrail.mapping = &mapping;
    scenes.push_back(&quickTrail);
    
    for(int i=0; i<scenes.size(); i++) {
        scenes[i]->setupScene(OUTWIDTH, OUTHEIGHT);
    }
    

}

//--------------------------------------------------------------
void testApp::update() {


    // send some fun stuff to the sharpy army

    ofxOscMessage m;

    m.setAddress("/sharpy");
    m.addIntArg(0); // device number
    m.addFloatArg(sin(ofGetElapsedTimeMillis()/4500.) * 8.); // x
    m.addFloatArg(0); // y
    m.addFloatArg(sin(ofGetElapsedTimeMillis()/4000.) * 2.); // z
    oscSender.sendMessage(m);

    ofxOscMessage m2;
    m2.setAddress("/sharpy");
    m2.addIntArg(1); // device number
    m2.addFloatArg(sin(ofGetElapsedTimeMillis()/4500.) * 8.); // x
    m2.addFloatArg(0); // y
    m2.addFloatArg(sin(ofGetElapsedTimeMillis()/4000.) * 2.); // z
    oscSender.sendMessage(m2);


    // Scenes
    
    for(int i=0; i<scenes.size(); i++) {
        scenes[i]->updateScene();
    }

    

}


void testApp::draw() {
    ofBackground(0);
    
    /* Move this to a scene
    ofSetLineWidth(2);
    // waves going across red
    for(int i =0; i<mapping.triangles.size();i++) {
        ofSetColor( ofNoise(mapping.triangles[i]->centroid.y/600 - ofGetElapsedTimef()/2)*255, 100, 100);
        mapping.triangles[i]->mesh.draw();

        ofSetColor( ofNoise(mapping.triangles[i]->centroid.x/600 - ofGetElapsedTimef()/2) *255, 90, 90 );
        mapping.triangles[i]->mesh.drawWireframe();
    }
    */


    for(int i=0; i<scenes.size(); i++) {
        scenes[i]->drawScene();
        // TODO: Layer ordering
    }
        
    
    fboOut.begin();
    
    ofClear(0, 0);
    
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_ONE, GL_ONE);
    
    for(int i=0; i<scenes.size(); i++) {
        ofSetColor(255,255,255,scenes[i]->opacity);
        scenes[i]->fbo.draw(0,0);
    }
    
    //glDisable(GL_BLEND);
    
    fboOut.end();
    
    syphonOut.publishTexture(&fboOut.getTextureReference());
    
    ofPushMatrix();
    ofScale(0.4, 0.4);
    ofBackground(0);
    
    ofSetColor(255,255,255,96);
    drawGrid();
    ofSetColor(255,255,255,255);
    
    fboOut.draw(0, 0);
    
    ofPopMatrix();

    ofDrawBitmapString(ofToString(ofGetFrameRate()), 20, 20);
}


void InputTriangle::debugDraw() {
    ofFill();
    ofSetColor(255, 255, 255, 40);
    ofRect(polyline.getBoundingBox());

    //path.draw();
    //vector<ofPolyline>& lines = path.getOutline();
    //for(int j=0;j<(int)lines.size();j++){

    ofFill();
    ofSetColor(255,255,255,200);
    polyline.draw();

    ofSetColor(255, 255, 255, 255);
    ofDrawBitmapString(ofToString(index), centroid);

    ofSetColor(255, 0, 0, 60);
    for(int i=0; i<polyline.getVertices().size(); i++) {
        ofCircle(polyline.getVertices()[i].x, polyline.getVertices()[i].y, 20);
    }
}


//------------------------------------------------------------
void testApp::debugDraw() {
    for(int i =0; i<mapping.triangles.size();i++) {
        mapping.triangles[i]->debugDraw();
    }
}

void testApp::drawGrid() {
    for(int i =0; i<mapping.triangles.size();i++) {
        ofSetLineWidth(1);
        mapping.triangles[i]->mesh.drawWireframe();
    }
}


void testApp::guiEvent(ofxUIEventArgs &e)
{
}


//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){

}

//--------------------------------------------------------------
void testApp::exit()
{
    //gui->saveSettings("GUI/guiSettings.xml");
    //delete gui;s

    XML.saveFile("calibration.xml");
}

void testApp::setGUI()
{

    float xInit = OFX_UI_GLOBAL_WIDGET_SPACING;
    float length = 255-xInit;

    gui = new ofxUICanvas(0, 0, length+xInit, ofGetHeight());

    ofAddListener(gui->newGUIEvent,this,&testApp::guiEvent);
}