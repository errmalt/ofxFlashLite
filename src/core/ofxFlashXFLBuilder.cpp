/*
 *  ofxFlashXFLBuilder.cpp
 *  emptyExample
 *
 *  Created by lukasz karluk on 5/11/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofxFlashXFLBuilder.h"

ofxFlashXFLBuilder :: ofxFlashXFLBuilder()
{
    bVerbose    = false;

    xflRoot     = "";
	xflFile		= "";
	container	= NULL;
	domType		= DOM_DOCUMENT_TYPE;
	totalFrames	= 1;
    currentFrame = 1;
}

ofxFlashXFLBuilder :: ~ofxFlashXFLBuilder()
{
	//
}

///////////////////////////////////////////
//	BUILD.
///////////////////////////////////////////

void ofxFlashXFLBuilder :: build ( const string& root, const string& file, ofxFlashDisplayObjectContainer* container )
{
    xflRoot = root;
	xflFile	= file;
    string xflFilePath = xflRoot + xflFile;
    
	this->container = container;
	
    bool success = loadFile( xflFilePath );
    
    if( bVerbose )
    {
        if( success )
            cout << "[ ofxFlashXFLBuilder :: build ] - loading movieclip xml - SUCCESS :: " << xflFilePath << endl;
        else
            cout << "[ ofxFlashXFLBuilder :: build ] - loading movieclip xml - FAILED  :: " << xflFilePath << endl;
    }
    
	if( success )
	{
		TiXmlElement* child = ( storedHandle.FirstChild() ).ToElement();
		domType = child->Value();
		
		pushTag( domType, 0 );
		
		if( domType == DOM_DOCUMENT_TYPE )
		{
			pushTag( "timelines", 0 );
		}
		else if( domType == DOM_SYMBOL_ITEM_TYPE )
		{
			pushTag( "timeline", 0 );
		}
		
		countTotalFrames();
		
		ofxFlashMovieClip* mc;
		mc = (ofxFlashMovieClip*)container;
		mc->setTotalFrames( totalFrames );
		
		buildTimelines();
		
		popTag();
		popTag();
	}
}

void ofxFlashXFLBuilder :: countTotalFrames ()
{
	pushTag( "DOMTimeline", 0 );
	pushTag( "layers", 0 );
	
	int numOfLayers;
	numOfLayers = getNumTags( "DOMLayer" );
	
	for( int i=0; i<numOfLayers; i++ )
	{
		string layerType;
		layerType = getAttribute( "DOMLayer", "layerType", "", i );
		
		if( layerType == "guide" )		// skip guide layers.
			continue;
		
		pushTag( "DOMLayer", i );
		pushTag( "frames", 0 );
		
		int numOfFrames;
		numOfFrames = getNumTags( "DOMFrame" );
		
		for( int j=0; j<numOfFrames; j++ )
		{
			int frameIndex		= getAttribute( "DOMFrame", "index",	0, j );
			int frameDuration	= getAttribute( "DOMFrame", "duration",	1, j );
			int frameEnd		= frameIndex + frameDuration;
			
			if( frameEnd > totalFrames )
			{
				totalFrames = frameEnd;
			}
		}
		
		popTag();
		popTag();
	}
	
	popTag();
	popTag();
}

void ofxFlashXFLBuilder :: buildTimelines ()
{
	int numOfTimelines;
	numOfTimelines = getNumTags( "DOMTimeline" );
	
	for( int i=0; i<numOfTimelines; i++ )
	{
		DOMTimeline dom;
		dom.name			= getAttribute( "DOMTimeline", "name",			"", i );
		dom.currentFrame	= getAttribute( "DOMTimeline", "currentFrame",	1,  i );
		domTimeline			= dom;
		
		pushTag( "DOMTimeline", i );
		pushTag( "layers", 0 );
		
		buildLayers();
		
		popTag();
		popTag();
		
		return;		// SUPPORT ONLY ONE TIMELINE.
	}
	
	popTag();
}

void ofxFlashXFLBuilder :: buildLayers ()
{
	int numOfLayers;
	numOfLayers = getNumTags( "DOMLayer" );
	
	for( int i=numOfLayers-1; i>=0; i-- )	// work backwards through layers. so when adding to stage, objects sit in right order.
	{
		DOMLayer dom;
		dom.name		= getAttribute( "DOMLayer", "name",			"",		i );
		dom.color		= getAttribute( "DOMLayer", "color",		0,		i );
		dom.locked		= getAttribute( "DOMLayer", "locked",		false,  i );
		dom.current		= getAttribute( "DOMLayer", "current",		false,  i );
		dom.isSelected	= getAttribute( "DOMLayer", "isSelected",	false,  i );
		dom.autoNamed	= getAttribute( "DOMLayer", "autoNamed",	false,  i );
		dom.layerType	= getAttribute( "DOMLayer", "layerType",	"",		i );
		domLayer		= dom;
		
		if( domLayer.layerType == "guide" )		// skip guide layers.
			continue;
		
		pushTag( "DOMLayer", i );
		pushTag( "frames", 0 );
		
		buildFrames();
		
		popTag();
		popTag();
	}
}

void ofxFlashXFLBuilder::buildFrames() {
	int numOfFrames;
	numOfFrames = getNumTags("DOMFrame");
	
    vector<DOMFrame> domFrames;
    
	for(int i=0; i<numOfFrames; i++) {
		DOMFrame dom;
		dom.index = getAttribute("DOMFrame", "index", 0, i);
		dom.duration = getAttribute("DOMFrame", "duration", 1, i);
		dom.tweenType = getAttribute("DOMFrame", "tweenType", "", i);
        dom.motionTweenRotate = getAttribute("DOMFrame", "motionTweenRotate", "", i);
        dom.motionTweenRotateTimes = getAttribute("DOMFrame", "motionTweenRotateTimes", 0, i);
        dom.motionTweenSync = getAttribute("DOMFrame", "motionTweenSync", false, i);
        dom.motionTweenScale = getAttribute("DOMFrame", "motionTweenScale", false, i);
        dom.motionTweenSnap	= getAttribute("DOMFrame", "motionTweenSnap", false, i);
		dom.keyMode = getAttribute("DOMFrame", "keyMode", 0, i);
        dom.acceleration = getAttribute("DOMFrame", "acceleration", 0, i);
		domFrame = dom;
        domFrames.push_back(dom);
        
        pushTag("DOMFrame", i);
		pushTag("elements", 0);
        
        int j = domFrame.index;
        int t = domFrame.index + domFrame.duration;
        for(j; j<t; j++) {
            currentFrame = j + 1;
            buildElements();
        }
        
        if(tweenShape.size() > 0) { // tween was recorded on previous loop.

            DOMFrame domPrev = domFrames[domFrames.size() - 2];
            DOMFrame domCurr = domFrames[domFrames.size() - 1];

            ofxFlashMovieClip * containerMc;
            containerMc = (ofxFlashMovieClip *)container;

            containerMc->gotoAndStop(domPrev.index + 1);
            int lastChildIndex = containerMc->numChildren() - 1;
            /*
             *  TODO :: not 100% happy with lastChildIndex.
             *  there should be a better way of working out which
             *  object to tween rather then assuming the top one.
             */
            
            ofxFlashDisplayObject * obj1;
            ofxFlashDisplayObject * obj2;
            
            containerMc->gotoAndStop(domPrev.index + 1);
            obj1 = containerMc->getChildAt(lastChildIndex);
            
            containerMc->gotoAndStop(domCurr.index + 1);
            obj2 = containerMc->getChildAt(lastChildIndex);
            
            if(obj1 == NULL || obj2 == NULL) {
                continue;
            }
            
            ofxFlashMatrix mat1;
            ofxFlashMatrix mat2;
            mat1 = obj1->matrix();
            mat2 = obj2->matrix();
            
            float alpha1;
            float alpha2;
            alpha1 = obj1->alpha();
            alpha2 = obj2->alpha();
            
            for(int k=domPrev.index; k<=domCurr.index; k++) {
                float p = (k - domPrev.index) / (float)(domCurr.index - domPrev.index);
                p = tweenAt(tweenShape, p);
                
                int frameNum = k + 1;
                containerMc->gotoAndStop(frameNum);
                
                ofxFlashDisplayObject * obj;
                obj = containerMc->getChildAt(lastChildIndex);
                
                if(true) { // TODO - check if matrixes are different.
                    ofxFlashMatrix mat;
                    mat = ofxFlashMatrix::interpolate(mat1, mat2, p);
                    
                    float rotation = 0;
                    if(domPrev.motionTweenRotate == "clockwise") {
                        rotation = domPrev.motionTweenRotateTimes;
                    } else if(domPrev.motionTweenRotate == "counter-clockwise") {
                        rotation = domPrev.motionTweenRotateTimes * -1;
                    }
                    if(rotation != 0) {
                        ofxFlashMatrix rotMat;
                        rotMat.set_rotation(rotation * p * TWO_PI);
                        mat.concatenate(rotMat);
                    }
                    
                    obj->matrix(mat);
                }
                
                if(alpha1 != alpha2) {
                    float alpha;
                    alpha = (alpha2 - alpha1) * p + alpha1;
                    obj->alpha(alpha);
                }
            }
            
            containerMc->gotoAndPlay(1);
        }
        
        popTag();
        
        buildTween();
        
		popTag();
	}
}

