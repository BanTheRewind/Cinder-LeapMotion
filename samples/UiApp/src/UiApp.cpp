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
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "Cinder-LeapSdk.h"

class UiApp : public ci::app::AppBasic
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
	LeapSdk::HandMap		mHands;
	LeapSdk::DeviceRef		mLeap;
	void 					onFrame( LeapSdk::Frame frame );

	// Camera
	ci::CameraPersp			mCamera;
	ci::Vec3f				mOffset;
	
	// Cursor
	enum
	{
		GRAB, HAND, TOUCH, NONE
	} typedef CursorType;
	ci::Vec3f				mCursorPosition;
	ci::Vec3f				mCursorPositionTarget;
	CursorType				mCursorType;
	ci::gl::Texture			mTexture[ 3 ];
	ci::Vec3f				warp( const ci::Vec3f& v );
	
	// UI
	ci::gl::Texture			mButton[ 2 ];
	ci::Vec3f				mButtonPosition[ 3 ];
	bool					mButtonState[ 3 ];
	ci::gl::Texture			mSlider;
	ci::Vec3f				mSliderPosition;
	ci::gl::Texture			mTrack;
	ci::Vec3f				mTrackPosition;
	
	// Params
	float					mFrameRate;
	bool					mFullScreen;
	ci::params::InterfaceGl	mParams;

	// Save screen shot
	void					screenShot();
};

#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "Resources.h"

// Imports
using namespace ci;
using namespace ci::app;
using namespace LeapSdk;
using namespace std;

// Render
void UiApp::draw()
{
	// Clear window
	gl::setViewport( getWindowBounds() );
	gl::clear( Colorf::white() );
	gl::setMatrices( mCamera );
	
	// Make PNG backgrounds transparent
	gl::enableAlphaBlending();
	gl::disableDepthRead();
	gl::disableDepthWrite();
	
	// Master offset
	gl::pushMatrices();
	gl::translate( mOffset );
	
	// Draw buttons
	for ( size_t i = 0; i < 3; ++i ) {
		bool pressed = mButtonState[ i ];
		gl::pushMatrices();
		gl::translate( mButtonPosition[ i ] );
		gl::draw( mButton[ pressed ? 1 : 0 ] );
		gl::popMatrices();
	}
	
	// Draw slider
	gl::pushMatrices();
	gl::translate( mTrackPosition );
	gl::draw( mTrack );
	gl::popMatrices();
	gl::pushMatrices();
	gl::translate( mSliderPosition );
	gl::draw( mSlider );
	gl::popMatrices();
	
	// Draw cursor
	if ( mCursorType != CursorType::NONE ) {
		gl::color( ColorAf::white() );
		gl::pushMatrices();
		gl::translate( mCursorPosition );
		gl::draw( mTexture[ (size_t)mCursorType ] );
		gl::popMatrices();
	}
	
	gl::popMatrices();
	
	// Draw the interface
	mParams.draw();
}

// Called when Leap frame data is ready
void UiApp::onFrame( Frame frame )
{
	mHands = frame.getHands();
}

// Prepare window
void UiApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1024, 768 );
	settings->setFrameRate( 60.0f );
}

// Take screen shot
void UiApp::screenShot()
{
#if defined( CINDER_MSW )
	fs::path path = getAppPath();
#else
	fs::path path = getAppPath().parent_path();
#endif
	writeImage( path / fs::path( "frame" + toString( getElapsedFrames() ) + ".png" ), copyWindowSurface() );
}

