#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

void ofApp::setup() {
    
    // General
    myfont.load("ofxbraitsch/fonts/Verdana.ttf", 12);
    
    // CCV
    ccv.setup("image-net-2012.sqlite3");  // download
    ccv.setEncode(true);
    ccv.start();    
    
    //ofParameters
    learningTime.set("Learning Time", 20, 0, 30);
    thresholdValue.set("Threshold Value", 2.4, 0, 255);
    minArea.set("Min area", 11.5, 1, 100);
    maxArea.set("Max area", 200, 1, 500);
    threshold.set("Threshold", 5, 0, 255);
    blurValue.set("blurValue", 3, 0, 10);
    holes.set("Holes", false);
    
    // datGUI
    // Menu
    gui1 = new ofxDatGui();
    // gui1->addLabel("Settings");
    // gui1->addFRM();
    // gui1->addBreak();
    // gui1->addToggle("Pause");
    gui1->addButton("Live feed");
    // CCV folder
    ofxDatGuiFolder* ccvFolder = gui1->addFolder("CCV Settings", ofColor::blueSteel);
    // ccvFolder->addLabel("Filters");
    vector<string> layerNames = ccv.getLayerNames();
    for (int i=0; i<layerNames.size(); i++) {
        ccvFolder->addButton(layerNames[i]);
    }
    ccvFolder->setVisible(true);
    ccvFolder->expand();
    // CV folder
    ofxDatGuiFolder* cvFolder = gui1->addFolder("CV Settings", ofColor::white);
    cvFolder->addButton("Reset Background");
    cvFolder->addSlider(learningTime);
    cvFolder->addSlider(thresholdValue);
    cvFolder->addSlider(minArea);
    cvFolder->addSlider(maxArea);
    cvFolder->addSlider(threshold);
    cvFolder->addSlider(learningTime);
    // cvFolder->expand();
    cvFolder->setVisible(true);
    //
    gui1->setPosition(0, 400);
    
    //
    gui1->onToggleEvent(this, &ofApp::onToggleEvent);
    gui1->onButtonEvent(this, &ofApp::onButtonEvent);
    gui1->setTheme(new ofxDatGuiThemeCharcoal());
    
    //
    showLabels = true;
    movieIsPlaying = true;
    showGui = false;
    showCcv = false;
    
    //
    #ifdef _USE_LIVE_VIDEO
        cam.setup(640, 480);  // not tested
    #else
        movie.load("Clip.MOV");  // 1280x720
        movie.play();
    #endif
    
    // openCV
    // wait for half a frame before forgetting something
    contourFinder.getTracker().setPersistence(15);
    // an object can move up to 32 pixels per frame
    contourFinder.getTracker().setMaximumDistance(32);
    contourFinder.setSimplify(true);
    background.setDifferenceMode(ofxCv::RunningBackground::DARKER); //ABSDIFF, BRIGHTER, DARKER
    
}

void ofApp::update() {

    //
    gui1->setWidth(400);
    movie.update();
    
    // CCV
    if (showCcv && movie.isFrameNew() && ccv.isReady()) {
        ccv.update(activeObject, ccv.numLayers());
        pauseMovie();
    }
    
    // openCV
    if(resetBackground) {
        background.reset();
        resetBackground = false;
    }
	if(movie.isFrameNew()) {
        blur(movie, blurValue);
        //
        background.setLearningTime(learningTime);
        background.setThresholdValue(thresholdValue);
		background.update(movie, thresholded);
		thresholded.update();
        //
        contourFinder.setMinAreaRadius(minArea);
        contourFinder.setMaxAreaRadius(maxArea);
        contourFinder.setThreshold(threshold);
        contourFinder.findContours(background.getForeground());
        contourFinder.setFindHoles(holes);
	}

}

