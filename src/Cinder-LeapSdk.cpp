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

Finger fromLeapFinger( const Leap::Finger& f )
{
	return (Finger)Pointable( (Leap::Pointable)f );
}

Leap::Finger toLeapFinger( const Finger& f )
{
	return (Leap::Finger)f.mPointable;
}

Frame fromLeapFrame( const Leap::Frame& f )
{
	return Frame( f );
}

Leap::Frame toLeapFrame( const Frame& f )
{
	return f.mFrame;
}

Hand fromLeapHand( const Leap::Hand& h, const Leap::Frame& f )
{
	return Hand( h, f );
}

Leap::Hand toLeapHand( const Hand& h )
{
	return h.mHand;
}

Matrix33f fromLeapMatrix33( const Leap::Matrix& m )
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

Leap::Matrix toLeapMatrix33( const Matrix33f& m )
{
	Leap::Matrix matrix;
	matrix.xBasis = Leap::Vector( m.m00, m.m01, m.m02 );
	matrix.yBasis = Leap::Vector( m.m10, m.m11, m.m12 );
	matrix.zBasis = Leap::Vector( m.m20, m.m21, m.m22 );
	return matrix;
}
	
Matrix44f fromLeapMatrix44( const Leap::Matrix& m )
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
	
Leap::Matrix toLeapMatrix44( const Matrix44f m )
{
	Leap::Matrix matrix;
	matrix.xBasis = Leap::Vector( m.m00, m.m01, m.m02 );
	matrix.yBasis = Leap::Vector( m.m10, m.m11, m.m12 );
	matrix.zBasis = Leap::Vector( m.m20, m.m21, m.m22 );
	matrix.origin = Leap::Vector( m.m30, m.m31, m.m32 );
	return matrix;
}

Pointable fromLeapPointable( const Leap::Pointable& p )
{
	return Pointable( p );
}

Leap::Pointable	toLeapPointable( const Pointable& p )
{
	return p.mPointable;
}

Screen fromLeapScreen( const Leap::Screen& s )
{
	return Screen( s );
}

Leap::Screen toLeapScreen( const Screen& s )
{
	return s.mScreen;
}

Tool fromLeapTool( const Leap::Tool& t )
{
	return (Tool)Pointable( (Leap::Pointable)t );
}

Leap::Tool toLeapTool( const Tool& t )
{
	return (Leap::Tool)t.mPointable;
}

Vec3f fromLeapVector( const Leap::Vector& v )
{
	return Vec3f( v.x, v.y, v.z );
}
	
Leap::Vector toLeapVector( const Vec3f& v )
{
	return Leap::Vector( v.x, v.y, v.z );
}
	
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
	return fromLeapVector( mPointable.direction() );
}

float Pointable::getLength() const
{
	return (float)mPointable.length();
}

Vec3f Pointable::getPosition() const
{
return fromLeapVector( mPointable.tipPosition() );
}