// Set up
void UiApp::setup()
{
	glShadeModel( GL_FLAT );
	
	// Set up camera
	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 45.0f, 0.01f, 1000.0f );
	mOffset = Vec3f( 240.0f, -480.0f, 0.0f );
	
	// Start device
	mLeap = Device::create();
	mLeap->addCallback( &UiApp::onFrame, this );
	
	// Load cursor textures
	for ( size_t i = 0; i < 3; ++i ) {
		switch ( (CursorType)i ) {
			case CursorType::GRAB:
				mTexture[ i ] = gl::Texture( loadImage( loadResource( RES_TEX_GRAB ) ) );
				break;
			case CursorType::HAND:
				mTexture[ i ] = gl::Texture( loadImage( loadResource( RES_TEX_HAND ) ) );
				break;
			case CursorType::TOUCH:
				mTexture[ i ] = gl::Texture( loadImage( loadResource( RES_TEX_TOUCH ) ) );
				break;
			case NONE:
				break;
		}
		mTexture[ i ].setFlipped( true );
		mTexture[ i ].setMagFilter( GL_NEAREST );
		mTexture[ i ].setMinFilter( GL_NEAREST );
	}
	
	// Initialize cursor
	mCursorType				= CursorType::NONE;
	mCursorPosition			= Vec3f::zero();
	mCursorPositionTarget	= Vec3f::zero();
	
	// Load UI textures
	mButton[ 0 ]	= gl::Texture( loadImage( loadResource( RES_TEX_BUTTON_OFF ) ) );
	mButton[ 1 ]	= gl::Texture( loadImage( loadResource( RES_TEX_BUTTON_ON ) ) );
	mSlider			= gl::Texture( loadImage( loadResource( RES_TEX_SLIDER ) ) );
	mTrack			= gl::Texture( loadImage( loadResource( RES_TEX_TRACK ) ) );

	// Flip textures
	mButton[ 0 ].setFlipped( true );
	mButton[ 1 ].setFlipped( true );
	mSlider.setFlipped( true );
	mTrack.setFlipped( true );
	
	// Disable anti-aliasing
	mButton[ 0 ].setMagFilter( GL_NEAREST );
	mButton[ 0 ].setMinFilter( GL_NEAREST );
	mButton[ 1 ].setMagFilter( GL_NEAREST );
	mButton[ 1 ].setMinFilter( GL_NEAREST );
	mSlider.setMagFilter( GL_NEAREST );
	mSlider.setMinFilter( GL_NEAREST );
	mTrack.setMagFilter( GL_NEAREST );
	mTrack.setMinFilter( GL_NEAREST );
	
	// Initialize buttons
	Vec3f position( -120.0f, 950.0f, 0.0f );
	for ( size_t i = 0; i < 3; ++i, position.x += 320.0f ) {
		mButtonPosition[ i ]	= position;
		mButtonState[ i ]		= false;
	}
	
	// Initialize sliider
	mTrackPosition		= Vec3f( 0.0f, 700.0f, 0.0f );
	mSliderPosition		= mTrackPosition;
	mSliderPosition.y	-= 45.0f;
	
	// Params
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	mParams = params::InterfaceGl( "Params", Vec2i( 200, 105 ) );
	mParams.addParam( "Frame rate",		&mFrameRate,						"", true );
	mParams.addParam( "Full screen",	&mFullScreen,						"key=f"		);
	mParams.addButton( "Screen shot",	bind( &UiApp::screenShot, this ),	"key=space" );
	mParams.addButton( "Quit",			bind( &UiApp::quit, this ),			"key=q" );
}

// Quit
void UiApp::shutdown()
{
	mLeap->removeCallback( mCallbackId );
	mHands.clear();
}

// Runs update logic
void UiApp::update()
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
	
	// Interact with first hand
	if ( mHands.empty() ) {
		mCursorType		= CursorType::NONE;
	} else {
		const Hand& hand = mHands.begin()->second;
		
		// Update cursor position
		mCursorPositionTarget	= warp( hand.getPosition() );
		if ( mCursorType == CursorType::NONE ) {
			mCursorPosition = mCursorPositionTarget;
		}
		
		// Choose cursor type based on number of exposed fingers
		switch ( hand.getFingers().size() ) {
			case 0:
				mCursorType	= CursorType::GRAB;
				break;
			case 1:
				mCursorType	= CursorType::TOUCH;
				break;
			default:
				mCursorType	= CursorType::HAND;
				break;
		}
	}
	
	// Smooth cursor animation
	mCursorPosition = mCursorPosition.lerp( 0.21f, mCursorPositionTarget );
	mCursorPosition.z = 0.0f;
	
	// Hit buttons
	for ( size_t i = 0; i < 3; ++i ) {
		mButtonState[ i ] = false;
		if ( mCursorType == CursorType::TOUCH &&
			mCursorPosition.distance( mButtonPosition[ i ] ) < (float)mButton[ 0 ].getSize().length() * 0.5f ) {
			mButtonState[ i ] = true;
		}
	}
	
	// Slider
	if ( mCursorType == CursorType::GRAB &&
		math<float>::abs( mCursorPosition.x - mSliderPosition.x ) < (float)mSlider.getWidth() * 0.5f &&
		math<float>::abs( mCursorPosition.y - mSliderPosition.y ) < (float)mSlider.getHeight() * 0.5f ) {
		float x1			= mTrackPosition.x;
		float x2			= mTrackPosition.x + (float)( mTrack.getWidth() - mSlider.getWidth() );
		mSliderPosition.x	= math<float>::clamp( mCursorPosition.x, x1, x2 );
	}
}

Vec3f UiApp::warp( const Vec3f& v )
{
	if ( mLeap ) {
		const ScreenMap& screens = mLeap->getScreens();
		if ( !screens.empty() ) {
			const Screen& screen = screens.begin()->second;
			console() << screen.getDescription() << endl;
			console() << "BL: " << screen.getBottomLeft() << endl;
			console() << "HA: " << screen.getHorizontalAxis() << endl;
			console() << "VA: " << screen.getVerticalAxis() << endl;
			console() << "N: " << screen.getNormal() << endl;
		}
	}
	return v;
}

// Run application
CINDER_APP_BASIC( UiApp, RendererGl( RendererGl::AA_NONE ) )
