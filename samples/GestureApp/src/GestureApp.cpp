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

/* 
 * The Cinder-LeapSdk block does not wrap Leap's
 * Gesture API.  Instead, it delivers native Leap
 * Gestures in each LeapSdk::Frame. The properties 
 * of each Leap::Gesture may be converted to Cinder-
 * friendly types by using the LeapSdk::fromLeap*
 * methods, as demonstrated here.
 */
class GestureApp : public ci::app::AppBasic
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
	ci::Vec2f				warp( const ci::Vec3f& v );
	
	// Camera
	ci::CameraPersp			mCamera;

	// Params
	float					mFrameRate;
	bool					mFullScreen;
	ci::params::InterfaceGl	mParams;
	
	void					drawDottedRect( const ci::Vec2f& center, const ci::Vec2f& size );

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

static const float	kDotRadius	= 5.0f;

// Render
void GestureApp::draw()
{
	// Clear window
	gl::setViewport( getWindowBounds() );
	gl::clear( Colorf( 0.0f, 0.1f, 0.2f ) );
	gl::setMatricesWindow( getWindowSize() );
	gl::color( Colorf::white() );
		
	// Iterate through gestures
	const vector<Leap::Gesture>& gestures = mFrame.getGestures();
	for ( vector<Leap::Gesture>::const_iterator iter = gestures.begin(); iter != gestures.end(); ++iter ) {
		Gesture::Type type = iter->type();
		if ( type == Gesture::Type::TYPE_CIRCLE ) {
				
			// Cast to circle gesture and read data
			const Leap::CircleGesture& gesture = (Leap::CircleGesture)*iter;
			Vec2f center	= warp( fromLeapVector( gesture.center() ) );
			float progress	= gesture.progress() * (float)M_PI;
			float radius	= gesture.radius();
			
			// Draw circle
			uint32_t resolution	= 32;
			float delta			= ( (float)M_PI * 2.0f ) / (float)resolution;
			for ( float theta = 0.0f; theta <= progress; theta += delta ) {
				float t			= theta - (float)M_PI * 0.5f;
				float x			= math<float>::cos( t );
				float y			= math<float>::sin( t );
				
				Vec2f pos		= center + Vec2f( x, y ) * radius;
				gl::drawSolidCircle( pos, kDotRadius, 32 );
			}
		} else if ( type == Gesture::Type::TYPE_KEY_TAP ) {
			
			// Cast to circle gesture and read data
			const Leap::KeyTapGesture& gesture = (Leap::KeyTapGesture)*iter;
			Vec2f center = warp( fromLeapVector( gesture.position() ) );
			
			// Draw square where key press happened
			Vec2f size( 30.0f, 30.0f );
			drawDottedRect( center, size );
			
		} else if ( type == Gesture::Type::TYPE_SCREEN_TAP ) {
			
			// Draw big square on center of screen
			Vec2f center = getWindowCenter();
			Vec2f size( 300.0f, 300.0f );
			drawDottedRect( center, size );
			
			
			console() << "screen tap" << endl;
		} else if ( type == Gesture::Type::TYPE_SWIPE ) {
			
			// Cast to swipe gesture and read data
			const Leap::SwipeGesture& gesture = (Leap::SwipeGesture)*iter;
			ci::Vec2f a	= warp( fromLeapVector( gesture.startPosition() ) );
			ci::Vec2f b	= warp( fromLeapVector( gesture.position() ) );
						
			// Set draw direction
			float spacing = kDotRadius * 3.0f;
			float direction = 1.0f;
			if ( b.x < a.x ) {
				direction *= -1.0f;
				swap( a, b );
			}
			
			// Draw swipe line
			Vec2f pos = a;
			while ( pos.x <= b.x ) {
				pos.x += spacing;
				gl::drawSolidCircle( pos, kDotRadius, 32 );
			}
			
			// Draw arrow head
			if ( direction > 0.0f ) {
				pos		= b;
				spacing	*= -1.0f;
			} else {
				pos		= a;
				pos.x	+= spacing;
			}
			pos.y		= a.y;
			pos.x		+= spacing;
			gl::drawSolidCircle( pos + Vec2f( 0.0f, spacing ), kDotRadius, 32 );
			gl::drawSolidCircle( pos + Vec2f( 0.0f, spacing * -1.0f ), kDotRadius, 32 );
			pos.x		+= spacing;
			gl::drawSolidCircle( pos + Vec2f( 0.0f, spacing * 2.0f ), kDotRadius, 32 );
			gl::drawSolidCircle( pos + Vec2f( 0.0f, spacing * -2.0f ), kDotRadius, 32 );
			
		}
	}
	
	// Draw the interface
	mParams.draw();
}

