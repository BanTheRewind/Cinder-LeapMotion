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
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/params/Params.h"
#include "Cinder-LeapMotion.h"
#include "Ribbon.h"

class TracerApp : public ci::app::AppBasic
{
public:
	void						draw();
	void						prepareSettings( ci::app::AppBasic::Settings* settings );
	void						setup();
	void						update();
private:
	RibbonMap					mRibbons;

	Leap::Frame					mFrame;
	LeapMotion::DeviceRef		mDevice;
	
	ci::gl::Fbo					mFbo[ 3 ];
	ci::gl::GlslProg			mShader[ 2 ];

	ci::CameraPersp				mCamera;

	float						mFrameRate;
	ci::params::InterfaceGlRef	mParams;
	void						screenShot();
};

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
	mFbo[ 0 ].bindFramebuffer();
	gl::setViewport( mFbo[ 0 ].getBounds() );
	gl::setMatricesWindow( mFbo[ 0 ].getSize() );
	gl::enableAlphaBlending();

	// Dim last frame
	gl::color( ColorAf( Colorf::black(), 0.03f ) );
	gl::drawSolidRect( Rectf( mFbo[ 0 ].getBounds() ) );

	// Draw finger tips into the accumulation buffer
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
	Vec2f pixel	= ( Vec2f::one() / Vec2f( mFbo[ 0 ].getSize() ) ) * 3.0f;
	for ( size_t i = 0; i < 2; ++i ) {
		mFbo[ i + 1 ].bindFramebuffer();
		gl::clear();
		
		mShader[ i ].bind();
		mShader[ i ].uniform( "size",	pixel );
		mShader[ i ].uniform( "tex",	0 );
				
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

	mParams->draw();
}

void TracerApp::prepareSettings( Settings* settings )
{
	settings->setFrameRate( 60.0f );
	settings->setResizable( false );
	settings->setWindowSize( 1024, 768 );
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
	mCamera.lookAt( Vec3f( 0.0f, 93.75f, 250.0f ), Vec3f( 0.0f, 250.0f, 0.0f ) );
	
	mDevice = Device::create();
	mDevice->connectEventHandler( [ & ]( Leap::Frame frame )
{
	mFrame = frame;
}
 );

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

	mFrameRate	= 0.0f;
	mParams = params::InterfaceGl::create( "Params", Vec2i( 200, 105 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,				"", true );
	mParams->addButton( "Screen shot",	[ & ]() { screenShot(); },	"key=space" );
	mParams->addButton( "Quit",			[ & ]() { quit(); },		"key=q" );

	gl::enable( GL_POLYGON_SMOOTH );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	
	gl::Fbo::Format format;
#if defined( GL_RGBA32F )
	format.setColorInternalFormat( GL_RGBA32F );
#else if defined( GL_RGBA32F_ARB )
	format.setColorInternalFormat( GL_RGBA32F_ARB );
#endif
	format.setMinFilter( GL_LINEAR );
	format.setMagFilter( GL_LINEAR );
	format.setWrap( GL_CLAMP, GL_CLAMP );
	for ( size_t i = 0; i < 3; ++i ) {
		mFbo[ i ]	= gl::Fbo( getWindowWidth(), getWindowHeight(), format );
		mFbo[ i ].bindFramebuffer();
		gl::setViewport( mFbo[ i ].getBounds() );
		gl::clear();
		mFbo[ i ].unbindFramebuffer();
	}
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );
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
				Vec3f v = randVec3f() * 0.01f;
				v.x = math<float>::abs( v.x );
				v.y = math<float>::abs( v.y );
				v.z = math<float>::abs( v.z );
				Colorf color( ColorModel::CM_RGB, v );
				Ribbon ribbon( id, color );
				mRibbons[ id ] = ribbon;
			}
			float width = math<float>::abs( pointable.tipVelocity().y ) * 0.0025f;
			width		= math<float>::max( width, 5.0f );
			mRibbons[ id ].addPoint( LeapMotion::toVec3f( pointable.tipPosition() ), width );
		}
	}

	// Update ribbons
	for ( RibbonMap::iterator iter = mRibbons.begin(); iter != mRibbons.end(); ++iter ) {
		iter->second.update();
	}
}

CINDER_APP_BASIC( TracerApp, RendererGl )
