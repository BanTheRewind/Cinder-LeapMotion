#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"

#include "Cinder-LeapSdk.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class _TBOX_PREFIX_App : public AppNative {
  public:
	void setup();
	void update();
	void draw();
	void shutdown();

  private:	
	uint32_t				mCallbackId;
	LeapSdk::HandMap		mHands;
	LeapSdk::DeviceRef		mLeap;
	void 					onFrame( LeapSdk::Frame frame );

	// Camera
	ci::CameraPersp			mCamera;
};

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
	mLeap 		= LeapSdk::Device::create();
	mCallbackId = mLeap->addCallback( &_TBOX_PREFIX_App::onFrame, this );
}

void _TBOX_PREFIX_App::update()
{	 
	if( mLeap && mLeap->isConnected() ) {		
		mLeap->update();
	}
}

void _TBOX_PREFIX_App::onFrame( LeapSdk::Frame frame )
{
	mHands = frame.getHands();
}

void _TBOX_PREFIX_App::draw() 
{
	// Clear window
	gl::setViewport( getWindowBounds() );
	gl::clear( Colorf::black() );
	gl::setMatrices( mCamera );
	
	// Iterate through hands
	for( LeapSdk::HandMap::const_iterator handIter = mHands.begin(); handIter != mHands.end(); ++handIter ) {
		const LeapSdk::Hand& hand = handIter->second;

		// Fingers
		const LeapSdk::FingerMap& fingers = hand.getFingers();
		for( LeapSdk::FingerMap::const_iterator fingerIter = fingers.begin(); fingerIter != fingers.end(); ++fingerIter ) {
			const LeapSdk::Finger& finger = fingerIter->second;

			// Finger tip
			gl::drawColorCube( finger.getPosition(), Vec3f( 20, 20, 20 ) );

			// Finger orientation
			Vec3f tipPosition = finger.getPosition() + finger.getDirection() * finger.getLength();
			gl::color( ColorAf::gray( 0.8f ) );
			gl::drawLine( finger.getPosition(), tipPosition );
		}
	}
}

void _TBOX_PREFIX_App::shutdown()
{
	mLeap->removeCallback( mCallbackId );
	mHands.clear();
}

CINDER_APP_NATIVE( _TBOX_PREFIX_App, RendererGl )