// Draw dotted rectangle
void GestureApp::drawDottedRect( const Vec2f &center, const Vec2f &size )
{
	float spacing = kDotRadius * 3.0f;
	int32_t resolution = 32;
	Rectf rect( center - size, center + size );
	Vec2f pos = rect.getUpperLeft();
	while ( pos.x < rect.getX2() ) {
		gl::drawSolidCircle( pos, kDotRadius, resolution );
		pos.x += spacing;
	}
	while ( pos.y < rect.getY2() ) {
		gl::drawSolidCircle( pos, kDotRadius, resolution );
		pos.y += spacing;
	}
	while ( pos.x > rect.getX1() ) {
		gl::drawSolidCircle( pos, kDotRadius, resolution );
		pos.x -= spacing;
	}
	while ( pos.y > rect.getY1() ) {
		gl::drawSolidCircle( pos, kDotRadius, resolution );
		pos.y -= spacing;
	}
}

// Called when Leap frame data is ready
void GestureApp::onFrame( Frame frame )
{
	mFrame = frame;
}

// Prepare window
void GestureApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1024, 768 );
	settings->setFrameRate( 60.0f );
}

// Take screen shot
void GestureApp::screenShot()
{
#if defined( CINDER_MSW )
	fs::path path = getAppPath();
#else
	fs::path path = getAppPath().parent_path();
#endif
	writeImage( path / fs::path( "frame" + toString( getElapsedFrames() ) + ".png" ), copyWindowSurface() );
}

// Set up
void GestureApp::setup()
{
	// Set up OpenGL
	gl::enable( GL_POLYGON_SMOOTH );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	
	// Set up camera
	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 45.0f, 0.01f, 10.0f );
	mCamera.lookAt( Vec3f( 0.0f, 0.0f, -300.0f ), Vec3f::zero() );
	
	// Start device
	mLeap 		= Device::create();
	mCallbackId = mLeap->addCallback( &GestureApp::onFrame, this );

	// Enable all gesture types
	mLeap->enableGesture( Gesture::Type::TYPE_CIRCLE );
	mLeap->enableGesture( Gesture::Type::TYPE_KEY_TAP );
	mLeap->enableGesture( Gesture::Type::TYPE_SCREEN_TAP );
	mLeap->enableGesture( Gesture::Type::TYPE_SWIPE );
	
	// Params
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	mParams = params::InterfaceGl( "Params", Vec2i( 200, 105 ) );
	mParams.addParam( "Frame rate",		&mFrameRate,						"", true );
	mParams.addParam( "Full screen",	&mFullScreen,						"key=f"		);
	mParams.addButton( "Screen shot",	bind( &GestureApp::screenShot, this ), "key=space" );
	mParams.addButton( "Quit",			bind( &GestureApp::quit, this ),		"key=q" );
}

// Quit
void GestureApp::shutdown()
{
	mLeap->removeCallback( mCallbackId );
}

// Runs update logic
void GestureApp::update()
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
}

// Maps Leap vector to the screen in pixels
Vec2f GestureApp::warp( const Vec3f& v )
{
	Vec3f result = Vec3f::zero();
	if ( mLeap ) {
		const ScreenMap& screens = mLeap->getScreens();
		if ( !screens.empty() ) {
			const Screen& screen = screens.begin()->second;
			
			result		= v - screen.getBottomLeft();
			// Divide by camera resolution for good approximation
			result		/= Vec3f( 320.0f, 240.0f, 1.0f );
			result		*= Vec3f( Vec2f( getWindowSize() ), 0.0f );
			result.y	= (float)getWindowHeight() - result.y;
		}
	}
	return result.xy();
}

// Run application
CINDER_APP_BASIC( GestureApp, RendererGl )
