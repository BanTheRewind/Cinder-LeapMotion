/*
* 
* Copyright (c) 2012, Ban the Rewind
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

const Vec3f& Finger::getDirection() const
{
	return mDirection;
}

float Finger::getLength() const
{
	return mLength;
}

const Vec3f& Finger::getPosition() const
{
	return mPosition;
}

const Vec3f& Finger::getVelocity() const
{
	return mVelocity;
}

float Finger::getWidth() const
{
	return mWidth;
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

const Vec3f& Hand::getBallPosition() const
{
	return mBallPosition;
}

float Hand::getBallRadius() const
{
	return mBallRadius;
}

const Vec3f& Hand::getDirection() const
{
	return mDirection;
}

const FingerMap& Hand::getFingers() const
{
	return mFingers;
}

const Vec3f& Hand::getNormal() const
{
	return mNormal;
}

const Vec3f& Hand::getPosition() const
{
	return mPosition;
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

const HandMap& Frame::getHands() const
{
	return mHands;
}

int64_t Frame::getId() const
{
	return mId;
}

int64_t Frame::getTimestamp() const
{
	return mTimestamp;
}

//////////////////////////////////////////////////////////////////////////////////////////////

Listener::Listener( mutex *mutex )
{
	mConnected		= false;
	mInitialized	= false;
	mMutex			= mutex;
	mNewFrame		= false;
}

void Listener::onConnect( const Leap::Controller& controller ) 
{
	lock_guard<mutex> lock( *mMutex );
	mConnected = true;
}

void Listener::onDisconnect( const Leap::Controller& controller ) 
{
	lock_guard<mutex> lock( *mMutex );
	mConnected = false;
}

void Listener::onFrame( const Leap::Controller& controller ) 
{
	lock_guard<mutex> lock( *mMutex );
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
				ballPosition			= Vec3f( (float)p.x, (float)p.y, (float)p.z );
				ballRadius				= (float)ball->radius;
			}

			const Leap::Ray* palmRay = handIter->palm();
			if ( palmRay != 0 ) {
				const Leap::Vector& d	= palmRay->direction;
				const Leap::Vector& p	= palmRay->position;
				direction				= Vec3f( (float)d.x, (float)d.y, (float)d.z );
				position				= Vec3f( (float)p.x, (float)p.y, (float)p.z );
			}

			const Leap::Vector* n		= handIter->normal();
			if ( n != 0 ) {
				normal					= Vec3f( (float)n->x, (float)n->y, (float)n->z );
			}

			const vector<Leap::Finger>& fingers = handIter->fingers();
			for ( vector<Leap::Finger>::const_iterator fingerIter = fingers.begin(); fingerIter != fingers.end(); ++fingerIter ) {
				const Leap::Ray& tip	= fingerIter->tip();
				const Leap::Vector& d	= tip.direction;
				const Leap::Vector& p	= tip.position;
				Vec3f fingerDirection	= Vec3f( (float)d.x, (float)d.y, (float)d.z );
				Vec3f fingerPosition	= Vec3f( (float)p.x, (float)p.y, (float)p.z );

				Vec3f fingerVelocity	= Vec3f::zero();
				const Leap::Vector* v	= fingerIter->velocity();
				if ( v != 0 ) {
					fingerVelocity		= Vec3f( (float)v->x, (float)v->y, (float)v->z );
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
	lock_guard<mutex> lock( *mMutex );
	mInitialized = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////

DeviceRef Device::create()
{
	return DeviceRef( new Device() );
}

Device::Device()
{
	mListener	= new Listener( &mMutex );
	mController = new Leap::Controller( mListener );
}

Device::~Device()
{
	for ( CallbackList::iterator iter = mCallbacks.begin(); iter != mCallbacks.end(); ) {
		iter->second->disconnect();
		iter = mCallbacks.erase( iter );
	}
}

bool Device::isConnected() const
{
	return mListener->mConnected;
}

bool Device::isInitialized() const
{
	return mListener->mInitialized;
}

void Device::removeCallback( uint32_t id )
{
	if ( mCallbacks.find( id ) != mCallbacks.end() ) {
		mCallbacks.find( id )->second->disconnect();
		mCallbacks.erase( id ); 
	}
}

void Device::update()
{
	lock_guard<mutex> lock( mMutex );
	if ( mListener->mNewFrame ) {
		mSignal( mListener->mFrame );
		mListener->mNewFrame = false;
	}
}

}
