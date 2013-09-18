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
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "Cinder-LeapMotion.h"

class UiApp : public ci::app::AppBasic
{
public:
	void						draw();
	void						prepareSettings( ci::app::AppBasic::Settings* settings );
	void						resize();
	void						setup();
	void						update();
private:
	// Leap
	Leap::Frame					mFrame;
	LeapMotion::DeviceRef		mDevice;
	void 						onFrame( Leap::Frame frame );
	ci::Vec2f					warpPointable( const Leap::Pointable& p );
	ci::Vec2f					warpVector( const Leap::Vector& v );

	// Cursor
	enum
	{
		GRAB, HAND, TOUCH, NONE
	} typedef CursorType;
	ci::Vec2f					mCursorPosition;
	ci::Vec2f					mCursorPositionTarget;
	CursorType					mCursorType;
	ci::Vec2f					mFingerTipPosition;
	ci::gl::Texture				mTexture[ 3 ];
	
	// UI
	ci::gl::Texture				mButton[ 2 ];
	ci::Vec2f					mButtonPosition[ 3 ];
	bool						mButtonState[ 3 ];
	ci::gl::Texture				mSlider;
	ci::Vec2f					mSliderPosition;
	ci::gl::Texture				mTrack;
	ci::Vec2f					mTrackPosition;
	
	// Params
	float						mFrameRate;
	bool						mFullScreen;
	ci::params::InterfaceGlRef	mParams;

	// Save screen shot
	void						screenShot();
};

#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "Resources.h"

// Imports
using namespace ci;
using namespace ci::app;
using namespace LeapMotion;
using namespace std;

// Render
void UiApp::draw()
{
	// Clear window
	gl::setViewport( getWindowBounds() );
	gl::clear( Colorf::white() );
	gl::setMatricesWindow( getWindowSize() );
	gl::color( ColorAf::white() );
	
	// Make PNG backgrounds transparent
	gl::enableAlphaBlending();
	gl::disableDepthRead();
	gl::disableDepthWrite();
	
	// Master offset
	gl::pushMatrices();
	
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
	
	// Draw finger position for pressing buttons
	if ( mCursorType == CursorType::TOUCH )
	{
		gl::color( ColorAf( 1.0f, 0.7f, 0.0f ) );
		gl::drawSolidCircle( mFingerTipPosition, 20.0f );
		gl::drawStrokedCircle( mFingerTipPosition, 40.0f );
	}
	
	// Draw the interface
	mParams->draw();
}