void ofxFlashXFLBuilder::buildTween() {
    
    tweenShape.clear();
    
    if(domFrame.tweenType != "motion") {
        return;
    }
    
/* CustomEase example.
<tweens>
    <CustomEase target="all">
        <Point/>
        <Point x="0.3333"/>
        <Point x="0.7034" y="0.3356"/>
        <Point x="1" y="1"/>
    </CustomEase>
</tweens>
*/
    if(tagExists("tweens")) {
        pushTag("tweens");
        pushTag("CustomEase");

        int numOfPoints;
        numOfPoints = getNumTags("Point");

        float px = getAttribute("Point", "x", 0.0f, 0);
        float py = getAttribute("Point", "y", 0.0f, 0);
        tweenShape.addVertex(px, py);
        
        for(int i=1; i<numOfPoints; i+=3) {
            float cx1 = getAttribute("Point", "x", 0.0f, i);
            float cy1 = getAttribute("Point", "y", 0.0f, i);
            float cx2 = getAttribute("Point", "x", 0.0f, i+1);
            float cy2 = getAttribute("Point", "y", 0.0f, i+1);
            float px2 = getAttribute("Point", "x", 0.0f, i+2);
            float py2 = getAttribute("Point", "y", 0.0f, i+2);
            
            tweenShape.bezierTo(cx1, cy1, cx2, cy2, px2, py2, 60);
        }
        
        popTag();
        popTag();
    } else {
        float acceleration = domFrame.acceleration;
        acceleration /= 100;
        acceleration *= -1;
        
        tweenShape.addVertex(0, 0);

        float cx1 = 0.3333;
        float cy1 = 0.3333 + acceleration * 0.3333;
        float cx2 = 0.6667;
        float cy2 = 0.6667 + acceleration * 0.3333;
        float px2 = 1.0000;
        float py2 = 1.0000;
        
        tweenShape.bezierTo(cx1, cy1, cx2, cy2, px2, py2);
    }

    /*
     *  HACK!
     *  erase first point as its a duplicate.
     *  duplicate points cause issues with resampling.
     *  https://github.com/openframeworks/openFrameworks/issues/1664
     */
    tweenShape.getVertices().erase(tweenShape.getVertices().begin(), tweenShape.getVertices().begin()+1);
}

