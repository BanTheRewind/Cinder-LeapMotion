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
}
	
Pointable::Pointable( const Leap::Pointable& p )
{
	mPointable = p;
}
	
Pointable::Pointable( const Pointable& p )
{
	mPointable	= p.mPointable;
}
	
Vec3f Pointable::getDirection() const
{
	return toVec3f( mPointable.direction() );
}

float Pointable::getLength() const
{
	return (float)mPointable.length();
}

Vec3f Pointable::getPosition() const
{
return toVec3f( mPointable.tipPosition() );
}

Vec3f Pointable::getVelocity() const
{
	return toVec3f( mPointable.tipVelocity() );
}

float Pointable::getWidth() const
{
return (float)mPointable.width();
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
	
Hand::Hand( const Leap::Hand& hand, const FingerMap& fingerMap, const ToolMap& toolMap, float rotAngle,
		   const ci::Vec3f& rotAxis, const ci::Matrix44f& rotMatrix, float scale,
		   const ci::Vec3f& translation )
{
	mHand			= hand;
	mFingers		= fingerMap;
	mRotationAngle	= rotAngle;
	mRotationAxis	= rotAxis;
	mRotationMatrix	= rotMatrix;
	mScale			= scale;
	mTools			= toolMap;
	mTranslation	= translation;
}

Hand::~Hand()
{
	mFingers.clear();
	mTools.clear();
}

Vec3f Hand::getDirection() const
{
	return toVec3f( mHand.direction() );
}

const FingerMap& Hand::getFingers() const
{
	return mFingers;
}

Vec3f Hand::getNormal() const
{
	return toVec3f( mHand.palmNormal() );
}

Vec3f Hand::getPosition() const
{
	return toVec3f( mHand.palmPosition() );
}

float Hand::getRotationAngle() const
{
	return mRotationAngle;
}
	
float Hand::getRotationAngle( const Frame& frame ) const
{
	return (float)mHand.rotationAngle( frame.mFrame );
}
	
const Vec3f& Hand::getRotationAxis() const
{
	return mRotationAxis;
}
	
Vec3f Hand::getRotationAxis( const Frame& frame ) const
{
	return toVec3f( mHand.rotationAxis( frame.mFrame ) );
}

const Matrix44f& Hand::getRotationMatrix() const
{
	return mRotationMatrix;
}

Matrix44f Hand::getRotationMatrix( const Frame& frame ) const
{
	return toMatrix44f( mHand.rotationMatrix( frame.mFrame ) );
}

float Hand::getScale() const
{
	return mScale;
}

float Hand::getScale( const Frame& frame ) const
{
	return (float)mHand.scaleFactor( frame.mFrame );
}
	
Vec3f Hand::getSpherePosition() const
{
	return toVec3f( mHand.sphereCenter() );
}

float Hand::getSphereRadius() const
{
	return (float)mHand.sphereRadius();
}

const ToolMap& Hand::getTools() const
{
	return mTools;
}

const Vec3f& Hand::getTranslation() const
{
	return mTranslation;
}

Vec3f Hand::getTranslation( const Frame& frame ) const
{
	return toVec3f( mHand.translation( frame.mFrame ) );
}

Vec3f Hand::getVelocity() const
{
	return toVec3f( mHand.palmVelocity() );
}

//////////////////////////////////////////////////////////////////////////////////////////////

Frame::Frame()
{
}

Frame::Frame( const Leap::Frame& frame, const HandMap& handMap )
{
	mFrame	= frame;
	mHands	= handMap;
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
	return mFrame.id();
}

int64_t Frame::getTimestamp() const
{
	return mFrame.timestamp();
}

	
//////////////////////////////////////////////////////////////////////////////////////////////

Screen::Screen()
{
}

Screen::Screen( const Leap::Screen& screen )
{
	mScreen = screen;
}
	
float Screen::distanceToPoint( const Vec3f& v ) const
{
	Leap::Vector point;
	point.x = v.x;
	point.y = v.y;
	point.z = v.z;
	return mScreen.distanceToPoint( point );
}

Vec3f Screen::getBottomLeft() const
{
	return toVec3f( mScreen.bottomLeftCorner() );
}

string Screen::getDescription() const
{
	return mScreen.toString();
}

int32_t Screen::getHeight() const
{
	return mScreen.heightPixels();
}

Vec3f Screen::getHorizontalAxis() const
{
	return toVec3f( mScreen.horizontalAxis() );
}

Vec3f Screen::getNormal() const
{
	return toVec3f( mScreen.normal() );
}

Vec2i Screen::getSize() const
{
	return Vec2i( mScreen.widthPixels(), mScreen.heightPixels() );
}

Vec3f Screen::getVerticalAxis() const
{
	return toVec3f( mScreen.verticalAxis() );
}

int32_t Screen::getWidth() const
{
	return mScreen.widthPixels();
}

bool Screen::intersects( const Pointable& p, Vec3f* result, bool normalize,
						float clampRatio ) const
{
	Leap::Vector v	= mScreen.intersect( p.mPointable, normalize, clampRatio );
	if ( v.x != v.x ||
		v.y != v.y ||
		v.z != v.z ) { // NaN
		return false;
	}
	*result			= toVec3f( v );
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////

Listener::Listener()
{
	mConnected			= false;
	mExited				= false;
	mFirstFrameReceived	= false;
	mInitialized		= false;
	mNewFrame			= false;
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
		for ( Leap::HandList::const_iterator iter = hands.begin(); iter != hands.end(); ++iter ) {
			const Leap::Hand& hand = *iter;
			FingerMap fingerMap;
			ToolMap toolMap;
			const Leap::PointableList& pointables = hand.pointables();
			for ( Leap::PointableList::const_iterator ptIter = pointables.begin(); ptIter != pointables.end(); ++ptIter ) {
				const Leap::Pointable& pt = *ptIter;
				if ( pt.isValid() ) {
					Pointable pointable( pt );
					if ( pt.isFinger() ) {
						fingerMap[ pt.id() ] = Finger( pointable );
					} else if ( pt.isTool() ) {
						toolMap[ pt.id() ] = Tool( pointable );
					}
				}
			}
			
			float rotAngle			= (float)hand.rotationAngle( mFirstFrame.mFrame );
			Vec3f rotAxis			= toVec3f( hand.rotationAxis( mFirstFrame.mFrame ) );
			Matrix44f rotMatrix		= toMatrix44f( hand.rotationMatrix( mFirstFrame.mFrame ) );
			float scale				= (float)hand.scaleFactor( mFirstFrame.mFrame );
			Vec3f translation		= toVec3f( hand.translation( mFirstFrame.mFrame ) );
			
			handMap[ hand.id() ]	= Hand( hand, fingerMap, toolMap, rotAngle, rotAxis,
										   rotMatrix, scale, translation );
		}

		mFrame		= Frame( controllerFrame, handMap );
		if ( !mFirstFrameReceived ) {
			mFirstFrame			= Frame( controllerFrame, handMap );
			mFirstFrameReceived	= true;
		}
		mNewFrame	= true;
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
	mListener.mMutex	= &mMutex;
	mController			= new Leap::Controller( mListener );
}

Device::~Device()
{
	for ( CallbackList::const_iterator iter = mCallbacks.begin(); iter != mCallbacks.end(); ++iter ) {
		iter->second->disconnect();
	}
	mCallbacks.clear();
}

const Screen& Device::getClosestScreen( const Pointable& p ) const
{
	const Leap::ScreenList& screens = mController->calibratedScreens();
	Leap::Screen closestScreen = screens.closestScreenHit( p.mPointable );
	for ( ScreenMap::const_iterator iter = mScreens.begin(); iter != mScreens.end(); ++iter ) {
		const Screen& screen = iter->second;
		if ( screen.mScreen.id() == closestScreen.id() ) {
			return screen;
		}
	}
	if ( mScreens.begin() != mScreens.end() ) {
		return mScreens.begin()->second;
	}
	throw Exception();
}
	
Leap::Config Device::getConfig() const
{
	return mController->config();
}
	
const ScreenMap& Device::getScreens() const
{
	return mScreens;
}

bool Device::hasExited() const
{
	return mListener.mExited;
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
	const Leap::ScreenList& screens = mController->calibratedScreens();
	mScreens.clear();
	size_t count = screens.count();
	for ( size_t i = 0; i < count; ++i ) {
		mScreens[ i ] = Screen( screens[ i ] );
	}
}
	
//////////////////////////////////////////////////////////////////////////////////////////////

Matrix33f toMatrix33f( const Leap::Matrix& m )
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

Matrix44f toMatrix44f( const Leap::Matrix& m )
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

Vec3f toVec3f( const Leap::Vector& v )
{
	return Vec3f( (float)v.x, (float)v.y, (float)v.z );
}

}