// Called when Leap frame data is ready
void UiApp::onFrame( Leap::Frame frame )
{
	mFrame = frame;
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

// Handles window resize
void UiApp::resize()
{
	// Initialize buttons
	float h = (float)getWindowHeight() * 0.333f;
	float w = (float)getWindowWidth() * 0.25f;
	Vec2f position( w, h );
	position -= Vec2f( mButton[ 0 ].getSize() ) * 0.5f;
	for ( size_t i = 0; i < 3; ++i, position.x += w ) {
		mButtonPosition[ i ]	= position;
		mButtonState[ i ]		= false;
	}

	// Initialize slider
	position = Vec2f( w * 2.0f, h * 2.0f );
	mTrackPosition		= position - Vec2f( mTrack.getSize() ) * 0.5f;
	mSliderPosition		= mTrackPosition;
	mSliderPosition.y	-= 45.0f;
}

// Set up
void UiApp::setup()
{
	glShadeModel( GL_FLAT );
		
	// Start device
	mDevice 		= Device::create();
	mDevice->connectEventHandler( &UiApp::onFrame, this );
	
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
		mTexture[ i ].setMagFilter( GL_NEAREST );
		mTexture[ i ].setMinFilter( GL_NEAREST );
	}
	
	// Initialize cursor
	mCursorType				= CursorType::NONE;
	mCursorPosition			= Vec2f::zero();
	mCursorPositionTarget	= Vec2f::zero();
	mFingerTipPosition		= Vec2i::zero();
	
	// Load UI textures
	mButton[ 0 ]	= gl::Texture( loadImage( loadResource( RES_TEX_BUTTON_OFF ) ) );
	mButton[ 1 ]	= gl::Texture( loadImage( loadResource( RES_TEX_BUTTON_ON ) ) );
	mSlider			= gl::Texture( loadImage( loadResource( RES_TEX_SLIDER ) ) );
	mTrack			= gl::Texture( loadImage( loadResource( RES_TEX_TRACK ) ) );
	
	// Disable anti-aliasing
	mButton[ 0 ].setMagFilter( GL_NEAREST );
	mButton[ 0 ].setMinFilter( GL_NEAREST );
	mButton[ 1 ].setMagFilter( GL_NEAREST );
	mButton[ 1 ].setMinFilter( GL_NEAREST );
	mSlider.setMagFilter( GL_NEAREST );
	mSlider.setMinFilter( GL_NEAREST );
	mTrack.setMagFilter( GL_NEAREST );
	mTrack.setMinFilter( GL_NEAREST );
	
	// Params
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	mParams = params::InterfaceGl::create( "Params", Vec2i( 200, 105 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,						"", true );
	mParams->addParam( "Full screen",	&mFullScreen,						"key=f" );
	mParams->addButton( "Screen shot",	bind( &UiApp::screenShot, this ),	"key=space" );
	mParams->addButton( "Quit",			bind( &UiApp::quit, this ),			"key=q" );
	
	// Run resize to initialize layout
	resize();
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

	// Interact with first hand
	const Leap::HandList& hands = mFrame.hands();
	if ( hands.isEmpty() ) {
		mCursorType		= CursorType::NONE;
	} else {
		const Leap::Hand& hand = *hands.begin();
		
		// Update cursor position
		mCursorPositionTarget	= warpVector( hand.palmPosition() );
		if ( mCursorType == CursorType::NONE ) {
			mCursorPosition = mCursorPositionTarget;
		}
		
		// Choose cursor type based on number of exposed fingers
		switch ( hand.fingers().count() ) {
			case 0:
				mCursorType	= CursorType::GRAB;
				
				// Slider
				if ( mSlider.getBounds().contains( mCursorPosition - mSliderPosition ) ) {
					float x1			= mTrackPosition.x;
					float x2			= mTrackPosition.x + (float)( mTrack.getWidth() - mSlider.getWidth() );
					mSliderPosition.x	= math<float>::clamp( mCursorPosition.x, x1, x2 );
				}
				break;
			case 1:
				mCursorType	= CursorType::TOUCH;
				
				// Buttons
				mFingerTipPosition = warpPointable( *hand.fingers().begin() );
				for ( size_t i = 0; i < 3; ++i ) {
					mButtonState[ i ] = false;
					if ( mButton[ 0 ].getBounds().contains( mFingerTipPosition - mButtonPosition[ i ] ) ) {
						mButtonState[ i ] = true;
					}
				}
				break;
			default:
				mCursorType	= CursorType::HAND;
				break;
		}
	}
	
	// Smooth cursor animation
	mCursorPosition = mCursorPosition.lerp( 0.21f, mCursorPositionTarget );
}

// Maps pointable's ray to the screen in pixels
Vec2f UiApp::warpPointable( const Leap::Pointable& p )
{
	Vec3f result	= Vec3f::zero();
	if ( mDevice ) {
		const Leap::Screen& screen = mDevice->getController()->locatedScreens().closestScreenHit( p );
		
		result		= LeapMotion::toVec3f( screen.intersect( p, true, 1.0f ) );
	}
	result			*= Vec3f( Vec2f( getWindowSize() ), 0.0f );
	result.y		= (float)getWindowHeight() - result.y;
	return result.xy();
}

// Maps Leap vector to the screen in pixels
Vec2f UiApp::warpVector( const Leap::Vector& v )
{
	Vec3f result	= Vec3f::zero();
	if ( mDevice ) {
		const Leap::Screen& screen = mDevice->getController()->locatedScreens().closestScreen( v );
		
		result		= LeapMotion::toVec3f( screen.project( v, true ) );
	}
	result			*= Vec3f( getWindowSize(), 0.0f );
	result.y		= (float)getWindowHeight() - result.y;
	return result.xy();
}

// Run application
CINDER_APP_BASIC( UiApp, RendererGl( RendererGl::AA_NONE ) )
