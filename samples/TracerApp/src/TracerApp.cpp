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
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Shader.h"
#include "cinder/params/Params.h"
#include "Cinder-LeapMotion.h"
#include "Ribbon.h"

class TracerApp : public ci::app::App
{
public:
	void						draw() override;
	void						resize() override;
	void						setup() override;
	void						update() override;
private:
	RibbonMap					mRibbons;

	Leap::Frame					mFrame;
	LeapMotion::DeviceRef		mDevice;
	
	ci::gl::FboRef				mFbo[ 3 ];
	ci::gl::GlslProgRef			mGlslProg[ 2 ];

	ci::CameraPersp				mCamera;

	float						mFrameRate;
	bool						mFullScreen;
	ci::params::InterfaceGlRef	mParams;
	void						screenShot();
};

#include "cinder/app/RendererGl.h"
#include "cinder/gl/Context.h"
#include "cinder/ImageIo.h"
#include "cinder/Rand.h"
#include "cinder/Utilities.h"
#include "Resources.h"

// Imports
using namespace ci;
using namespace ci::app;
using namespace LeapMotion;
using namespace std;

void TracerApp::draw()
{
	gl::viewport( getWindowSize() );
	gl::enableAlphaBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();

	// Add to accumulation buffer
	{
		gl::ScopedFramebuffer scopedFramebuffer( mFbo[ 0 ] );

		// Dim last frame
		gl::setMatricesWindow( getWindowSize() );
		gl::color( ColorAf( Colorf::black(), 0.075f ) );
		gl::color( Colorf::black() );
		gl::drawSolidRect( Rectf( mFbo[ 0 ]->getBounds() ) );

		// Draw finger tips into the accumulation buffer
		gl::setMatrices( mCamera );
		gl::enableAdditiveBlending();
		for ( RibbonMap::const_iterator iter = mRibbons.begin(); iter != mRibbons.end(); ++iter ) {
			iter->second.draw();
		}
	}

	gl::setMatricesWindow( getWindowSize() );
	gl::disableDepthRead();
	gl::disableDepthWrite();
	gl::color( ColorAf::white() );

	// Blur the accumulation buffer
	vec2 pixel	= ( vec2( 1.0f ) / vec2( mFbo[ 0 ]->getSize() ) ) * 3.0f;
	for ( size_t i = 0; i < 2; ++i ) {
		gl::ScopedFramebuffer scopedFramebuffer( mFbo[ i + 1 ] );
		gl::clear();
		gl::ScopedGlslProg scopedGlslProg( mGlslProg[ i ] );
		gl::ScopedTextureBind scopedTextureBind( mFbo[ i ]->getColorTexture() );
		
		mGlslProg[ i ]->uniform( "uSize",		pixel );
		mGlslProg[ i ]->uniform( "uTexture",	0 );

		gl::drawSolidRect( getWindowBounds() );
	}

	// Draw blurred image
	gl::clear();
	
	gl::color( ColorAf::white() );
	gl::draw( mFbo[ 0 ]->getColorTexture(), getWindowBounds() );
	gl::color( ColorAf( Colorf::white(), 0.8f ) );
	gl::draw( mFbo[ 2 ]->getColorTexture(), getWindowBounds() );

	mParams->draw();
}

void TracerApp::screenShot()
{
#if defined( CINDER_MSW )
	fs::path path = getAppPath();
#else
	fs::path path = getAppPath().parent_path();
#endif
	writeImage( path / fs::path( "frame" + toString( getElapsedFrames() ) + ".png" ), copyWindowSurface() );
}

void TracerApp::resize()
{
	for ( size_t i = 0; i < 3; ++i ) {
		gl::Fbo::Format format;
		format.colorTexture( 
#if defined( GL_RGBA32F )
			gl::Texture2d::Format().internalFormat( GL_RGBA32F ) 
#elif defined( GL_RGBA32F_ARB )
			gl::Texture2d::Format().internalFormat( GL_RGBA32F_ARB ) 
#endif
			).disableDepth();
		mFbo[ i ] = gl::Fbo::create( getWindowWidth(), getWindowHeight(), format );
		gl::ScopedFramebuffer scopedFramebuffer( mFbo[ i ] );
		gl::viewport( mFbo[ i ]->getSize() );
		gl::clear();
	}
}

void TracerApp::setup()
{
	mFrameRate	= 0.0f;
	mFullScreen	= isFullScreen();

	mCamera	= CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f, 0.01f, 1000.0f );
	mCamera.lookAt( vec3( 0.0f, 93.75f, 250.0f ), vec3( 0.0f, 250.0f, 0.0f ) );
	
	mDevice = Device::create();
	mDevice->connectEventHandler( [ & ]( Leap::Frame frame )
	{
		mFrame = frame;
	} );

	// Compile shaders
	try {
		mGlslProg[ 0 ] = gl::GlslProg::create( loadResource( RES_GLSL_PASS_THROUGH_VERT ), loadResource( RES_GLSL_BLUR_X_FRAG ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << "Unable to compile blur X shader: \n" << string( ex.what() ) << "\n";
		quit();
	}
	try {
		mGlslProg[ 1 ] = gl::GlslProg::create( loadResource( RES_GLSL_PASS_THROUGH_VERT ), loadResource( RES_GLSL_BLUR_Y_FRAG ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << "Unable to compile blur Y shader: \n" << string( ex.what() ) << "\n";
		quit();
	}

	resize();

	mParams = params::InterfaceGl::create( "Params", ivec2( 200, 105 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,				"", true );
	mParams->addParam( "Full screen",	&mFullScreen ).key( "f" );
	mParams->addButton( "Screen shot",	[ &]() { screenShot(); },	"key=space" );
	mParams->addButton( "Quit",			[ & ]() { quit(); },		"key=q" );
}

void TracerApp::update()
{
	mFrameRate = getAverageFps();

	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}

	// Process hand data
	const Leap::HandList& hands = mFrame.hands();
	for ( Leap::HandList::const_iterator handIter = hands.begin(); handIter != hands.end(); ++handIter ) {
		const Leap::Hand& hand = *handIter;
		
		const Leap::FingerList& fingers = hand.fingers();
		for ( Leap::FingerList::const_iterator iter = fingers.begin(); iter != fingers.end(); ++iter ) {
			const Leap::Finger& finger = *iter;
			if ( finger.isExtended() ) {
				int32_t id = finger.id();
				if ( mRibbons.find( id ) == mRibbons.end() ) {
					vec3 v = randVec3f();
					v.x = math<float>::abs( v.x );
					v.y = math<float>::abs( v.y );
					v.z = math<float>::abs( v.z );
					Colorf color( ColorModel::CM_RGB, v );
					Ribbon ribbon( id, color );
					mRibbons[ id ] = ribbon;
				}
				float width = math<float>::abs( finger.tipVelocity().y ) * 0.00075f;
				width		= math<float>::max( width, 2.0f );
				mRibbons.at( id ).addPoint( LeapMotion::toVec3( finger.tipPosition() ), width );
			}
		}
	}

	// Update ribbons
	for ( RibbonMap::iterator iter = mRibbons.begin(); iter != mRibbons.end(); ++iter ) {
		iter->second.update();
	}
}

CINDER_APP( TracerApp, RendererGl( RendererGl::Options().coreProfile( true ).version( 3, 3 ).msaa( 16 ) ), []( App::Settings* settings )
{
	settings->setFrameRate( 60.0f );
	settings->setWindowSize( 1024, 768 );
} )