void ofApp::draw() {
    
    ofBackground(8);

    // Draw the current object
    activeObject.setFromPixels(activeObjectPixels);
    activeObject.draw(0,0,gui1->getWidth(),gui1->getWidth());
    
    // set scale + translate
    float scale = 1.6;
    float translateX = gui1->getWidth();
    float translateY = 0;
    
    // Draw the cam + openCV
    ofPushMatrix();
    ofTranslate(translateX, translateY);
    ofScale(scale, scale);
    //
    movie.draw(0, 0);

    // ofSetBackgroundAuto(showLabels);
    RectTracker& tracker = contourFinder.getTracker();
    if(showLabels && !showCcv) {
        for(int i = 0; i < contourFinder.size(); i++) {
            // get bounding rect texture
            cv::Rect boundRect = contourFinder.getBoundingRect(i);
            cv::Mat matROI;
            cv::Mat sourceMat = toCv(movie);
            sourceMat(boundRect).copyTo(matROI);

            // draw polyline contour
            ofPolyline pol = contourFinder.getPolyline(i);
            ofPushStyle();
            ofSetColor(255);
            ofNoFill();
            pol.draw();
            // ofBeginShape();
            // for( int i = 0; i < pol.getVertices().size(); i++) {
            //     ofVertex(pol.getVertices().at(i).x, pol.getVertices().at(i).y);
            // }
            // ofEndShape();
            
            // compare mouse position with bounding boxes and save the active pixels
            if (toOf(contourFinder.getBoundingRect(i)).inside((mouseX-gui1->getWidth())/scale, mouseY/scale)) {
                copy(matROI, activeObjectPixels);
                ofSetColor(255,100);
            } else {
                ofSetColor(255,30);
            }
            
            // draw bounding rect
            ofFill();
            ofDrawRectangle(toOf(contourFinder.getBoundingRect(i)));
            ofSetColor(255,100);
            ofNoFill();
            ofDrawRectangle(toOf(contourFinder.getBoundingRect(i)));
            ofPopStyle();
            // draw label
            ofPoint center = toOf(contourFinder.getCenter(i));
            ofPushMatrix();
            ofTranslate(center.x, center.y);
            int label = contourFinder.getLabel(i);
            string msg = ofToString(label) + ":" + ofToString(tracker.getAge(label));
            ofDrawBitmapString(msg, 0-boundRect.width/2+10, 0+boundRect.height/2-10);
            // ofVec2f velocity = toOf(contourFinder.getVelocity(i));
            // ofScale(5, 5);
            // ofDrawLine(0, 0, velocity.x, velocity.y);
            ofPopMatrix();
        }
    }
    
    //
    if(thresholded.isAllocated()) {
        ofPushStyle();
        ofSetColor(255,130);
        thresholded.draw(movie.getWidth()-thresholded.getWidth()/5-10, 10, thresholded.getWidth()/5,thresholded.getHeight()/5);
        ofPopStyle();
    }
    
    // Draw the CCV things
    if (ccv.isLoaded() && showCcv) {
        
        // int l = ofMap(mouseX, 0, ofGetWidth(), 0, ccv.numLayers()-3);
        ofSetColor(255);
        ofDrawRectangle(0, 0, movie.getWidth(), ofGetHeight()/scale);
        maps = ccv.getFeatureMaps(layer);
        ofImage img;
        for (int m=0; m<maps.size(); m++) {
            ofPushMatrix();
            ofSetColor(255);
            if (maps.size() > 1) {
                maps[m].getImage(img);
                ofTranslate((m%12)*107, (floor(m/12))*107 - scroll);
                img.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
                img.draw(0, 0, 106, 106);
            }
            ofPopMatrix();
        }
        
        // Draw highlighted
        if (highlighted != -1 && highlighted < maps.size() && maps.size() > 1) {
            ofxCcv::FeatureMap map = maps[highlighted];
            map.getImage(img);
            ofPushMatrix();
            ofTranslate(ofClamp((highlighted%12)*107 - 53.5, 0, ofGetWidth()-220), ofClamp(floor(highlighted/12)*112 - scroll - 53.5, 20, ofGetHeight()-220));
            ofSetColor(255, 255, 255);
            ofFill();
            ofDrawRectangle(-10, -25, 234, 249);
            ofSetColor(0);
            ofDrawBitmapString(ccv.getLayerNames()[layer]+" - "+ofToString(highlighted), 0, -5);
            ofSetColor(255);
            img.draw(0, 0, 214, 214);
            ofPopMatrix();
        }    
        
        // Draw weights
        if (layer == ccv.getLayerNames().size()-1) {
            // results = ccv.classify(activeObject);
            results = ccv.getResults();
            
            ofPushStyle();
            ofPushMatrix();
            ofSetColor(16);
            ofDrawRectangle(0, 0, movie.getWidth(), ofGetHeight()/scale);
            ofTranslate(20, 20);
            for(int i = 0; i < results.size(); i++) {
                ofSetColor(40);
                ofFill();
                ofDrawRectangle(0, 0, 200, 20);
                ofSetColor(ofColor::darkorange);
                ofDrawRectangle(1, 2, (200-2) * results[i].confidence, 15);
                ofSetColor(ofColor::white);
                myfont.drawString(results[i].imageNetName, 210, 15);

                // ofDrawBitmapStringHighlight(results[i].imageNetName, 106, 15);
                ofTranslate(0, 25);
            }
            ofPopMatrix();
            ofPopStyle();
        }

        
    }
    ofPopMatrix();

}

