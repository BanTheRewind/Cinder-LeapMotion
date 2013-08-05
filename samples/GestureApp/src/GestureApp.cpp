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

#include "Cinder-LeapSdk.h"

class GestureApp : public ci::app::AppBasic
{
public:
	void					draw();
	void					prepareSettings( ci::app::AppBasic::Settings* settings );
	void					resize();
	void					setup();
	void					update();
private:
	// Leap
	Leap::Frame				mFrame;
	LeapSdk::DeviceRef		mLeap;
	void 					onFrame( Leap::Frame frame );
	ci::Vec2f				warpPointable( const Leap::Pointable& p );
	ci::Vec2f				warpVector( const Leap::Vector& v );
	
	// UI
	float					mBackgroundBrightness;
	ci::Colorf				mBackgroundColor;
	int32_t					mCircleResolution;
	float					mDialBrightness;
	ci::Vec2f				mDialPosition;
	float					mDialRadius;
	float					mDialSpeed;
	float					mDialValue;
	float					mDialValueDest;
	float					mDotRadius;
	float					mDotSpacing;
	float					mFadeSpeed;
	float					mKeySpacing;
	ci::Rectf				mKeyRect;
	float					mKeySize;
	ci::Vec2f				mOffset;
	float					mPointableRadius;
	float					mSwipeBrightness;
	float					mSwipePos;
	float					mSwipePosDest;
	float					mSwipePosSpeed;
	ci::Rectf				mSwipeRect;
	float					mSwipeStep;
	
	// A key to press
	struct Key
	{
		Key( const ci::Rectf& bounds = ci::Rectf() )
		: mBounds( bounds ), mBrightness( 0.0f )
		{
		}
		ci::Rectf			mBounds;
		float				mBrightness;
	};
	std::vector<Key>		mKeys;

	// Rendering
	void					drawDottedCircle( const ci::Vec2f& center, float radius,
											 float dotRadius, int32_t resolution,
											 float progress = 1.0f );
	void					drawDottedRect( const ci::Vec2f& center, const ci::Vec2f& size );
	void					drawGestures();
	void					drawPointables();
	void					drawUi();
	