void ofxFlashXFLBuilder :: buildElements ()
{
	int numOfElements = 0;
	TiXmlElement* child = ( storedHandle.FirstChildElement() ).ToElement();
	for( numOfElements = 0; child; child = child->NextSiblingElement(), ++numOfElements ) {}
	
	for( int i=0; i<numOfElements; i++ )
	{
		TiXmlElement* child = ( storedHandle.ChildElement( i ) ).ToElement();
		string elementTag = child->Value();
		
		if( elementTag == "DOMGroup" )
		{
			pushTagAt( i );
			pushTag( "members", 0 );
			
			buildElements();
			
			popTag();
			popTag();
			
		}
		else if( elementTag == "DOMBitmapInstance" )
		{
			DOMBitmapInstance dom;
			dom.libraryItemName = child->Attribute( "libraryItemName" );
			if(child->Attribute( "name" )) dom.name = child->Attribute( "name" ); // Support for CS5.5, which doesn't export name tags if empty
			dom.referenceID		= "";
			domBitmapInstance	= dom;
			
			pushTagAt( i );
			
			buildBitmap();
			
			popTag();
		}
		else if( elementTag == "DOMSymbolInstance" )
		{
			DOMSymbolInstance dom;
			dom.libraryItemName	= child->Attribute( "libraryItemName" );
			if(child->Attribute( "name" )) dom.name = child->Attribute( "name" ); // Support for CS5.5, which doesn't export name tags if empty
			dom.centerPoint3DX	= 0.0;
			dom.centerPoint3DY	= 0.0; 
			domSymbolInstance	= dom;
			
			pushTagAt( i );
			
			buildMovieClip();
			
			popTag();
		}
		else if( elementTag == "DOMRectangleObject" )
		{
			DOMRectangleObject dom;
			child->QueryFloatAttribute( "x",			&dom.x );
			child->QueryFloatAttribute( "y",			&dom.y );
			child->QueryFloatAttribute( "objectWidth",	&dom.objectWidth );
			child->QueryFloatAttribute( "objectHeight",	&dom.objectHeight );
			domRectangleObject = dom;
			
			pushTagAt( i );
			
			buildRectangleShape();
			
			popTag();
		}
		else if( elementTag == "DOMOvalObject" )
		{
			DOMOvalObject dom;
			child->QueryFloatAttribute( "x",			&dom.x );
			child->QueryFloatAttribute( "y",			&dom.y );
			child->QueryFloatAttribute( "objectWidth",	&dom.objectWidth );
			child->QueryFloatAttribute( "objectHeight",	&dom.objectHeight );
			child->QueryFloatAttribute( "endAngle",		&dom.endAngle );
			domOvalObject = dom;
			
			pushTagAt( i );
			
			buildOvalShape();
			
			popTag();
		}
		else if( elementTag == "DOMShape" )
		{
			continue;	// NOT SUPPORTED AT THE MOMENT.
		}
		else if( elementTag == "DOMStaticText" )
		{
			continue;	// NOT SUPPORTED AT THE MOMENT.
		}
		else if( elementTag == "DOMDynamicText" )
		{
			continue;	// NOT SUPPORTED AT THE MOMENT.
		}
		else if( elementTag == "DOMInputText" )
		{
			continue;	// NOT SUPPORTED AT THE MOMENT.
		}
	}
}

