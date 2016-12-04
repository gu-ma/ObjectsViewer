#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "ofxCcvThreaded.h"
#include "ofxDatGui.h"

//#define _USE_LIVE_VIDEO

class ofApp : public ofBaseApp {
public:

    void setup();
	void update();
	void draw();
    void mouseDragged(int x, int y, int button);
    void mouseMoved(int x, int y);
    void keyPressed(int key);
    //
    void pauseMovie();
    
    ofTrueTypeFont myfont;
    bool showLabels, movieIsPlaying, showGui, showCcv;
    ofImage thresholded,activeObject;
    ofPixels activeObjectPixels;
    
    // datGUI
    ofxDatGui* gui1,gui2;
    void onButtonEvent(ofxDatGuiButtonEvent e);
    void onToggleEvent(ofxDatGuiToggleEvent e);
    void onSliderEvent(ofxDatGuiSliderEvent e);
    void onTextInputEvent(ofxDatGuiTextInputEvent e);
    void on2dPadEvent(ofxDatGui2dPadEvent e);
    void onDropdownEvent(ofxDatGuiDropdownEvent e);
    void onColorPickerEvent(ofxDatGuiColorPickerEvent e);
    void onMatrixEvent(ofxDatGuiMatrixEvent e);

    // ofParam
    ofParameter<bool> resetBackground;
    ofParameter<float> learningTime, thresholdValue, blurValue;
    ofParameter<float> minArea, maxArea, threshold;
    ofParameter<bool> holes;
    
    // vid + mov
	ofVideoGrabber cam;
    ofVideoPlayer movie;
    
    // CCV
	ofxCv::RunningBackground background;
    ofxCv::ContourFinder contourFinder;
    ofxCcvThreaded ccv;
    vector<ofxCcv::FeatureMap> maps;
    vector<ofxCcv::Classification> results;
    int layer, inputMode, viewMode, highlighted, scroll;

};
