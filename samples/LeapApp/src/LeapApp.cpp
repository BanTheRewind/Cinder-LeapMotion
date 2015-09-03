/*
* 
* Copyright (c) 2015, Ban the Rewind
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

#include "cinder/app/App.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "Cinder-LeapMotion.h"

class LeapApp : public ci::app::App
{
public:
	LeapApp();

	void						draw() override;
	void						update() override;
private:
	LeapMotion::DeviceRef		mDevice;
	Leap::Frame					mFrame;
	void 						onFrame( Leap::Frame frame );

	ci::CameraPersp				mCamera;

	float						mFrameRate;
	bool						mFullScreen;
	ci::params::InterfaceGlRef	mParams;
	void						screenShot();
};

#include "cinder/app/RendererGl.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace LeapMotion;
using namespace std;

LeapApp::LeapApp()
{
	mFrameRate	= 0.0f;
	mFullScreen	= false;

	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f, 1.0f, 1000.0f );
	mCamera.lookAt( vec3( 0.0f, 250.0f, 500.0f ), vec3( 0.0f, 250.0f, 0.0f ) );
	
	mDevice = Device::create();
	mDevice->connectEventHandler( &LeapApp::onFrame, this );

	mParams = params::InterfaceGl::create( "Params", ivec2( 200, 105 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,						"", true );
	mParams->addParam( "Full screen",	&mFullScreen ).key( "f" );
	mParams->addButton( "Screen shot",	bind( &LeapApp::screenShot, this ),	"key=space" );
	mParams->addButton( "Quit",			bind( &LeapApp::quit, this ),		"key=q" );

	gl::enable( GL_LINE_SMOOTH );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
	gl::enable( GL_POLYGON_SMOOTH );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	gl::enableVerticalSync();
}

void LeapApp::draw()
{
	const gl::ScopedViewport scopedViewport( ivec2( 0 ), getWindowSize() );
	const gl::ScopedMatrices scopedMatrices;
	const gl::ScopedBlendAlpha scopedBlendAlpha;
	gl::clear( Colorf::white() );
	gl::setMatrices( mCamera );

	gl::enableDepthRead();
	gl::enableDepthWrite();
	
	float headLength = 6.0f;
	float headRadius = 3.0f;
	const Leap::HandList& hands = mFrame.hands();
	for ( Leap::HandList::const_iterator handIter = hands.begin(); handIter != hands.end(); ++handIter ) {
		const Leap::Hand& hand = *handIter;

		// Get hand data
		const vec3 handDir			= LeapMotion::toVec3( hand.direction() );
		const vec3 palmNorm			= LeapMotion::toVec3( hand.palmNormal() );
		const vec3 palmPos			= LeapMotion::toVec3( hand.palmPosition() );
		const vec3 palmVel			= LeapMotion::toVec3( hand.palmVelocity() );
		const vec3 sphereCenter		= LeapMotion::toVec3( hand.sphereCenter() );
		const float sphereRadius	= hand.sphereRadius();

		// Hand sphere
		{
			const gl::ScopedColor scopedColor( ColorAf( Colorf::gray( 0.9f ), 0.5f ) );
			gl::enableWireframe();
			gl::drawSphere( sphereCenter, sphereRadius, 16 );
			gl::disableWireframe();
		}

		// Hand plane
		{
			const gl::ScopedColor scopedColor( ColorAf( 0.75f, 0.0f, 0.75f, 0.25f ) );
			const gl::ScopedModelMatrix scopedModelMatrix;
			gl::translate( palmPos );
			gl::rotate( quat( palmPos, handDir ) );
			for ( float i = 0.25f; i <= 1.0f; i += 0.25f ) {
				gl::drawStrokedCircle( vec2( 0.0f ), sphereRadius * i, 16 );
			}
		}

		// Hand direction
		{
			const gl::ScopedColor scopedColor( ColorAf( 1.0f, 0.0f, 1.0f, 0.5f ) );
			gl::drawVector( palmPos, palmPos + handDir * 30.0f, headLength, headRadius );
		}

		// Hand normal
		{
			const gl::ScopedColor scopedColor( ColorAf( 0.0f, 0.0f, 1.0f, 0.5f ) );
			gl::drawVector( palmPos, palmPos + palmNorm * 30.0f, headLength, headRadius );
		}

		// Hand velocity
		{
			const gl::ScopedColor scopedColor( ColorAf( 0.0f, 1.0f, 0.0f, 0.5f ) );
			gl::drawVector( palmPos, palmPos + palmVel * 0.05f, headLength, headRadius );
		}

		// Fingers
		const Leap::PointableList& pointables = hand.pointables();
		for ( Leap::PointableList::const_iterator pointIter = pointables.begin(); pointIter != pointables.end(); ++pointIter ) {
			const Leap::Pointable& pointable = *pointIter;

			// Get pointable data
			const vec3 dir		= LeapMotion::toVec3( pointable.direction() );
			const bool isTool	= pointable.isTool();
			const float length	= pointable.length();
			const vec3 tipPos	= LeapMotion::toVec3( pointable.tipPosition() );
			const vec3 tipVel	= LeapMotion::toVec3( pointable.tipVelocity() );
			const float width	= pointable.width();
			const vec3 basePos	= tipPos + dir * -length;
			
			// Draw line representing pointable's length
			{
				const gl::ScopedColor scopedColor( ColorAf::gray( 0.3f ) );
				gl::drawLine( basePos, tipPos );
			}

			// Tip position
			{
				const gl::ScopedColor scopedColor( isTool ? Colorf( 1.0f, 0.0f, 0.0f ) : Colorf::black() );
				const gl::ScopedModelMatrix scopedModelMatrix;
				gl::translate( tipPos );
				gl::drawStrokedCircle( vec2( 0.0f ), width, 16 );
			}

			// Finger velocity
			{
				const gl::ScopedColor scopedColor( ColorAf( 0.0f, 1.0f, 0.0f, 0.5f ) );
				gl::drawVector( tipPos, tipPos + tipVel * 0.05f, headLength, headRadius );
			}
		}
	}
	
	mParams->draw();
}

void LeapApp::onFrame( Leap::Frame frame )
{
	mFrame = frame;
}

void LeapApp::screenShot()
{
	writeImage( getAppPath() / fs::path( "frame" + toString( getElapsedFrames() ) + ".png" ), copyWindowSurface() );
}

void LeapApp::update()
{
	mFrameRate = getAverageFps();

	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
	}
}

CINDER_APP( LeapApp, RendererGl, []( App::Settings* settings )
{
	settings->setWindowSize( 1024, 768 );
	settings->disableFrameRate();
} )
