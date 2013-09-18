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

#include "Cinder-LeapMotion.h"

#include "cinder/app/App.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace LeapMotion {

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

Leap::Matrix toLeapMatrix( const Matrix33f& m )
{
	Leap::Matrix matrix;
	matrix.xBasis = Leap::Vector( m.m00, m.m01, m.m02 );
	matrix.yBasis = Leap::Vector( m.m10, m.m11, m.m12 );
	matrix.zBasis = Leap::Vector( m.m20, m.m21, m.m22 );
	return matrix;
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
	
Leap::Matrix toLeapMatrix( const Matrix44f m )
{
	Leap::Matrix matrix;
	matrix.xBasis = Leap::Vector( m.m00, m.m01, m.m02 );
	matrix.yBasis = Leap::Vector( m.m10, m.m11, m.m12 );
	matrix.zBasis = Leap::Vector( m.m20, m.m21, m.m22 );
	matrix.origin = Leap::Vector( m.m30, m.m31, m.m32 );
	return matrix;
}

Vec3f toVec3f( const Leap::Vector& v )
{
	return Vec3f( v.x, v.y, v.z );
}
	
Leap::Vector toLeapVector( const Vec3f& v )
{
	return Leap::Vector( v.x, v.y, v.z );
}

//////////////////////////////////////////////////////////////////////////////////////////////

Listener::Listener()
{
	mConnected			= false;
	mExited				= false;
	mFocused			= false;
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

void Listener::onFocusGained( const Leap::Controller& controller )
{
	lock_guard<mutex> lock( *mMutex );
	mFocused = true;
}

void Listener::onFocusLost( const Leap::Controller& controller )
{
	lock_guard<mutex> lock( *mMutex );
	mFocused = false;
}
	
void Listener::onFrame( const Leap::Controller& controller ) 
{
	lock_guard<mutex> lock( *mMutex );
	if ( !mNewFrame ) {
		mFrame		= controller.frame();
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

	App::get()->getSignalUpdate().connect( bind( &Device::update, this ) );
}

Device::~Device()
{
}

Leap::Controller* Device::getController() const
{
	return mController;
}
	
bool Device::hasExited() const
{
	return mListener.mExited;
}
	
bool Device::hasFocus() const
{
	return mListener.mFocused;
}
	
bool Device::isConnected() const
{
	return mListener.mConnected;
}

bool Device::isInitialized() const
{
	return mListener.mInitialized;
}

void Device::connectEventHandler( const function<void( Leap::Frame )>& eventHandler )
{
	mEventHandler = eventHandler;
}

void Device::update()
{
	lock_guard<mutex> lock( mMutex );
	if ( mListener.mConnected && mListener.mInitialized && mListener.mNewFrame ) {
		mEventHandler( mListener.mFrame );
		mListener.mNewFrame = false;
	}
}
	
}