void ofxFlashXFLBuilder :: buildBitmap ()
{
	ofBaseDraws* bitmapImage;
	bitmapImage = ofxFlashLibrary :: getInstance()->getAsset( domBitmapInstance.libraryItemName );
	
    //
    // if no name is given for bitmap, use the file name.
    // file name is taken from libraryItemName.
    //
    string name;
    name = domBitmapInstance.name;
    if( name.size() == 0 )
        name = ofSplitString( domBitmapInstance.libraryItemName, "/" ).back();
    
	ofxFlashBitmap* bm;
	bm = new ofxFlashBitmap( bitmapImage );
	bm->name( name );
	bm->libraryItemName( domBitmapInstance.libraryItemName );

	setupMatrixForDisplayObject( bm );
	
	addDisplayObjectToFrames( bm );
}

void ofxFlashXFLBuilder :: buildMovieClip ()
{
    bool bAddLibraryToPath = false;
    bAddLibraryToPath = bAddLibraryToPath || domType == DOM_DOCUMENT_TYPE;
    bAddLibraryToPath = bAddLibraryToPath || domType == DOM_SYMBOL_ITEM_TYPE;
    
	string libraryItemPath = "";
	libraryItemPath += bAddLibraryToPath ? "LIBRARY/" : "";
	libraryItemPath += domSymbolInstance.libraryItemName;
	libraryItemPath += ".xml";

	ofxFlashMovieClip* mc;
	mc = new ofxFlashMovieClip();
	mc->name( domSymbolInstance.name );
	mc->libraryItemName( domSymbolInstance.libraryItemName );
	
	setupMatrixForDisplayObject( mc );
	setupColorForDisplayObject( mc );
    
    addDisplayObjectToFrames( mc );
	
	ofxFlashXFLBuilder* builder;
	builder = new ofxFlashXFLBuilder();
    builder->setVerbose( bVerbose );
	builder->build( xflRoot, libraryItemPath, mc );
    
	delete builder;
	builder = NULL;
}