//--------------------------------------------------------------

void ofApp::pauseMovie() {
    movie.setPaused(movieIsPlaying);
    movieIsPlaying = !movieIsPlaying;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if(key == ' ') {
        pauseMovie();
    }
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
    scroll = ofClamp(scroll - (ofGetMouseY() - ofGetPreviousMouseY()), 0, 32 * 110);
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
    for (int i=0; i<12; i++) {
        for (int j=0; j<22; j++) {
            if (ofRectangle(gui1->getWidth() + 112*i*1.6, (5 + 112*j - scroll)*1.6, 107*1.6, 107*1.6).inside(x, y)) {
                highlighted = i + 12 * j;
                return;
            }
        }
    }
    highlighted = -1;
}


void ofApp::onButtonEvent(ofxDatGuiButtonEvent e) {
    if (e.target->is("Reset Background")) {
        resetBackground = true;
    }
    if (e.target->is("Live feed")) {
        showCcv = false;
        pauseMovie();
    }
    
    else {
        vector<string> layerNames = ccv.getLayerNames();
        for (int i=0; i<layerNames.size(); i++) {
            if (e.target->is(layerNames[i])) {
                layer = i;
            }
        }
        if (layer == layerNames.size()-1) {
            ccv.setClassify(true, 20);
        } else {
            ccv.setClassify(false);
        }
        ccv.update(activeObject, ccv.numLayers());
        showCcv = true;
        scroll = 0;
    }
    cout << "onButtonEvent: " << e.target->getLabel() << endl;
}

void ofApp::onToggleEvent(ofxDatGuiToggleEvent e)
{
    if (e.target->is("Pause")) {
        pauseMovie();
    }
    cout << "onToggleEvent: " << e.target->getLabel() << " " << e.checked << endl;
}

void ofApp::onSliderEvent(ofxDatGuiSliderEvent e)
{
    cout << "onSliderEvent: " << e.target->getLabel() << " "; e.target->printValue();
}

void ofApp::onTextInputEvent(ofxDatGuiTextInputEvent e)
{
    cout << "onTextInputEvent: " << e.target->getLabel() << " " << e.target->getText() << endl;
}

void ofApp::on2dPadEvent(ofxDatGui2dPadEvent e)
{
    cout << "on2dPadEvent: " << e.target->getLabel() << " " << e.x << ":" << e.y << endl;
}

void ofApp::onDropdownEvent(ofxDatGuiDropdownEvent e)
{
    cout << "onDropdownEvent: " << e.target->getLabel() << " Selected" << endl;
}

void ofApp::onColorPickerEvent(ofxDatGuiColorPickerEvent e)
{
    cout << "onColorPickerEvent: " << e.target->getLabel() << " " << e.target->getColor() << endl;
    ofSetBackgroundColor(e.color);
}

void ofApp::onMatrixEvent(ofxDatGuiMatrixEvent e)
{
    cout << "onMatrixEvent " << e.child << " : " << e.enabled << endl;
    cout << "onMatrixEvent " << e.target->getLabel() << " : " << e.target->getSelected().size() << endl;
}
