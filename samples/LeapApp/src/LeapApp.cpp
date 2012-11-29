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
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "Cinder-LeapSdk.h"

class LeapApp : public ci::app::AppBasic
{
public:
	void					draw();
	void					prepareSettings( ci::app::AppBasic::Settings *settings );
	void					setup();
	void					shutdown();
	void					update();
private:
	// Leap
	uint32_t				mCallbackId;
	LeapSdk::HandMap		mHands;
	LeapSdk::DeviceRef		mLeap;
	void 					onFrame( LeapSdk::Frame frame );

	// Camera
	ci::CameraPersp			mCamera;

	// Params
	float					mFrameRate;
	bool					mFullScreen;
	ci::params::InterfaceGl	mParams;

	// Save screen shot
	void					screenShot();
};

#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"

// Imports
using namespace ci;
using namespace ci::app;
using namespace LeapSdk;
using namespace std;

// Render
void LeapApp::draw()
{
	// Clear window
	gl::setViewport( getWindowBounds() );
	gl::clear( Colorf::black() );
	gl::setMatrices( mCamera );
	
	// Iterate through hands
	for ( HandMap::const_iterator handIter = mHands.begin(); handIter != mHands.end(); ++handIter ) {
		const Hand& hand = handIter->second;

		// Hand ball
		if ( hand.getBallRadius() > 0.0f ) {
			gl::color( ColorAf( 1.0f, 1.0f, 0.0f, 0.25f ) );
			gl::enableWireframe();
			gl::drawSphere( hand.getBallPosition(), hand.getBallRadius(), 16 );
			gl::disableWireframe();
		}

		// Hand direction
		gl::color( 1.0f, 0.0f, 1.0f, 1.0f );
		gl::drawVector( hand.getPosition(), hand.getPosition() + hand.getDirection() * 10.0f );

		// Hand normal
		gl::color( 0.0f, 0.0f, 1.0f, 1.0f );
		gl::drawVector( hand.getPosition(), hand.getPosition() + hand.getNormal() * 5.0f );

		// Hand velocity
		gl::color( 1.0f, 0.0f, 0.0f, 1.0f );
		gl::drawVector( hand.getPosition(), hand.getPosition() + hand.getVelocity() );

		// Fingers
		const FingerMap& fingers = hand.getFingers();
		for ( FingerMap::const_iterator fingerIter = fingers.begin(); fingerIter != fingers.end(); ++fingerIter ) {
			const Finger& finger = fingerIter->second;

			// Finger
			gl::color( ColorAf::white() );
			gl::drawLine( finger.getPosition(), finger.getPosition() + finger.getDirection() * -finger.getLength() );

			// Finger tip
			gl::pushMatrices();
			gl::translate( finger.getPosition() );
			gl::drawStrokedCircle( Vec2f::zero(), finger.getWidth(), 16 );
			gl::popMatrices();

			// Finger velocity
			gl::color( 0.0f, 1.0f, 0.0f, 1.0f );
			gl::drawVector( finger.getPosition(), finger.getPosition() + finger.getVelocity() );
		}
	}
	
	// Draw the interface
	mParams.draw();

}

// Called when Leap frame data is ready
void LeapApp::onFrame( Frame frame )
{
	mHands = frame.getHands();
}

// Prepare window
void LeapApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1024, 768 );
	settings->setFrameRate( 60.0f );
}

// Take screen shot
void LeapApp::screenShot()
{
	writeImage( getAppPath() / fs::path( "frame" + toString( getElapsedFrames() ) + ".png" ), copyWindowSurface() );
}

// Set up
void LeapApp::setup()
{
	// Set up OpenGL
	gl::enable( GL_LINE_SMOOTH );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST ); 
	gl::enable( GL_POLYGON_SMOOTH );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	gl::enableAlphaBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();

	// Set up camera
	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f, 0.01f, 1000.0f );
	mCamera.lookAt( Vec3f( 0.0f, 100.0f, 500.0f ), Vec3f( 0.0f, 50.0f, 0.0f ) );
	
	// Start device
	mLeap = Device::create();
	mLeap->addCallback( &LeapApp::onFrame, this );
	mLeap->start();

	// Params
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	mParams = params::InterfaceGl( "Params", Vec2i( 200, 120 ) );
	mParams.addParam( "Frame rate",		&mFrameRate, "", true );
	mParams.addParam( "Full screen",	&mFullScreen, "key=f" );
	mParams.addButton( "Screen shot",	bind( &LeapApp::screenShot, this ), "key=space" );
	mParams.addButton( "Quit",			bind( &LeapApp::quit, this ), "key=q" );
}

// Quit
void LeapApp::shutdown()
{
	mLeap->removeCallback( mCallbackId );
	mHands.clear();
}

// Runs update logic
void LeapApp::update()
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
}

// Run application
CINDER_APP_BASIC( LeapApp, RendererGl )