void ofxFlashXFLBuilder :: buildRectangleShape ()
{
	ofxFlashShape* shape;
	shape = new ofxFlashShape();
	
	//-- position & transform.
	
	float cx = domRectangleObject.x + domRectangleObject.objectWidth  * 0.5;		// center point.
	float cy = domRectangleObject.y + domRectangleObject.objectHeight * 0.5;		// center point.
	
	float transformationPointX = cx;												// default transformation point is center.
	float transformationPointY = cy;												// default transformation point is center.
	
	if( tagExists( "transformationPoint", 0 ) )
	{
		pushTag( "transformationPoint", 0 );
		
		transformationPointX = getAttribute( "Point", "x", cx, 0 );
		transformationPointY = getAttribute( "Point", "y", cy, 0 );
		
		popTag();
	}
	
	setupMatrixForDisplayObject( shape );
	
	float shiftX = transformationPointX - cx;
	float shiftY = transformationPointY - cy;
	
	ofxFlashMatrix matrix;
	matrix = shape->matrix();				// get matrix.
	
	float tx = matrix.getTx() + shiftX;
	float ty = matrix.getTy() + shiftY;
	
	matrix.setTx( tx );						// adjust matrix.
	matrix.setTy( ty );
	
	shape->matrix( matrix );				// set matrix.
	
	ofRectangle shapeRect;
	shapeRect.x			= domRectangleObject.x + shiftX;
	shapeRect.y			= domRectangleObject.y + shiftY;
	shapeRect.width		= domRectangleObject.objectWidth;
	shapeRect.height	= domRectangleObject.objectHeight;
	
	shape->setRectangle( shapeRect.x, shapeRect.y, shapeRect.width, shapeRect.height );
	
	setupFillForShape( shape );
	setupStrokeForShape( shape );
	
	addDisplayObjectToFrames( shape );
}

void ofxFlashXFLBuilder :: buildOvalShape ()
{
	ofxFlashShape* shape;
	shape = new ofxFlashShape();
	
	float cx = domOvalObject.x + domOvalObject.objectWidth  * 0.5;		// center point.
	float cy = domOvalObject.y + domOvalObject.objectHeight * 0.5;		// center point.
	
	float transformationPointX = cx;									// default transformation point is center.
	float transformationPointY = cy;									// default transformation point is center.
	
	if( tagExists( "transformationPoint", 0 ) )
	{
		pushTag( "transformationPoint", 0 );
		
		transformationPointX = getAttribute( "Point", "x", cx, 0 );
		transformationPointY = getAttribute( "Point", "y", cy, 0 );
		
		popTag();
	}
	
	setupMatrixForDisplayObject( shape );
	
	float shiftX = transformationPointX - cx;
	float shiftY = transformationPointY - cy;
	
	ofxFlashMatrix matrix;
	matrix = shape->matrix();				// get matrix.
	
	float tx = matrix.getTx() + shiftX;
	float ty = matrix.getTy() + shiftY;
	
	matrix.setTx( tx );						// adjust matrix.
	matrix.setTy( ty );
	
	shape->matrix( matrix );				// set matrix.
	
	ofRectangle shapeRect;
	shapeRect.x			= domOvalObject.x + shiftX;
	shapeRect.y			= domOvalObject.y + shiftY;
	shapeRect.width		= domOvalObject.objectWidth;
	shapeRect.height	= domOvalObject.objectHeight;
	
	shape->setOval( shapeRect.x, shapeRect.y, shapeRect.width, shapeRect.height );
	
	setupFillForShape( shape );
	setupStrokeForShape( shape );
	
	addDisplayObjectToFrames( shape );
}

///////////////////////////////////////////
//	COMMON BUILDER FUNCTIONS.
///////////////////////////////////////////

void ofxFlashXFLBuilder::addDisplayObjectToFrames(ofxFlashDisplayObject * displayObject) {
    ofxFlashMovieClip * containerMc;
    containerMc = (ofxFlashMovieClip *)container;
    containerMc->gotoAndStop(currentFrame);
    containerMc->addChild(displayObject);
    containerMc->gotoAndPlay(1);
}

void ofxFlashXFLBuilder :: setupMatrixForDisplayObject ( ofxFlashDisplayObject* displayObject )
{
	if( tagExists( "matrix", 0 ) )
	{
		pushTag( "matrix", 0 );
		
		float a		= getAttribute( "Matrix", "a",  1.0, 0 );
		float b		= getAttribute( "Matrix", "b",  0.0, 0 );
		float c		= getAttribute( "Matrix", "c",  0.0, 0 );
		float d		= getAttribute( "Matrix", "d",  1.0, 0 );
		float tx	= getAttribute( "Matrix", "tx", 0.0, 0 );
		float ty	= getAttribute( "Matrix", "ty", 0.0, 0 );
		
		ofxFlashMatrix matrix;
		matrix.set( a, b, c, d, tx, ty );
		
		displayObject->matrix( matrix );
		
		popTag();
	}
}