Vec3f Pointable::getVelocity() const
{
	return fromLeapVector( mPointable.tipVelocity() );
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
	
Hand::Hand( const Leap::Hand& h, const Leap::Frame& f )
{
	mHand = h;
	const Leap::PointableList& pointables = h.pointables();
	for ( Leap::PointableList::const_iterator ptIter = pointables.begin(); ptIter != pointables.end(); ++ptIter ) {
		const Leap::Pointable& pt = *ptIter;
		if ( pt.isValid() ) {
			Pointable pointable( pt );
			if ( pt.isFinger() ) {
				mFingers[ pt.id() ] = Finger( pointable );
			} else if ( pt.isTool() ) {
				mTools[ pt.id() ] = Tool( pointable );
			}
		}
	}
	
	mRotationAngle		= (float)h.rotationAngle( f );
	mRotationAxis		= fromLeapVector( h.rotationAxis( f ) );
	mRotationMatrix		= fromLeapMatrix44( h.rotationMatrix( f ) );
	mScale				= (float)h.scaleFactor( f );
	mTranslation		= fromLeapVector( h.translation( f ) );
}

Hand::~Hand()
{
	mFingers.clear();
	mTools.clear();
}

Vec3f Hand::getDirection() const
{
	return fromLeapVector( mHand.direction() );
}

const FingerMap& Hand::getFingers() const
{
	return mFingers;
}

Vec3f Hand::getNormal() const
{
	return fromLeapVector( mHand.palmNormal() );
}

Vec3f Hand::getPosition() const
{
	return fromLeapVector( mHand.palmPosition() );
}

float Hand::getRotationAngle() const
{
	return mRotationAngle;
}
	
float Hand::getRotationAngle( const Frame& f ) const
{
	return (float)mHand.rotationAngle( f.mFrame );
}
	
const Vec3f& Hand::getRotationAxis() const
{
	return mRotationAxis;
}
	
Vec3f Hand::getRotationAxis( const Frame& f ) const
{
	return fromLeapVector( mHand.rotationAxis( f.mFrame ) );
}

const Matrix44f& Hand::getRotationMatrix() const
{
	return mRotationMatrix;
}

Matrix44f Hand::getRotationMatrix( const Frame& f ) const
{
	return fromLeapMatrix44( mHand.rotationMatrix( f.mFrame ) );
}

float Hand::getScale() const
{
	return mScale;
}

float Hand::getScale( const Frame& f ) const
{
	return (float)mHand.scaleFactor( f.mFrame );
}
	
Vec3f Hand::getSpherePosition() const
{
	return fromLeapVector( mHand.sphereCenter() );
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

Vec3f Hand::getTranslation( const Frame& f ) const
{
	return fromLeapVector( mHand.translation( f.mFrame ) );
}

Vec3f Hand::getVelocity() const
{
	return fromLeapVector( mHand.palmVelocity() );
}

//////////////////////////////////////////////////////////////////////////////////////////////

Frame::Frame()
{
}

Frame::Frame( const Leap::Frame& frame )
{
	mFrame	= frame;
	
	mGestures.clear();
	Leap::GestureList gestures = mFrame.gestures();
	for ( Leap::GestureList::const_iterator iter = gestures.begin(); iter != gestures.end(); ++iter ) {
		mGestures.push_back( *iter );
	}
	
	mHands.clear();
	Leap::HandList hands = mFrame.hands();
	for ( Leap::HandList::const_iterator iter = hands.begin(); iter != hands.end(); ++iter ) {
		const Leap::Hand& hand	= *iter;
		mHands[ hand.id() ]		= Hand( hand, frame );
	}
}
	
Frame::~Frame()
{
	mHands.clear();
}

const vector<Leap::Gesture>& Frame::getGestures() const
{
	return mGestures;
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
	return fromLeapVector( mScreen.bottomLeftCorner() );
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
	return fromLeapVector( mScreen.horizontalAxis() );
}

Vec3f Screen::getNormal() const
{
	return fromLeapVector( mScreen.normal() );
}

Vec2i Screen::getSize() const
{
	return Vec2i( mScreen.widthPixels(), mScreen.heightPixels() );
}

Vec3f Screen::getVerticalAxis() const
{
	return fromLeapVector( mScreen.verticalAxis() );
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
	*result			= fromLeapVector( v );
	return true;
}

Vec3f Screen::project( const Vec3f& v, bool normalize, float clampRatio ) const
{
	return fromLeapVector( mScreen.project( toLeapVector( v ), normalize, clampRatio ) );
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
		const Leap::Frame& frame	= controller.frame();
		
		mFrame						= Frame( frame );
		if ( !mFirstFrameReceived ) {
			mFirstFrame			= mFrame;
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

void Device::enableGesture( Gesture::Type t )
{
	if ( mController != 0 ) {
		mController->enableGesture( t );
	}
}

const Screen& Device::getClosestScreen( const Pointable& p ) const
{
	const Leap::ScreenList& screens = mController->calibratedScreens();
	if ( screens.empty() ) {
		throw ExcNoCalibratedScreens();
	}
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
	throw ExcNoClosestScreen();
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

}
