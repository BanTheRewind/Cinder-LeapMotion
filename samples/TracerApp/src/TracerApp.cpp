/*
* 
* Copyright (c) 2012, Ban the Rewind
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or 
* without modification, are permitted provided that the following 
* conditions are met:
* 
* Redistributions of source code must retain the above copyright 
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright 
* notice, this list of conditions and the following disclaimer in 
* the documentation and/or other materials provided with the 
* distribution.
* 
* Neither the name of the Ban the Rewind nor the names of its 
* contributors may be used to endorse or promote products 
* derived from this software without specific prior written 
* permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

#include "cinder/app/AppBasic.h"
#include "cinder/Camera.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/params/Params.h"
#include "Cinder-LeapSdk.h"
#include "Ribbon.h"

class TracerApp : public ci::app::AppBasic
{
public:
	void					draw();
	void					onResize( ci::app::ResizeEvent event );
	void					prepareSettings( ci::app::AppBasic::Settings *settings );
	void					setup();
	void					shutdown();
	void					update();
private:
	RibbonMap				mRibbons;

	// Leap
	uint32_t				mCallbackId;
	LeapSdk::HandMap		mHands;
	LeapSdk::DeviceRef		mLeap;
	void 					onFrame( LeapSdk::Frame frame );

	// Trails
	ci::gl::Fbo				mFbo[ 3 ];
	ci::gl::GlslProg		mShader[ 2 ];

	// Camera
	ci::CameraPersp			mCamera;
	ci::Vec3f				mLookAt;
	ci::Vec3f				mLookAtDest;

	// Params
	float					mFrameRate;
	bool					mFullScreen;
	ci::params::InterfaceGl	mParams;

	// Save screen shot
	void					screenShot();
};

#include "cinder/ImageIo.h"
#include "cinder/Rand.h"
#include "cinder/Utilities.h"
#include "Resources.h"

// Imports
using namespace ci;
using namespace ci::app;
using namespace LeapSdk;
using namespace std;

// Render
void TracerApp::draw()
{
	// Add to accumulation buffer
	mFbo[ 0 ].bindFramebuffer();
	gl::setViewport( mFbo[ 0 ].getBounds() );
	gl::setMatricesWindow( mFbo[ 0 ].getSize() );
	gl::enableAlphaBlending();

	// Dim last frame
	gl::color( ColorAf( Colorf::black(), 0.03f ) );
	gl::drawSolidRect( Rectf( mFbo[ 0 ].getBounds() ) );

	// Draw finger tips into the accumulation buffer
	float headLength = 6.0f;
	float headRadius = 3.0f;
	gl::setMatrices( mCamera );
	gl::enableAdditiveBlending();
	for ( RibbonMap::const_iterator iter = mRibbons.begin(); iter != mRibbons.end(); ++iter ) {
		iter->second.draw();
	}
	mFbo[ 0 ].unbindFramebuffer();

	// Blur the accumulation buffer
	gl::enable( GL_TEXTURE_2D );
	gl::enableAlphaBlending();
	gl::color( ColorAf::white() );
	Vec2f pixel	= Vec2f::one() / Vec2f( mFbo[ 0 ].getSize() );
	pixel		*= 3.0f;
	for ( size_t i = 0; i < 2; ++i ) {
		mFbo[ i + 1 ].bindFramebuffer();
		gl::clear();
		
		mShader[ i ].bind();
		mShader[ i ].uniform( "size",		pixel );
		mShader[ i ].uniform( "tex",		0 );
				
		gl::Texture& texture = mFbo[ i ].getTexture();
		texture.bind();
		gl::drawSolidRect( Rectf( mFbo[ i ].getBounds() ) );
		texture.unbind();
		
		mShader[ i ].unbind();
		mFbo[ i + 1 ].unbindFramebuffer();
	}

	// Draw blurred image
	gl::setMatricesWindow( getWindowSize(), false );
	gl::color( ColorAf::white() );
	mFbo[ 0 ].bindTexture();
	gl::drawSolidRect( Rectf( getWindowBounds() ) );
	mFbo[ 0 ].unbindTexture();
	
	gl::color( ColorAf( Colorf::white(), 0.8f ) );
	mFbo[ 2 ].bindTexture();
	gl::drawSolidRect( Rectf( getWindowBounds() ) );
	mFbo[ 2 ].unbindTexture();
	gl::disableAlphaBlending();
	gl::disable( GL_TEXTURE_2D );

	// Draw the interface
	mParams.draw();
}

// Called when Leap frame data is ready
void TracerApp::onFrame( Frame frame )
{
	mHands = frame.getHands();
}

void TracerApp::onResize( ResizeEvent event )
{
	// Enable polygon smoothing
	gl::enable( GL_POLYGON_SMOOTH );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );

	// Set up FBOs
	gl::Fbo::Format format;
	format.setColorInternalFormat( GL_RGBA32F );
	format.setMinFilter( GL_LINEAR );
	format.setMagFilter( GL_LINEAR );
	format.setWrap( GL_CLAMP, GL_CLAMP );
	for ( size_t i = 0; i < 3; ++i ) {
		mFbo[ i ]	= gl::Fbo( event.getWidth(), event.getHeight(), format );
		mFbo[ i ].bindFramebuffer();
		gl::setViewport( mFbo[ i ].getBounds() );
		gl::clear();
		mFbo[ i ].unbindFramebuffer();
	}
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
}

// Prepare window
void TracerApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1024, 768 );
	settings->setFrameRate( 60.0f );
}

// Take screen shot
void TracerApp::screenShot()
{
	writeImage( getAppPath() / fs::path( "frame" + toString( getElapsedFrames() ) + ".png" ), copyWindowSurface() );
}

// Set up
void TracerApp::setup()
{
	// Set up camera
	mLookAt		= Vec3f( 0.0f, 250.0f, 0.0f );
	mLookAtDest = mLookAt;
	mCamera		= CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f, 0.01f, 1000.0f );
	
	// Start device
	mLeap = Device::create();
	mLeap->addCallback( &TracerApp::onFrame, this );

	// Load shaders
	try {
		mShader[ 0 ]	= gl::GlslProg( loadResource( RES_GLSL_PASS_THROUGH_VERT ), loadResource( RES_GLSL_BLUR_X_FRAG ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << "Unable to compile blur X shader: \n" << string( ex.what() ) << "\n";
		quit();
	}
	try {
		mShader[ 1 ]	= gl::GlslProg( loadResource( RES_GLSL_PASS_THROUGH_VERT ), loadResource( RES_GLSL_BLUR_Y_FRAG ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << "Unable to compile blur Y shader: \n" << string( ex.what() ) << "\n";
		quit();
	}

	// Params
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	mParams = params::InterfaceGl( "Params", Vec2i( 200, 105 ) );
	mParams.addParam( "Frame rate",		&mFrameRate,							"", true );
	mParams.addParam( "Full screen",	&mFullScreen,							"key=f"		);
	mParams.addButton( "Screen shot",	bind( &TracerApp::screenShot, this ),	"key=space" );
	mParams.addButton( "Quit",			bind( &TracerApp::quit, this ),			"key=q" );

	// Run resize event to set up OpenGL
	onResize( ResizeEvent( getWindowSize() ) );
}

// Quit
void TracerApp::shutdown()
{
	mLeap->removeCallback( mCallbackId );
	mHands.clear();
}

// Runs update logic
void TracerApp::update()
{
	// Update frame rate
	mFrameRate = getAverageFps();

	// Toggle fullscreen
	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
	}

	// Update device
	if ( mLeap && mLeap->isConnected() ) {		
		mLeap->update();
	}
	
	// Process hand data
	float count		= 0.0f;
	Vec3f centroid	= Vec3f::zero();
	for ( HandMap::const_iterator handIter = mHands.begin(); handIter != mHands.end(); ++handIter ) {
		const Hand& hand = handIter->second;
		
		const FingerMap& fingers = hand.getFingers();
		for ( FingerMap::const_iterator fingerIter = fingers.begin(); fingerIter != fingers.end(); ++fingerIter, count += 1.0f ) {
			const Finger& finger = fingerIter->second;

			int32_t id = fingerIter->first;
			if ( mRibbons.find( id ) == mRibbons.end() ) {
				Vec3f v = randVec3f() * 0.01f;
				v.x = math<float>::abs( v.x );
				v.y = math<float>::abs( v.y );
				v.z = math<float>::abs( v.z );
				Colorf color( ColorModel::CM_RGB, v );
				Ribbon ribbon( id, color );
				mRibbons.insert( make_pair<int32_t, Ribbon>( id, ribbon ) );
			}
			float width = ( finger.getWidth() + finger.getVelocity().length() ) * 0.01f;
			mRibbons[ id ].addPoint( finger.getPosition(), width );

			centroid += finger.getPosition();
		}
	}
	if ( count > 0.0f ) {
		mLookAtDest = centroid / count;
	}
	mLookAt = mLookAt.lerp( 0.021f, mLookAtDest );
	mCamera.lookAt( Vec3f( -mLookAt.x * 0.5f, 125.0f + -mLookAt.y * 0.125f, 250.0f ), mLookAt );

	// Update ribbons
	for ( RibbonMap::iterator iter = mRibbons.begin(); iter != mRibbons.end(); ++iter ) {
		iter->second.update();
	}
}

// Run application
CINDER_APP_BASIC( TracerApp, RendererGl )