void ofxFlashXFLBuilder :: setupColorForDisplayObject ( ofxFlashDisplayObject* displayObject )
{
	if( tagExists( "color", 0 ) )
	{
		pushTag( "color", 0 );
		
		float alphaMultiplier = getAttribute( "Color", "alphaMultiplier",  1.0, 0 );
		
		displayObject->alpha( alphaMultiplier );
		
		popTag();
	}
}

void ofxFlashXFLBuilder :: setupFillForShape ( ofxFlashShape* shape )
{
	if( tagExists( "fill", 0 ) )
	{
		pushTag( "fill", 0 );
		
		string fillSolidColor;
		fillSolidColor = getAttribute( "SolidColor", "color", "#000000", 0 );
		fillSolidColor = cleanHexString( fillSolidColor );
		
		float fillAlpha;
		fillAlpha = getAttribute( "SolidColor", "alpha",  1.0, 0 );
		
		shape->setFill( true );
		shape->setFillColor( stringToHex( fillSolidColor ) );
		shape->setFillAlpha( fillAlpha );
		
		popTag();
	}
}

void ofxFlashXFLBuilder :: setupStrokeForShape ( ofxFlashShape* shape )
{
	if( tagExists( "stroke", 0 ) )
	{
		pushTag( "stroke", 0 );
		
		int solidStrokeWeight;
		solidStrokeWeight = getAttribute( "SolidStroke", "weight",  0, 0 );
		
		pushTag( "SolidStroke", 0 );
		pushTag( "fill", 0 );
		
		string fillSolidColor;
		fillSolidColor = getAttribute( "SolidColor", "color", "#000000", 0 );
		fillSolidColor = cleanHexString( fillSolidColor );
		
		float fillAlpha;
		fillAlpha = getAttribute( "SolidColor", "alpha",  1.0, 0 );
		
		shape->setStroke( true );
		shape->setStrokeWeight( solidStrokeWeight );
		shape->setStrokeColor( stringToHex( fillSolidColor ) );
		shape->setStrokeAlpha( fillAlpha );
		
		ofColor c;
		
		popTag();
		popTag();
		popTag();
	}
}

///////////////////////////////////////////
//	CUSTOM XML FUNCTIONS.
///////////////////////////////////////////

void ofxFlashXFLBuilder :: pushTagAt( int i )
{
	TiXmlHandle isRealHandle = storedHandle.ChildElement( i );
	if( isRealHandle.ToNode() )
	{
		storedHandle = isRealHandle;
		level++;
	}
}

///////////////////////////////////////////
//	STRING HEX TO INT CONVERSIONS.
///////////////////////////////////////////

float ofxFlashXFLBuilder::tweenAt(ofPolyline & polyline, float progress) {
    if(polyline.size() < 2) {
        return 0;
    }
    
    if(progress == 0 && polyline[0].x == 0) {
        return 0;
    }
    
    if(progress == 1 && polyline[polyline.size()-1].x == 1) {
        return 1;
    }
    
    ofPoint p1(progress, 0);
    ofPoint p2(progress, 1);
    ofPoint p5;
    
    for(int i=0; i<polyline.size()-1; i++) {
        ofPoint & p3 = polyline[i];
        ofPoint & p4 = polyline[i+1];
        
        bool bIntersect = ofLineSegmentIntersection(p1, p2, p3, p4, p5);
        if(bIntersect) {
            return p5.y;
        }
    }
}

///////////////////////////////////////////
//	STRING HEX TO INT CONVERSIONS.
///////////////////////////////////////////

string ofxFlashXFLBuilder :: cleanHexString ( string value )
{
    string clean = "0x";
    clean += value.substr( 1, value.length() - 1 );
    
	return clean;
}

int ofxFlashXFLBuilder :: stringToHex ( string value )
{
	unsigned int x;
	stringstream ss;
	ss << hex << value;
	ss >> x;
	
	return x;
}