	// Params
	float					mFrameRate;
	bool					mFullScreen;
	ci::params::InterfaceGl	mParams;
	
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

// Render
void GestureApp::draw()
{
	// Clear window
	gl::setViewport( getWindowBounds() );
	gl::clear( mBackgroundColor + Colorf::gray( mBackgroundBrightness ) );
	gl::setMatricesWindow( getWindowSize() );
	gl::enableAlphaBlending();
	gl::color( Colorf::white() );
	
	// Draw everything
	gl::pushMatrices();
	gl::translate( mOffset );
	
	drawUi();
	drawGestures();
	drawPointables();
	
	gl::popMatrices();
	
	// Draw the interface
	mParams.draw();
}

// Draw dotted circle
void GestureApp::drawDottedCircle( const Vec2f& center, float radius, float dotRadius,
								  int32_t resolution, float progress )
{
	float twoPi		= (float)M_PI * 2.0f;
	progress		*= twoPi;
	float delta		= twoPi / (float)resolution;
	for ( float theta = 0.0f; theta <= progress; theta += delta ) {
		float t		= theta - (float)M_PI * 0.5f;
		float x		= math<float>::cos( t );
		float y		= math<float>::sin( t );
		
		Vec2f pos	= center + Vec2f( x, y ) * radius;
		gl::drawSolidCircle( pos, dotRadius, 32 );
	}
}

// Draw dotted rectangle
void GestureApp::drawDottedRect( const Vec2f& center, const Vec2f &size )
{
	Rectf rect( center - size, center + size );
	Vec2f pos = rect.getUpperLeft();
	while ( pos.x < rect.getX2() ) {
		gl::drawSolidCircle( pos, mDotRadius, mCircleResolution );
		pos.x += mDotSpacing;
	}
	while ( pos.y < rect.getY2() ) {
		gl::drawSolidCircle( pos, mDotRadius, mCircleResolution );
		pos.y += mDotSpacing;
	}
	while ( pos.x > rect.getX1() ) {
		gl::drawSolidCircle( pos, mDotRadius, mCircleResolution );
		pos.x -= mDotSpacing;
	}
	while ( pos.y > rect.getY1() ) {
		gl::drawSolidCircle( pos, mDotRadius, mCircleResolution );
		pos.y -= mDotSpacing;
	}
}

// Draw active gestures with dotted lines
void GestureApp::drawGestures()
{
	gl::color( ColorAf::white() );
	
	// Iterate through gestures
	const Leap::GestureList& gestures = mFrame.gestures();
	for ( Leap::GestureList::const_iterator iter = gestures.begin(); iter != gestures.end(); ++iter ) {
		const Leap::Gesture& gesture	= *iter;
		Leap::Gesture::Type type		= gesture.type();
		if ( type == Leap::Gesture::Type::TYPE_CIRCLE ) {
			
			// Cast to circle gesture and read data
			const Leap::CircleGesture& gesture = (Leap::CircleGesture)*iter;
						
			Vec2f pos	= warpVector( gesture.center() );
			float progress	= gesture.progress();
			float radius	= gesture.radius() * 2.0f; // Don't ask, it works
			
			drawDottedCircle( pos, radius, mDotRadius, mCircleResolution, progress );
			
		} else if ( type == Leap::Gesture::Type::TYPE_KEY_TAP ) {
			
			// Cast to circle gesture and read data
			const Leap::KeyTapGesture& gesture = (Leap::KeyTapGesture)*iter;
			Vec2f center = warpVector( gesture.position() );
			
			// Draw square where key press happened
			Vec2f size( 30.0f, 30.0f );
			drawDottedRect( center, size );
			
		} else if ( type == Leap::Gesture::Type::TYPE_SCREEN_TAP ) {
			
			// Draw big square on center of screen
			Vec2f center = getWindowCenter();
			Vec2f size( 300.0f, 300.0f );
			drawDottedRect( center, size );
		} else if ( type == Leap::Gesture::Type::TYPE_SWIPE ) {
			
			// Cast to swipe gesture and read data
			const Leap::SwipeGesture& gesture = (Leap::SwipeGesture)*iter;
			ci::Vec2f a	= warpVector( gesture.startPosition() );
			ci::Vec2f b	= warpVector( gesture.position() );
			
			// Set draw direction
			float spacing = mDotRadius * 3.0f;
			float direction = 1.0f;
			if ( b.x < a.x ) {
				direction *= -1.0f;
				swap( a, b );
			}
			
			// Draw swipe line
			Vec2f pos = a;
			while ( pos.x <= b.x ) {
				pos.x += spacing;
				gl::drawSolidCircle( pos, mDotRadius, 32 );
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
			gl::drawSolidCircle( pos + Vec2f( 0.0f, spacing ), mDotRadius, 32 );
			gl::drawSolidCircle( pos + Vec2f( 0.0f, spacing * -1.0f ), mDotRadius, 32 );
			pos.x		+= spacing;
			gl::drawSolidCircle( pos + Vec2f( 0.0f, spacing * 2.0f ), mDotRadius, 32 );
			gl::drawSolidCircle( pos + Vec2f( 0.0f, spacing * -2.0f ), mDotRadius, 32 );
		}
	}
}

// Draw pointables calibrated to the screen
void GestureApp::drawPointables()
{
	gl::color( ColorAf::white() );
	const Leap::HandList& hands = mFrame.hands();
	for ( Leap::HandList::const_iterator handIter = hands.begin(); handIter != hands.end(); ++handIter ) {
		const Leap::Hand& hand = *handIter;
		const Leap::PointableList& pointables = hand.pointables();
		for ( Leap::PointableList::const_iterator pointIter = pointables.begin(); pointIter != pointables.end(); ++pointIter ) {
			const Leap::Pointable& pointable = *pointIter;
			
			Vec2f pos( warpPointable( pointable ) );
			drawDottedCircle( pos, mPointableRadius, mDotRadius * 0.5f, mCircleResolution / 2 );
		}
	}
}

// Draw interface
void GestureApp::drawUi()
{
	// Draw dial
	gl::color( ColorAf( Colorf::white(), 0.2f + mDialBrightness ) );
	gl::drawSolidCircle( mDialPosition, mDialRadius, mCircleResolution * 2 );
	gl::color( mBackgroundColor );
	float angle = mDialValue * (float)M_PI * 2.0f;
	Vec2f pos( math<float>::cos( angle ), math<float>::sin( angle ) );
	pos *= mDialRadius - mDotSpacing;
	gl::drawSolidCircle( mDialPosition + pos, mDotRadius, mCircleResolution );
	
	// Draw swipe
	float x = mSwipeRect.x1 + mSwipePos * mSwipeRect.getWidth();
	Rectf a( mSwipeRect.x1, mSwipeRect.y1, x, mSwipeRect.y2 );
	Rectf b( x, mSwipeRect.y1, mSwipeRect.x2, mSwipeRect.y2 );
	gl::color( ColorAf( Colorf::gray( 0.5f ), 0.2f + mSwipeBrightness ) );
	gl::drawSolidRect( a );
	gl::color( ColorAf( Colorf::white(), 0.2f + mSwipeBrightness ) );
	gl::drawSolidRect( b );
	
	// Draw keys
	for ( vector<Key>::iterator iter = mKeys.begin(); iter != mKeys.end(); ++iter ) {
		gl::color( ColorAf( Colorf::white(), 0.2f + iter->mBrightness ) );
		gl::drawSolidRect( iter->mBounds );
	}
}

// Called when Leap frame data is ready
void GestureApp::onFrame( Leap::Frame frame )
{
	mFrame = frame;
}

// Prepare window
void GestureApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1024, 768 );
	settings->setFrameRate( 60.0f );
}

