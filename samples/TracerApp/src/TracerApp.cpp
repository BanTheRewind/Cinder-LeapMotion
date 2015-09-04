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
#include "Ribbon.h"

class TracerApp : public ci::app::App
{
public:
	TracerApp();

	void						draw() override;
	void						resize() override;
	void						update() override;
private:
	RibbonMap					mRibbons;

	Leap::Frame					mFrame;
	LeapMotion::DeviceRef		mDevice;
	
	ci::gl::BatchRef			mBatchBlur;
	ci::gl::FboRef				mFbo[ 3 ];

	ci::CameraPersp				mCamera;

	float						mFrameRate;
	bool						mFullScreen;
	ci::params::InterfaceGlRef	mParams;
	void						screenShot();
};

#include "cinder/app/RendererGl.h"
#include "cinder/gl/Context.h"
#include "cinder/ImageIo.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"
#include "cinder/Utilities.h"
#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace LeapMotion;
using namespace std;
TracerApp::TracerApp()
{
	mFrameRate = 0.0f;
	mFullScreen = isFullScreen();

	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f, 0.01f, 1000.0f );
	mCamera.lookAt( vec3( 0.0f, 93.75f, 250.0f ), vec3( 0.0f, 250.0f, 0.0f ) );

	mDevice = Device::create();
	mDevice->connectEventHandler( [ & ]( Leap::Frame frame )
	{
		mFrame = frame;
	} );

	gl::GlslProgRef glsl;
	try {
		glsl = gl::GlslProg::create( gl::GlslProg::Format()
			.vertex( loadResource( RES_GLSL_PASS_THROUGH_VERT ) )
			.fragment( loadResource( RES_GLSL_BLUR_FRAG ) )
			.version( 330 ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		CI_LOG_V( "Unable to compile blur X shader: \n" << string( ex.what() ) );
		quit();
		return;
	}

	glsl->uniform( "uSampler", 0 );
	mBatchBlur = gl::Batch::create( geom::Rect()
		.texCoords( vec2( 0.0f ), vec2( 1.0f, 0.0f ), vec2( 1.0f ), vec2( 0.0f, 1.0f ) ),
		glsl );

	resize();

	mParams = params::InterfaceGl::create( "Params", ivec2( 200, 105 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,				"", true );
	mParams->addParam( "Full screen",	&mFullScreen ).key( "f" );
	mParams->addButton( "Screen shot",	[ & ]() { screenShot(); },	"key=space" );
	mParams->addButton( "Quit",			[ & ]() { quit(); },		"key=q" );
}

void TracerApp::draw()
{
	const gl::ScopedViewport scopedViewport( ivec2( 0 ), getWindowSize() );
	gl::enableDepthRead();
	gl::enableDepthWrite();

	// Add to accumulation buffer
	{
		gl::ScopedFramebuffer scopedFramebuffer( mFbo[ 0 ] );

		// Dim last frame
		{
			const gl::ScopedMatrices scopedMatrices;
			const gl::ScopedBlendAlpha scopedBlendAlpha;
			const gl::ScopedColor scopedColor( ColorAf( Colorf::black(), 0.075f ) );
			gl::setMatricesWindow( getWindowSize() );
			gl::drawSolidRect( Rectf( mFbo[ 0 ]->getBounds() ) );
		}

		// Draw finger tips into the accumulation buffer
		{
			const gl::ScopedMatrices scopedMatrices;
			const gl::ScopedBlendAdditive scopedBlendAdditive;
			gl::setMatrices( mCamera );
			for ( RibbonMap::const_iterator iter = mRibbons.begin(); iter != mRibbons.end(); ++iter ) {
				iter->second.draw();
			}
		}
	}

	const gl::ScopedMatrices scopedMatrices;
	gl::setMatricesWindow( getWindowSize() );
	gl::disableDepthRead();
	gl::disableDepthWrite();

	// Blur the accumulation buffer
	vec2 pixel = ( vec2( 1.0f ) / vec2( mFbo[ 0 ]->getSize() ) ) * 3.0f;
	for ( size_t i = 0; i < 2; ++i ) {
		gl::ScopedFramebuffer scopedFramebuffer( mFbo[ i + 1 ] );
		gl::clear();
		const gl::ScopedModelMatrix scopedModelMatrix;
		gl::ScopedTextureBind scopedTextureBind( mFbo[ i ]->getColorTexture() );
		mBatchBlur->getGlslProg()->uniform( "uSize", pixel * ( i == 0 ? vec2( 1.0f, 0.0f ) : vec2( 0.0f, 1.0f ) ) );
		gl::translate( getWindowCenter() );
		gl::scale( getWindowSize() );
		mBatchBlur->draw();
	}

	// Draw blurred image
	gl::clear();
	{
		const gl::ScopedColor scopedColor( ColorAf::white() );
		gl::draw( mFbo[ 0 ]->getColorTexture(), getWindowBounds() );
	}
	{
		const gl::ScopedColor scopedColor( ColorAf( Colorf::white(), 0.8f ) );
		gl::draw( mFbo[ 2 ]->getColorTexture(), getWindowBounds() );
	}

	mParams->draw();
}

void TracerApp::screenShot()
{
	writeImage( getAppPath() / fs::path( "frame" + toString( getElapsedFrames() ) + ".png" ), copyWindowSurface() );
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
					vec3 v = randVec3();
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
