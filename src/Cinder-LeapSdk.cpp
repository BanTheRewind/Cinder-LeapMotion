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

#include "Cinder-LeapSdk.h"

using namespace ci;
using namespace std;

namespace LeapSdk {

//////////////////////////////////////////////////////////////////////////////////////////////

Pointable::Pointable()
{
	mDirection	= Vec3f::zero();
	mLength		= 0.0f;
	mPosition	= Vec3f::zero();
	mVelocity	= Vec3f::zero();
	mWidth		= 0.0f;
}
	
Pointable::Pointable( const Pointable& p )
{
	mDirection	= p.mDirection;
	mLength		= p.mLength;
	mPosition	= p.mPosition;
	mVelocity	= p.mVelocity;
	mWidth		= p.mWidth;
}
	
const Vec3f& Pointable::getDirection() const
{
	return mDirection;
}

float Pointable::getLength() const
{
	return mLength;
}

const Vec3f& Pointable::getPosition() const
{
	return mPosition;
}

const Vec3f& Pointable::getVelocity() const
{
	return mVelocity;
}

float Pointable::getWidth() const
{
	return mWidth;
}

Finger::Finger()
: Pointable()
{
}

Finger::Finger( const Pointable& p )
: Pointable( p )
{
}

Tool::Tool()
: Pointable()
{
}
	
Tool::Tool( const Pointable& p )
: Pointable( p )
{
}

//////////////////////////////////////////////////////////////////////////////////////////////

Hand::Hand()
{
}

Hand::~Hand()
{
	mFingers.clear();
	mTools.clear();
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
	
const Vec3f& Hand::getSpherePosition() const
{
	return mSpherePosition;
}

float Hand::getSphereRadius() const
{
	return mSphereRadius;
}

const Vec3f& Hand::getVelocity() const
{
	return mVelocity;
}

//////////////////////////////////////////////////////////////////////////////////////////////

Frame::Frame()
{
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

Listener::Listener()
{
	mConnected		= false;
	mExited			= false;
	mInitialized	= false;
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
	
void Listener::onExit( const Leap::Controller& controller )
{
	lock_guard<mutex> lock( *mMutex );
	mExited = true;
}

void Listener::onFrame( const Leap::Controller& controller ) 
{
	lock_guard<mutex> lock( *mMutex );
	if ( !mNewFrame ) {
		const Leap::Frame& controllerFrame	= controller.frame();
		const Leap::HandList& hands			= controllerFrame.hands();
		
		HandMap handMap;
		for ( Leap::HandList::const_iterator handIter = hands.begin(); handIter != hands.end(); ++handIter ) {
			const Leap::Hand& hand	= *handIter;
			Hand outHand;

			FingerMap fingerMap;
			ToolMap toolMap;
			const Leap::PointableList& pointables = hand.pointables();
			for ( Leap::PointableList::const_iterator ptIter = pointables.begin(); ptIter != pointables.end(); ++ptIter ) {
				const Leap::Pointable& pt = *ptIter;
				if ( pt.isValid() ) {
					Pointable pointable;
					
					pointable.mDirection	= toVec3f( pt.direction() );
					pointable.mLength		= (float)pt.length();
					pointable.mPosition		= toVec3f( pt.tipPosition() );
					pointable.mVelocity		= toVec3f( pt.tipVelocity() );
					pointable.mWidth		= (float)pt.width();
					
					if ( pt.isFinger() ) {
						fingerMap[ pt.id() ] = Finger( pointable );
					} else if ( pt.isTool() ) {
						toolMap[ pt.id() ] = Tool( pointable );
					}
				}
			}
			
			outHand.mDirection			= toVec3f( hand.direction() );
			outHand.mFingers			= fingerMap;
			outHand.mNormal				= toVec3f( hand.palmNormal() );
			outHand.mPosition			= toVec3f( hand.palmPosition() );
			outHand.mRotationAngle		= (float)hand.rotationAngle( mLeapFrame );
			outHand.mRotationAxis		= toVec3f( hand.rotationAxis( mLeapFrame ) );
			outHand.mRotationMatrix2d	= toMatrix33f( hand.rotationMatrix( mLeapFrame ) );
			outHand.mRotationMatrix3d	= toMatrix44f( hand.rotationMatrix( mLeapFrame ) );
			outHand.mScale				= (float)hand.scaleFactor( mLeapFrame );
			outHand.mSphereRadius		= (float)hand.sphereRadius();
			outHand.mSpherePosition		= toVec3f( hand.sphereCenter() );
			outHand.mTools				= toolMap;
			outHand.mTranslation		= toVec3f( hand.translation( mLeapFrame ) );
			outHand.mVelocity			= toVec3f( hand.palmVelocity() );

			handMap[ hand.id() ] = outHand;
		}

		mFrame.mHands		= handMap;
		mFrame.mId			= controllerFrame.id();
		mFrame.mTimestamp	= controllerFrame.timestamp();
		
		mLeapFrame			= controllerFrame;
		mNewFrame			= true;
	}
}

void Listener::onInit( const Leap::Controller& controller ) 
{
	lock_guard<mutex> lock( *mMutex );
	mInitialized = true;
}

Matrix33f Listener::toMatrix33f( const Leap::Matrix& m )
{
	Matrix33f mtx;
	Leap::FloatArray a = m.toArray3x3();
	for ( size_t i = 0; i < 3; ++i ) {
		size_t j = i * 3;
		Vec3f row( a[ j + 0 ], a[ j + 1 ], a[ j + 2 ] );
		mtx.setRow( i, row );
	}
	return mtx;
}
	
Matrix44f Listener::toMatrix44f( const Leap::Matrix& m )
{
	Matrix44f mtx;
	Leap::FloatArray a = m.toArray4x4();
	for ( size_t i = 0; i < 4; ++i ) {
		size_t j = i * 4;
		Vec4f row( a[ j + 0 ], a[ j + 1 ], a[ j + 2 ], a[ j + 3 ] );
		mtx.setRow( i, row );
	}
	return mtx;
}
	
Vec3f Listener::toVec3f( const Leap::Vector& v )
{
	return Vec3f( (float)v.x, (float)v.y, (float)v.z );
}

//////////////////////////////////////////////////////////////////////////////////////////////

DeviceRef Device::create()
{
	return DeviceRef( new Device() );
}

Device::Device()
{
	mListener.mMutex	= &mMutex;
	mController			= new Leap::Controller( mListener );
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
	return mListener.mConnected;
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

void Device::update()
{
	lock_guard<mutex> lock( mMutex );
	if ( mListener.mNewFrame ) {
		mSignal( mListener.mFrame );
		mListener.mNewFrame = false;
	}
}

}
