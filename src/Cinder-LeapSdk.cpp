#include "Cinder-LeapSdk.h"

using namespace ci;
using namespace std;

namespace LeapSdk {

//////////////////////////////////////////////////////////////////////////////////////////////

Finger::Finger( const Vec3f& position, const Vec3f& direction, const Vec3f& velocity, 
					 float length, float width, bool isTool )
{
	mDirection	= direction.normalized();
	mIsTool		= isTool;
	mLength		= length;
	mPosition	= position;
	mVelocity	= velocity;
	mWidth		= width;
}

const Vec3f& Finger::getDirection()
{
	return mDirection;
}

const Vec3f& Finger::getDirection() const
{
	return mDirection;
}

float Finger::getLength()
{
	return mLength;
}

float Finger::getLength() const
{
	return mLength;
}

const Vec3f& Finger::getPosition()
{
	return mPosition;
}

const Vec3f& Finger::getPosition() const
{
	return mPosition;
}

const Vec3f& Finger::getVelocity()
{
	return mVelocity;
}

const Vec3f& Finger::getVelocity() const
{
	return mVelocity;
}

float Finger::getWidth()
{
	return mWidth;
}

float Finger::getWidth() const
{
	return mWidth;
}

bool Finger::isTool()
{
	return mIsTool;
}

bool Finger::isTool() const
{
	return mIsTool;
}

//////////////////////////////////////////////////////////////////////////////////////////////

Hand::Hand( const FingerMap& fingerMap, const Vec3f& position, const Vec3f& direction, 
	 const Vec3f& velocity, const Vec3f& normal, const Vec3f& ballPosition, float ballRadius )
{
	mBallPosition	= ballPosition;
	mBallRadius		= ballRadius;
	mDirection		= direction.normalized();
	mFingers		= fingerMap;
	mNormal			= normal.normalized();
	mPosition		= position;
	mVelocity		= velocity;
}

Hand::~Hand()
{
	mFingers.clear();
}

const Vec3f& Hand::getBallPosition()
{
	return mBallPosition;
}

const Vec3f& Hand::getBallPosition() const
{
	return mBallPosition;
}

float Hand::getBallRadius()
{
	return mBallRadius;
}

float Hand::getBallRadius() const
{
	return mBallRadius;
}

const Vec3f& Hand::getDirection()
{
	return mDirection;
}

const Vec3f& Hand::getDirection() const
{
	return mDirection;
}

const FingerMap& Hand::getFingers()
{
	return mFingers;
}

const FingerMap& Hand::getFingers() const
{
	return mFingers;
}

const Vec3f& Hand::getNormal()
{
	return mNormal;
}

const Vec3f& Hand::getNormal() const
{
	return mNormal;
}

const Vec3f& Hand::getPosition()
{
	return mPosition;
}

const Vec3f& Hand::getPosition() const
{
	return mPosition;
}

const Vec3f& Hand::getVelocity()
{
	return mVelocity;
}

const Vec3f& Hand::getVelocity() const
{
	return mVelocity;
}

//////////////////////////////////////////////////////////////////////////////////////////////

Frame::Frame( const HandMap& handMap, int64_t id, int64_t timestamp )
{
	mHands		= handMap;
	mId			= id;
	mTimestamp	= timestamp;
}

Frame::~Frame()
{
	mHands.clear();
}

const HandMap& Frame::getHands()
{
	return mHands;
}

const HandMap& Frame::getHands() const
{
	return mHands;
}

int64_t Frame::getId()
{
	return mId;
}

int64_t Frame::getId() const
{
	return mId;
}

int64_t Frame::getTimestamp()
{
	return mTimestamp;
}

int64_t Frame::getTimestamp() const
{
	return mTimestamp;
}

//////////////////////////////////////////////////////////////////////////////////////////////

Listener::Listener()
{
	mConnected		= false;
	mInitialized	= false;
	mNewFrame		= false;
}

void Listener::onConnect( const Leap::Controller& controller ) 
{
	lock_guard<mutex> lock( mMutex );
	mConnected = true;
}

void Listener::onDisconnect( const Leap::Controller& controller ) 
{
	lock_guard<mutex> lock( mMutex );
	mConnected = false;
}

void Listener::onFrame( const Leap::Controller& controller ) 
{
	lock_guard<mutex> lock( mMutex );
	if ( !mNewFrame ) {
		const Leap::Frame& controllerFrame	= controller.frame();
		const vector<Leap::Hand>& hands		= controllerFrame.hands();
		HandMap handMap;
		for ( vector<Leap::Hand>::const_iterator handIter = hands.begin(); handIter != hands.end(); ++handIter ) {
			float ballRadius			= 0.0f;	
			Vec3f ballPosition			= Vec3f::zero();
			Vec3f direction				= Vec3f::zero();
			FingerMap fingerMap;
			Vec3f normal				= Vec3f::zero();
			Vec3f position				= Vec3f::zero();
			Vec3f velocity				= Vec3f::zero();

			const Leap::Ball* ball		= handIter->ball();
			if ( ball != 0 ) {
				const Leap::Vector& p	= ball->position;
				ballPosition			= Vec3f( p.x, p.y, p.z );
				ballRadius				= ball->radius;
			}

			const Leap::Ray* palmRay = handIter->palm();
			if ( palmRay != 0 ) {
				const Leap::Vector& d	= palmRay->direction;
				const Leap::Vector& p	= palmRay->position;
				direction				= Vec3f( d.x, d.y, d.z );
				position				= Vec3f( p.x, p.y, p.z );
			}

			const Leap::Vector* n		= handIter->normal();
			if ( n != 0 ) {
				normal					= Vec3f( n->x, n->y, n->z );
			}

			const vector<Leap::Finger>& fingers = handIter->fingers();
			for ( vector<Leap::Finger>::const_iterator fingerIter = fingers.begin(); fingerIter != fingers.end(); ++fingerIter ) {
				const Leap::Ray& tip	= fingerIter->tip();
				const Leap::Vector& d	= tip.direction;
				const Leap::Vector& p	= tip.position;
				Vec3f fingerDirection	= Vec3f( d.x, d.y, d.z );
				Vec3f fingerPosition	= Vec3f( p.x, p.y, p.z );

				Vec3f fingerVelocity	= Vec3f::zero();
				const Leap::Vector* v	= fingerIter->velocity();
				if ( v != 0 ) {
					fingerVelocity		= Vec3f( v->x, v->y, v->z );
				}

				bool isTool				= fingerIter->isTool();
				float length			= (float)fingerIter->length();
				float width				= (float)fingerIter->width();

				Finger finger( fingerPosition, fingerDirection, fingerVelocity, length, width, isTool );
				fingerMap[ fingerIter->id() ] = finger;
			}

			Hand hand( fingerMap, position, direction, velocity, normal, ballPosition, ballRadius );
			handMap[ handIter->id() ] = hand;
		}

		int64_t id						= controllerFrame.id();
		int64_t timestamp				= controllerFrame.timestamp();
		mFrame							= Frame( handMap, id, timestamp );
		mNewFrame						= true;
	}
}

void Listener::onInit( const Leap::Controller& controller ) 
{
	lock_guard<mutex> lock( mMutex );
	mInitialized = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////

DeviceRef Device::create()
{
	return DeviceRef( new Device() );
}

Device::Device()
{
	mRunning = false;
}

Device::~Device()
{
	for ( CallbackList::iterator iter = mCallbacks.begin(); iter != mCallbacks.end(); ) {
		iter->second->disconnect();
		iter = mCallbacks.erase( iter );
	}
}

Listener* Device::getListener()
{
	return &mListener;
}

bool Device::isConnected()
{
	return mListener.mConnected;
}

bool Device::isConnected() const
{
	return mListener.mConnected;
}

bool Device::isInitialized()
{
	return mListener.mInitialized;
}

bool Device::isInitialized() const
{
	return mListener.mInitialized;
}

void Device::removeCallback( uint32_t id )
{
	if ( mCallbacks.find( id ) != mCallbacks.end() ) {
		mCallbacks.find( id )->second->disconnect();
		mCallbacks.erase( id ); 
	}
}

void Device::run()
{
	lock_guard<mutex> lock( mListener.mMutex );
	Leap::Controller controller( &mListener );
	while ( mRunning ) {
	}
}

void Device::start()
{
	mRunning	= true;
	mThread		= ThreadRef( new thread( &Device::run, this ) );
}

void Device::stop()
{
	mRunning = false;
	if ( mThread ) {
		mThread->join();
		mThread.reset();
	}
}

void Device::update()
{
	if ( mListener.mNewFrame ) {
		mSignal( mListener.mFrame );
		mListener.mNewFrame = false;
	}
}

}
