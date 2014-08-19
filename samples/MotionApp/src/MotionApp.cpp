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
#include "cinder/gl/Light.h"
#include "cinder/params/Params.h"
#include "Cinder-LeapMotion.h"

class MotionApp : public ci::app::AppBasic
{
public:
	void						draw();
	void						prepareSettings( ci::app::AppBasic::Settings* settings );
	void						setup();
	void						shutdown();
	void						update();
private:
	enum
	{
		FREE, ROTATE, SCALE, TRANSLATE
	} typedef Motion;
	bool						mConstrainMotion;
	
	Leap::Frame					mFrame;
	LeapMotion::DeviceRef		mDevice;
	void						onFrame( Leap::Frame frame );
	
	ci::gl::Light*				mLight;
	
	float						mRotAngle;
	ci::Vec3f					mRotAxis;
	float						mScale;
	ci::Matrix44f				mTransform;
	ci::Vec3f					mTranslate;
	
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

static const float	kRestitution	= 0.021f;
static const float	kRotSpeed		= 0.033f;
static const float	kTranslateSpeed	= 0.0033f;

void MotionApp::draw()
{
	gl::setViewport( getWindowBounds() );
	gl::clear( Colorf::white() );
	gl::setMatrices( mCamera );

	gl::enableAlphaBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();
	
	gl::enable( GL_LIGHTING );
	mLight->enable();
	
	gl::color( ColorAf::gray( 0.5f ) );
	gl::pushMatrices();
	gl::multModelView( mTransform );
	gl::drawCube( Vec3f::zero(), Vec3f::one() );
	gl::popMatrices();
	
	mLight->disable();
	gl::disable( GL_LIGHTING );
	
	mParams->draw();
}

void MotionApp::onFrame( Leap::Frame frame )
{
	const Leap::HandList& hands = frame.hands();
	for ( Leap::HandList::const_iterator handIter = hands.begin(); handIter != hands.end(); ++handIter ) {
		const Leap::Hand& hand = *handIter;

		Motion motion = FREE;
		
		// When constraining to a motion, this routine sorts the probabilities
		// for each motion and uses the highest value to select a type of motion
		if ( mConstrainMotion ) {
			map<float, Motion> motions;
			vector<float> values;
			motions[ hand.rotationProbability( mFrame ) ]		= ROTATE;
			motions[ hand.scaleProbability( mFrame ) ]			= SCALE;
			motions[ hand.translationProbability( mFrame ) ]	= TRANSLATE;
			for ( map<float, Motion>::const_iterator iter = motions.begin(); iter != motions.end(); ++iter ) {
				values.push_back( iter->first );
			}
			sort( values.begin(), values.end(), []( float a, float b )
			{
				return a > b;
			});
			motion = motions[ values.at( 0 ) ];
		}
		
		if ( motion == FREE || motion == ROTATE ) {
			mRotAngle	+= hand.rotationAngle( mFrame ) * kRotSpeed;
			mRotAxis	+= LeapMotion::toVec3f( hand.rotationAxis( mFrame ) ) * -1.0f; // Mirror
		}
		if ( motion == FREE || motion == SCALE ) {
			mScale		*= hand.scaleFactor( mFrame );
		}
		if ( motion == FREE || motion == TRANSLATE ) {
			mTranslate	+= LeapMotion::toVec3f( hand.translation( mFrame ) ) * kTranslateSpeed;
		}
	}
	mFrame = frame;
}

void MotionApp::prepareSettings( Settings* settings )
{
	settings->setWindowSize( 1024, 768 );
	settings->setFrameRate( 60.0f );
}

void MotionApp::screenShot()
{
#if defined( CINDER_MSW )
	fs::path path = getAppPath();
#else
	fs::path path = getAppPath().parent_path();
#endif
	writeImage( path / fs::path( "frame" + toString( getElapsedFrames() ) + ".png" ), copyWindowSurface() );
}

void MotionApp::setup()
{
	gl::enable( GL_POLYGON_SMOOTH );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	
	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 45.0f, 0.01f, 10.0f );
	mCamera.lookAt( Vec3f( 0.0f, 0.0f, 3.0f ), Vec3f::zero() );
	
	mLight = new gl::Light( gl::Light::DIRECTIONAL, 0 );
	mLight->setPosition( mCamera.getEyePoint() );
	mLight->setDiffuse( ColorAf( 1.0f, 0.5f, 0.0f, 1.0f ) );
	
	mConstrainMotion = false;
	
	mRotAngle	= 0.0f;
	mRotAxis	= Vec3f::zero();
	mScale		= 1.0f;
	mTranslate	= Vec3f::zero();
	mTransform.setToIdentity();
	
	// Start device
	mDevice = Device::create();
	mDevice->connectEventHandler( &MotionApp::onFrame, this );

	// Params
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	mParams = params::InterfaceGl::create( "Params", Vec2i( 200, 120 ) );
	mParams->addParam( "Frame rate",		&mFrameRate),				"", true );
	mParams->addParam( "Constrain motion",	&mConstrainMotion ).key( "m" );
	mParams->addParam( "Full screen",		&mFullScreen ).key( "f" );
	mParams->addButton( "Screen shot",		[ & ]() { screenShot(); },	"key=space" );
	mParams->addButton( "Quit",				[ & ]() { quit(); },		"key=q" );
}

void MotionApp::shutdown()
{
	if ( mLight != 0 ) {
		delete mLight;
		mLight = 0;
	}
}

void MotionApp::update()
{
	mFrameRate = getAverageFps();

	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
	}

	// Smooth animation
	mRotAngle	= lerp( mRotAngle, 0.0f, kRestitution );
	mRotAxis	= mRotAxis.lerp( kRestitution, Vec3f::zero() );
	mScale		= lerp( mScale, 1.0f, kRestitution );
	mTranslate	= mTranslate.lerp( kRestitution, Vec3f::zero() );
	
	// Update cube transform
	mTransform.setToIdentity();
	mTransform.translate( mTranslate );
	mTransform.rotate( mRotAxis * mRotAngle );
	mTransform.translate( mTranslate * -1.0f );
	mTransform.translate( mTranslate );
	mTransform.scale( Vec3f::one() * mScale );
}

CINDER_APP_BASIC( MotionApp, RendererGl )
