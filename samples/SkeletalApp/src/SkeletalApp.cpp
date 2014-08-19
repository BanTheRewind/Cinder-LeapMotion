/*
* 
* Copyright (c) 2014, Ban the Rewind
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
#include "Cinder-LeapMotion.h"

class SkeletalApp : public ci::app::AppBasic
{
public:
	void						draw();
	void						prepareSettings( ci::app::AppBasic::Settings* settings );
	void						setup();
	void						update();
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

#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace LeapMotion;
using namespace std;

void SkeletalApp::draw()
{
	gl::setViewport( getWindowBounds() );
	gl::clear( Colorf::white() );
	gl::setMatrices( mCamera );

	gl::enableAlphaBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::color( ColorAf::black() );

	const Leap::HandList& hands = mFrame.hands();
	for ( Leap::HandList::const_iterator handIter = hands.begin(); handIter != hands.end(); ++handIter ) {
		const Leap::Hand& hand	= *handIter;
		const Leap::Arm& arm	= hand.arm();
		
		Vec3f palm				= LeapMotion::toVec3f( hand.palmPosition() );
		Vec3f elbow				= LeapMotion::toVec3f( arm.elbowPosition() );
		Vec3f rotation			= Vec3f( hand.direction().pitch(), hand.direction().yaw(), hand.palmNormal().roll() );
		Vec3f wrist				= LeapMotion::toVec3f( arm.wristPosition() );

		gl::drawLine( elbow, wrist );

		vector<Vec3f> knuckles;
		const Leap::FingerList fingers = hand.fingers();
		for ( Leap::FingerList::const_iterator fingerIter = fingers.begin(); fingerIter != fingers.end(); ++fingerIter ) {
			const Leap::Finger& finger = *fingerIter;

			for ( int32_t i = 0; i < 4; ++i ) {
				const Leap::Bone& bone = finger.bone( (Leap::Bone::Type)i );
				Vec3f center	= LeapMotion::toVec3f( bone.center() );
				Vec3f direction	= LeapMotion::toVec3f( bone.direction() );
				Vec3f start		= center - direction * bone.length() * 0.5f;
				Vec3f end		= center + direction * bone.length() * 0.5f;
				
				if ( i == 0 ) {
					knuckles.push_back( start );
					gl::drawLine( wrist, start );
				} else {
					gl::drawLine( start, end );
				}
			}
		}

		if ( knuckles.size() > 1 ) {
			for ( size_t i = 1; i < knuckles.size(); ++i ) {
				const Vec3f& v0 = knuckles.at( i - 1 );
				const Vec3f& v1 = knuckles.at( i );
				gl::drawLine( v0,		v1 );
			}
			gl::drawLine( elbow, knuckles.at( 0 ) );
		}

	}

	mParams->draw();
}

void SkeletalApp::onFrame( Leap::Frame frame )
{
	mFrame = frame;
}

void SkeletalApp::prepareSettings( Settings* settings )
{
	settings->setWindowSize( 1024, 768 );
	settings->setFrameRate( 60.0f );
}

void SkeletalApp::screenShot()
{
#if defined( CINDER_MSW )
	fs::path path = getAppPath();
#else
	fs::path path = getAppPath().parent_path();
#endif
	writeImage( path / fs::path( "frame" + toString( getElapsedFrames() ) + ".png" ), copyWindowSurface() );
}

void SkeletalApp::setup()
{
	gl::enable( GL_LINE_SMOOTH );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST ); 
	gl::enable( GL_POLYGON_SMOOTH );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );

	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f, 1.0f, 5000.0f );
	mCamera.lookAt( Vec3f( 0.0f, 300.0f, 300.0f ), Vec3f( 0.0f, 250.0f, 0.0f ) );
	
	mDevice = Device::create();
	mDevice->connectEventHandler( [ & ]( Leap::Frame frame )
	{
		mFrame = frame;
	} );

	mFrameRate	= 0.0f;
	mFullScreen	= false;
	mParams = params::InterfaceGl::create( "Params", Vec2i( 200, 105 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,				"", true );
	mParams->addParam( "Full screen",	&mFullScreen ).key( "f" );
	mParams->addButton( "Screen shot",	[ & ]() { screenShot(); },	"key=space" );
	mParams->addButton( "Quit",			[ & ]() { quit(); },		"key=q" );
}

void SkeletalApp::update()
{
	mFrameRate = getAverageFps();

	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
	}
}

CINDER_APP_BASIC( SkeletalApp, RendererGl )
