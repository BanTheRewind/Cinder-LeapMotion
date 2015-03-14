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
#include "cinder/params/Params.h"
#include "Cinder-LeapMotion.h"
#include "Ribbon.h"

class TracerApp : public ci::app::App
{
public:
	void						draw() override;
	void						setup() override;
	void						update() override;
private:
	RibbonMap					mRibbons;

	Leap::Frame					mFrame;
	LeapMotion::DeviceRef		mDevice;
	
	ci::gl::FboRef				mFbo[ 3 ];
	ci::gl::GlslProgRef			mShader[ 2 ];

	ci::CameraPersp				mCamera;

	float						mFrameRate;
	ci::params::InterfaceGlRef	mParams;
	void						screenShot();
};

#include "cinder/app/RendererGl.h"
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
	// Add to accumulation buffer
	{
		gl::disable( GL_TEXTURE_2D );
		//gl::ScopedFramebuffer scopedFramebuffer( mFbo[ 0 ] );
		gl::viewport( mFbo[ 0 ]->getSize() );
		gl::setMatricesWindow( mFbo[ 0 ]->getSize() );
		gl::enableAlphaBlending();

		// Dim last frame
		gl::color( ColorAf( Colorf::black(), 0.03f ) );
		gl::drawSolidRect( Rectf( mFbo[ 0 ]->getBounds() ) );

		// Draw finger tips into the accumulation buffer
		gl::setMatrices( mCamera );
		gl::enableAdditiveBlending();
		for ( RibbonMap::const_iterator iter = mRibbons.begin(); iter != mRibbons.end(); ++iter ) {
			iter->second.draw();
		}
	}

	// Blur the accumulation buffer
	//gl::enable( GL_TEXTURE_2D );
	//gl::enableAlphaBlending();
	//gl::color( ColorAf::white() );
	//vec2 pixel	= ( vec2( 1.0f ) / vec2( mFbo[ 0 ]->getSize() ) ) * 3.0f;
	//for ( size_t i = 0; i < 2; ++i ) {
	//	gl::ScopedFramebuffer scopedFramebuffer( mFbo[ i + 1 ] );
	//	gl::clear();
	//	
	//	gl::ScopedGlslProg scopedGlslProg( mShader[ i ] );
	//	mShader[ i ]->uniform( "size",	pixel );
	//	mShader[ i ]->uniform( "tex",	0 );
	//	
	//	gl::ScopedTextureBind scopedTextureBind( mFbo[ i ]->getColorTexture() );
	//	gl::drawSolidRect( Rectf( mFbo[ i ]->getBounds() ) );
	//}

	//// Draw blurred image
	//gl::setMatricesWindow( getWindowSize(), false );
	//gl::clear();
	//gl::color( ColorAf::white() );
	//{
	//	gl::ScopedTextureBind scopedTextureBind( mFbo[ 0 ]->getColorTexture() );
	//	gl::drawSolidRect( Rectf( getWindowBounds() ) );
	//}

	//gl::color( ColorAf( Colorf::white(), 0.8f ) );
	//{
	//	gl::ScopedTextureBind scopedTextureBind( mFbo[ 2 ]->getColorTexture() );
	//	gl::drawSolidRect( Rectf( getWindowBounds() ) );
	//}
	//gl::disableAlphaBlending();
	//gl::disable( GL_TEXTURE_2D );

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

void TracerApp::setup()
{
	mCamera	= CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f, 0.01f, 1000.0f );
	mCamera.lookAt( vec3( 0.0f, 93.75f, 250.0f ), vec3( 0.0f, 250.0f, 0.0f ) );
	
	mDevice = Device::create();
	mDevice->connectEventHandler( [ & ]( Leap::Frame frame )
	{
		mFrame = frame;
	} );

	try {
		mShader[ 0 ] = gl::GlslProg::create( loadResource( RES_GLSL_PASS_THROUGH_VERT ), loadResource( RES_GLSL_BLUR_X_FRAG ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << "Unable to compile blur X shader: \n" << string( ex.what() ) << "\n";
		quit();
	}
	try {
		mShader[ 1 ] = gl::GlslProg::create( loadResource( RES_GLSL_PASS_THROUGH_VERT ), loadResource( RES_GLSL_BLUR_Y_FRAG ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << "Unable to compile blur Y shader: \n" << string( ex.what() ) << "\n";
		quit();
	}

	mFrameRate = 0.0f;
	mParams = params::InterfaceGl::create( "Params", ivec2( 200, 105 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,				"", true );
	mParams->addButton( "Screen shot",	[ & ]() { screenShot(); },	"key=space" );
	mParams->addButton( "Quit",			[ & ]() { quit(); },		"key=q" );

	gl::enable( GL_POLYGON_SMOOTH );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );

	gl::Texture2d::Format textureFormat;
	textureFormat.magFilter( GL_LINEAR ).minFilter( GL_LINEAR ).wrap( GL_CLAMP_TO_EDGE );
#if defined( GL_RGBA32F )
	textureFormat.internalFormat( GL_RGBA32F );
#else if defined( GL_RGBA32F_ARB )
	textureFormat.internalFormat( GL_RGBA32F_ARB );
#endif
	for ( size_t i = 0; i < 3; ++i ) {
		mFbo[ i ] = gl::Fbo::create( getWindowWidth(), getWindowHeight(), gl::Fbo::Format().colorTexture( textureFormat ) );
		gl::ScopedFramebuffer scopedFramebuffer( mFbo[ i ] );
		gl::viewport( mFbo[ i ]->getSize() );
		gl::clear();
	}
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
}

void TracerApp::update()
{
	mFrameRate = getAverageFps();

	// Process hand data
	const Leap::HandList& hands = mFrame.hands();
	for ( Leap::HandList::const_iterator handIter = hands.begin(); handIter != hands.end(); ++handIter ) {
		const Leap::Hand& hand = *handIter;
		
		const Leap::PointableList& pointables = hand.pointables();
		for ( Leap::PointableList::const_iterator pointIter = pointables.begin(); pointIter != pointables.end(); ++pointIter ) {
			const Leap::Pointable& pointable = *pointIter;

			int32_t id = pointable.id();
			if ( mRibbons.find( id ) == mRibbons.end() ) {
				vec3 v = randVec3f() * 0.01f;
				v.x = math<float>::abs( v.x );
				v.y = math<float>::abs( v.y );
				v.z = math<float>::abs( v.z );
				Colorf color( ColorModel::CM_RGB, v );
				Ribbon ribbon( id, color );
				mRibbons[ id ] = ribbon;
			}
			float width = math<float>::abs( pointable.tipVelocity().y ) * 0.0025f;
			width		= math<float>::max( width, 5.0f );
			mRibbons[ id ].addPoint( LeapMotion::toVec3( pointable.tipPosition() ), width );
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
	settings->setResizable( false );
	settings->setWindowSize( 1024, 768 );
} )
