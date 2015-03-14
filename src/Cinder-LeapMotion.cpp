/*
* 
* Copyright (c) 2015, Ban the Rewind
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

Channel8u toChannel8u( const Leap::Image& img, bool copyData )
{
	int32_t h = img.height();
	int32_t w = img.width();
	Channel8u channel;
	if ( copyData ) {
		channel = Channel8u( w, h );
		char_traits<uint8_t>::copy( channel.getData(), img.data(), w * h * sizeof( uint8_t ) );
	} else {
		channel = Channel8u( w, h, w * sizeof( uint8_t ), sizeof( uint8_t ), (uint8_t*)img.data() );
	}
	return channel;
}

mat3 toMat3( const Leap::Matrix& m )
{
	mat3 mat;
	Leap::FloatArray a = m.toArray3x3();
	for ( size_t x = 0; x < 3; ++x ) {
		for ( size_t y = 0; y < 3; ++y ) {
			mat[ x ][ y ] = a[ y * 3 + x ];
		}
	}
	return mat;
}

Leap::Matrix toLeapMatrix( const mat3& m )
{
	Leap::Matrix matrix;
	matrix.xBasis = Leap::Vector( m[ 0 ][ 0 ], m[ 0 ][ 1 ], m[ 0 ][ 2 ] );
	matrix.yBasis = Leap::Vector( m[ 1 ][ 0 ], m[ 1 ][ 1 ], m[ 1 ][ 2 ] );
	matrix.zBasis = Leap::Vector( m[ 2 ][ 0 ], m[ 2 ][ 1 ], m[ 2 ][ 2 ] );
	return matrix;
}
	
mat4 toMat4( const Leap::Matrix& m )
{
	mat4 mat;
	Leap::FloatArray a = m.toArray4x4();
	for ( size_t x = 0; x < 4; ++x ) {
		for ( size_t y = 0; y < 4; ++y ) {
			mat[ x ][ y ] = a[ y * 4 + x ];
		}
	}
	return mat;
}
	
Leap::Matrix toLeapMatrix( const mat4 m )
{
	Leap::Matrix matrix;
	matrix.xBasis = Leap::Vector( m[ 0 ][ 0 ], m[ 0 ][ 1 ], m[ 0 ][ 2 ] );
	matrix.yBasis = Leap::Vector( m[ 1 ][ 0 ], m[ 1 ][ 1 ], m[ 1 ][ 2 ] );
	matrix.zBasis = Leap::Vector( m[ 2 ][ 0 ], m[ 2 ][ 1 ], m[ 2 ][ 2 ] );
	matrix.origin = Leap::Vector( m[ 3 ][ 0 ], m[ 3 ][ 1 ], m[ 3 ][ 2 ] );
	return matrix;
}

vec3 toVec3( const Leap::Vector& v )
{
	return vec3( v.x, v.y, v.z );
}
	
Leap::Vector toLeapVector( const vec3& v )
{
	return Leap::Vector( v.x, v.y, v.z );
}

//////////////////////////////////////////////////////////////////////////////////////////////

Listener::Listener()
{
	mConnected		= false;
	mExited			= false;
	mFocused		= false;
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
	mController->removeListener( mListener );
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