// Handles window resize
void GestureApp::resize()
{
	mOffset = getWindowCenter() - Vec2f::one() * 320.0f;
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
	
	// UI
	mBackgroundBrightness	= 0.0f;
	mBackgroundColor		= Colorf( 0.0f, 0.1f, 0.2f );
	mCircleResolution		= 32;
	mDialBrightness			= 0.0f;
	mDialPosition			= Vec2f( 155.0f, 230.0f );
	mDialRadius				= 120.0f;
	mDialSpeed				= 0.21f;
	mDialValue				= 0.0f;
	mDialValueDest			= mDialValue;
	mDotRadius				= 3.0f;
	mDotSpacing				= mDotRadius * 3.0f;
	mFadeSpeed				= 0.95f;
	mKeySpacing				= 25.0f;
	mKeyRect				= Rectf( mKeySpacing, 360.0f + mKeySpacing, 600.0f, 600.0f );
	mKeySize				= 60.0f;
	mPointableRadius		= 15.0f;
	mSwipeBrightness		= 0.0f;
	mSwipePos				= 0.0f;
	mSwipePosDest			= mSwipePos;
	mSwipePosSpeed			= 0.33f;
	mSwipeRect				= Rectf( 310.0f, 100.0f, 595.0f, 360.0f );
	mSwipeStep				= 0.033f;
	
	// Sets master offset
	resize();
	
	// Lay out keys
	float spacing = mKeySize + mKeySpacing;
	for ( float y = mKeyRect.y1; y < mKeyRect.y2; y += spacing ) {
		for ( float x = mKeyRect.x1; x < mKeyRect.x2; x += spacing ) {
			Rectf bounds( x, y, x + mKeySize, y + mKeySize );
			Key key( bounds );
			mKeys.push_back( key );
		}
	}
	
	// Start device
	mLeap 		= Device::create();
	mLeap->connectEventHandler( &GestureApp::onFrame, this );

	// Enable all gesture types
	Leap::Controller* controller = mLeap->getController();
	controller->enableGesture( Leap::Gesture::Type::TYPE_CIRCLE );
	controller->enableGesture( Leap::Gesture::Type::TYPE_KEY_TAP );
	controller->enableGesture( Leap::Gesture::Type::TYPE_SCREEN_TAP );
	controller->enableGesture( Leap::Gesture::Type::TYPE_SWIPE );
	
	// Write gesture config to console
	Leap::Config config = controller->config();
	console() << "Gesture.Circle.MinRadius: " << config.getFloat( "Gesture.Circle.MinRadius" ) << endl;
	console() << "Gesture.Circle.MinArc: " << config.getFloat( "Gesture.Circle.MinArc" ) << endl;
	console() << "Gesture.Swipe.MinLength: " << config.getFloat( "Gesture.Swipe.MinLength" ) << endl;
	console() << "Gesture.Swipe.MinVelocity: " << config.getFloat( "Gesture.Swipe.MinVelocity" ) << endl;
	console() << "Gesture.KeyTap.MinDownVelocity: " << config.getFloat( "Gesture.KeyTap.MinDownVelocity" ) << endl;
	console() << "Gesture.KeyTap.HistorySeconds: " << config.getFloat( "Gesture.KeyTap.HistorySeconds" ) << endl;
	console() << "Gesture.KeyTap.MinDistance: " << config.getFloat( "Gesture.KeyTap.MinDistance" ) << endl;
	console() << "Gesture.ScreenTap.MinForwardVelocity: " << config.getFloat( "Gesture.ScreenTap.MinForwardVelocity" ) << endl;
	console() << "Gesture.ScreenTap.HistorySeconds: " << config.getFloat( "Gesture.ScreenTap.HistorySeconds" ) << endl;
	console() << "Gesture.ScreenTap.MinDistance: " << config.getFloat( "Gesture.ScreenTap.MinDistance" ) << endl;

	// Update config to make gestures easier
	config.setFloat( "Gesture.Circle.MinRadius", 2.5f );
	config.setFloat( "Gesture.Circle.MinArc", 3.0f );
	config.setFloat( "Gesture.Swipe.MinLength", 75.0f );
	config.setFloat( "Gesture.Swipe.MinVelocity", 500.0f );
	config.setFloat( "Gesture.KeyTap.MinDownVelocity", 25.0f );
	
	// Allows app to run in background
	controller->setPolicyFlags( Leap::Controller::PolicyFlag::POLICY_BACKGROUND_FRAMES );
	
	// Params
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	mParams = params::InterfaceGl( "Params", Vec2i( 200, 105 ) );
	mParams.addParam( "Frame rate",		&mFrameRate,							"", true	);
	mParams.addParam( "Full screen",	&mFullScreen,							"key=f"		);
	mParams.addButton( "Screen shot",	bind( &GestureApp::screenShot, this ), "key=space"	);
	mParams.addButton( "Quit",			bind( &GestureApp::quit, this ),		"key=q"		);
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
	
	const Leap::GestureList& gestures = mFrame.gestures();
	for ( Leap::GestureList::const_iterator iter = gestures.begin(); iter != gestures.end(); ++iter ) {
		const Leap::Gesture& gesture	= *iter;
		Leap::Gesture::Type type		= gesture.type();
		if ( type == Leap::Gesture::Type::TYPE_CIRCLE ) {
			
			// Cast to circle gesture
			const Leap::CircleGesture& gesture = (Leap::CircleGesture)*iter;
			
			// Control dial
			mDialBrightness	= 1.0f;
			mDialValueDest	= gesture.progress();
			
		} else if ( type == Leap::Gesture::Type::TYPE_KEY_TAP ) {
			
			// Cast to circle gesture and read data
			const Leap::KeyTapGesture& gesture = (Leap::KeyTapGesture)*iter;
			Vec2f center	= warpVector( gesture.position() );
			center			-= mOffset;
			
			// Press key
			for ( vector<Key>::iterator keyIter = mKeys.begin(); keyIter != mKeys.end(); ++keyIter ) {
				if ( keyIter->mBounds.contains( center ) ) {
					keyIter->mBrightness = 1.0f;
					break;
				}
			}
			
		} else if ( type == Leap::Gesture::Type::TYPE_SCREEN_TAP ) {
			
			// Turn background white for screen tap
			mBackgroundBrightness = 1.0f;
			
		} else if ( type == Leap::Gesture::Type::TYPE_SWIPE ) {
			
			// Cast to swipe gesture and read data
			const Leap::SwipeGesture& swipeGesture = (Leap::SwipeGesture)gesture;
			ci::Vec2f a	= warpVector( swipeGesture.startPosition() );
			ci::Vec2f b	= warpVector( swipeGesture.position() );
			
			// Update swipe position
			mSwipeBrightness	= 1.0f;
			if ( gesture.state() == Leap::Gesture::State::STATE_STOP ) {
				mSwipePosDest	= b.x < a.x ? 0.0f : 1.0f;
			} else {
				float step		= mSwipeStep;
				mSwipePosDest	+= b.x < a.x ? -step : step;
			}
			mSwipePosDest		= math<float>::clamp( mSwipePosDest, 0.0f, 1.0f );
		}
	}
	
	// UI animation
	mDialValue				= lerp( mDialValue, mDialValueDest, mDialSpeed );
	mSwipePos				= lerp( mSwipePos, mSwipePosDest, mSwipePosSpeed );
	mBackgroundBrightness	*= mFadeSpeed;
	mDialBrightness			*= mFadeSpeed;
	mSwipeBrightness		*= mFadeSpeed;
	for ( vector<Key>::iterator iter = mKeys.begin(); iter != mKeys.end(); ++iter ) {
		iter->mBrightness *= mFadeSpeed;
	}
}

// Maps pointable's ray to the screen in pixels
Vec2f GestureApp::warpPointable( const Leap::Pointable& p )
{
	Vec3f result	= Vec3f::zero();
	if ( mLeap ) {
		const Leap::Screen& screen = mLeap->getController()->calibratedScreens().closestScreenHit( p );
		
		result		= LeapSdk::toVec3f( screen.intersect( p, true, 1.0f ) );
	}
	result			*= Vec3f( Vec2f( getWindowSize() ), 0.0f );
	result.y		= (float)getWindowHeight() - result.y;
	return result.xy();
}

// Maps Leap vector to the screen in pixels
Vec2f GestureApp::warpVector( const Leap::Vector& v )
{
	Vec3f result	= Vec3f::zero();
	if ( mLeap ) {
		const Leap::Screen& screen = mLeap->getController()->calibratedScreens().closestScreen( v );
		
		result		= LeapSdk::toVec3f( screen.project( v, true ) );
	}
	result			*= Vec3f( getWindowSize(), 0.0f );
	result.y		= (float)getWindowHeight() - result.y;
	return result.xy();
}

// Run application
CINDER_APP_BASIC( GestureApp, RendererGl )
