/*
* 
* Copyright (c) 2013, Ban the Rewind
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
#include "cinder/gl/Light.h"
#include "cinder/params/Params.h"
#include "Cinder-LeapSdk.h"

class MotionApp : public ci::app::AppBasic
{
public:
	void					draw();
	void					prepareSettings( ci::app::AppBasic::Settings* settings );
	void					setup();
	void					shutdown();
	void					update();
private:
	// Leap
	uint32_t				mCallbackId;
	LeapSdk::Frame			mFrame;
	LeapSdk::DeviceRef		mLeap;
	void 					onFrame( LeapSdk::Frame frame );

	// Lighting
	ci::gl::Light			*mLight;
	
	// Motion matrices
	float					mRotAngle;
	ci::Vec3f				mRotAxis;
	float					mScale;
	ci::Vec3f				mTranslate;
	ci::Matrix44f			mTransform;
	
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

static const float	kRestitution	= 0.021f;
static const float	kRotSpeed		= 0.033f;
static const float	kTranslateSpeed	= 0.0033f;

// Render
void MotionApp::draw()
{
	// Clear window
	gl::setViewport( getWindowBounds() );
	gl::clear( Colorf::white() );
	gl::setMatrices( mCamera );

	// Enable depth
	gl::enableAlphaBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();
	
	// Draw cube representing transform
	gl::enable( GL_LIGHTING );
	mLight->enable();
	
	gl::color( ColorAf::gray( 0.5f ) );
	gl::pushMatrices();
	gl::multModelView( mTransform );
	gl::drawCube( Vec3f::zero(), Vec3f::one() );
	gl::popMatrices();
	
	mLight->disable();
	gl::disable( GL_LIGHTING );
	
	// Draw the interface
	mParams.draw();
}

// Called when Leap frame data is ready
void MotionApp::onFrame( Frame frame )
{
	HandMap hands = frame.getHands();
	if ( !hands.empty() ) {
		const Hand& hand	= hands.begin()->second;
		
		mRotAngle	+= hand.getRotationAngle( mFrame ) * kRotSpeed;
		mRotAxis	+= hand.getRotationAxis( mFrame ) * -1.0f; // Mirror
		mScale		*= hand.getScale( mFrame );
		mTranslate	+= hand.getTranslation( mFrame ) * kTranslateSpeed;
	}
	mFrame = frame;
}

// Prepare window
void MotionApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1024, 768 );
	settings->setFrameRate( 60.0f );
}

// Take screen shot
void MotionApp::screenShot()
{
#if defined( CINDER_MSW )
	fs::path path = getAppPath();
#else
	fs::path path = getAppPath().parent_path();
#endif
	writeImage( path / fs::path( "frame" + toString( getElapsedFrames() ) + ".png" ), copyWindowSurface() );
}

// Set up
void MotionApp::setup()
{
	// Set up OpenGL
	gl::enable( GL_LINE_SMOOTH );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST ); 
	gl::enable( GL_POLYGON_SMOOTH );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	
	// Set up camera
	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 45.0f, 0.01f, 10.0f );
	mCamera.lookAt( Vec3f( 0.0f, 0.0f, 3.0f ), Vec3f::zero() );
	
	// Light
	mLight = new gl::Light( gl::Light::DIRECTIONAL, 0 );
	mLight->setPosition( mCamera.getEyePoint() );
	mLight->setDiffuse( ColorAf( 1.0f, 0.5f, 0.0f, 1.0f ) );
	
	// Initialize modelview
	mRotAngle	= 0.0f;
	mRotAxis	= Vec3f::zero();
	mScale		= 1.0f;
	mTranslate	= Vec3f::zero();
	mTransform.setToIdentity();
	
	// Start device
	mLeap 		= Device::create();
	mCallbackId = mLeap->addCallback( &MotionApp::onFrame, this );

	// Params
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	mParams = params::InterfaceGl( "Params", Vec2i( 200, 105 ) );
	mParams.addParam( "Frame rate",		&mFrameRate,						"", true );
	mParams.addParam( "Full screen",	&mFullScreen,						"key=f"		);
	mParams.addButton( "Screen shot",	bind( &MotionApp::screenShot, this ), "key=space" );
	mParams.addButton( "Quit",			bind( &MotionApp::quit, this ),		"key=q" );
}

// Quit
void MotionApp::shutdown()
{
	delete mLight;
	mLeap->removeCallback( mCallbackId );
}

// Runs update logic
void MotionApp::update()
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
	
	mRotAngle	= lerp( mRotAngle, 0.0f, kRestitution );
	mRotAxis	= mRotAxis.lerp( kRestitution, Vec3f::zero() );
	mScale		= lerp( mScale, 1.0f, kRestitution );
	mTranslate	= mTranslate.lerp( kRestitution, Vec3f::zero() );
	
	mTransform.setToIdentity();
	mTransform.translate( mTranslate );
	mTransform.rotate( mRotAxis * mRotAngle );
	mTransform.translate( mTranslate * -1.0f );
	mTransform.translate( mTranslate );
	mTransform.scale( Vec3f::one() * mScale );
}

// Run application
CINDER_APP_BASIC( MotionApp, RendererGl )
