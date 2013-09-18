#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

#include "Cinder-LeapMotion.h"

class _TBOX_PREFIX_App : public ci::app::AppNative 
{
 public:
	void 					draw();
	void 					setup();
 private:	
	LeapMotion::Frame		mFrame;
	LeapMotion::DeviceRef	mLeap;
	void 					onFrame( Leap::Frame frame );

	ci::CameraPersp			mCamera;
};

using namespace ci;
using namespace ci::app;
using namespace std;

void _TBOX_PREFIX_App::draw() 
{
	// Clear window
	gl::setViewport( getWindowBounds() );
	gl::clear( Colorf::black() );
	gl::setMatrices( mCamera );
	
	// Iterate through hands
	const Leap::HandList& hands = mFrame.hands();
	for ( Leap::HandList::const_iterator handIter = hands.begin(); handIter != hands.end(); ++handIter ) {

		// Pointables
		const Leap::PointableList& pointables = hand.pointables();
		for ( Leap::PointableList::const_iterator pointIter = pointables.begin(); pointIter != pointables.end(); ++pointIter ) {
			const Leap::Pointable& pointable = *pointIter;

			Vec3f dir		= LeapMotion::toVec3f( pointable.direction() );
			float length	= pointable.length();
			Vec3f tipPos	= LeapMotion::toVec3f( pointable.tipPosition() );
			Vec3f basePos	= tipPos + dir * length;
			
			gl::drawColorCube( tipPos, Vec3f( 20, 20, 20 ) );
			gl::color( ColorAf::gray( 0.8f ) );
			gl::drawLine( basePos, tipPos );
		}
	}
}

void _TBOX_PREFIX_App::onFrame( Leap::Frame frame )
{
	mFrame = frame;
}

void _TBOX_PREFIX_App::setup()
{	 
	// Set up OpenGL
	gl::enableAlphaBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();
	
	// Set up camera
	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f, 0.01f, 1000.0f );
	mCamera.lookAt( Vec3f( 0.0f, 125.0f, 500.0f ), Vec3f( 0.0f, 250.0f, 0.0f ) );
	
	// Start device
	mLeap = LeapMotion::Device::create();
	mLeap->connectEventHandler( &_TBOX_PREFIX_App::onFrame, this );
}

CINDER_APP_NATIVE( _TBOX_PREFIX_App, RendererGl )
