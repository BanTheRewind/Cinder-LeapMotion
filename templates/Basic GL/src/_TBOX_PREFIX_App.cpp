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

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"

#include "Cinder-LeapMotion.h"

class _TBOX_PREFIX_App : public ci::app::App
{
 public:
	void 					draw() override;
	void 					setup() override;
 private:	
	Leap::Frame				mFrame;
	LeapMotion::DeviceRef	mLeap;

	ci::CameraPersp			mCamera;
};

using namespace ci;
using namespace ci::app;
using namespace std;

void _TBOX_PREFIX_App::draw() 
{
	gl::viewport( getWindowSize() );
	gl::clear( Colorf::black() );
	gl::setMatrices( mCamera );
	
	const Leap::HandList& hands = mFrame.hands();
	for ( const Leap::Hand& hand : hands ) {
		for ( const Leap::Pointable& pointable : hand.pointables() ) {
			vec3 dir		= LeapMotion::toVec3( pointable.direction() );
			float length	= pointable.length();
			vec3 tipPos		= LeapMotion::toVec3( pointable.tipPosition() );
			vec3 basePos	= tipPos + dir * length;
			
			gl::drawColorCube( tipPos, vec3( 20.0f, 20.0f, 20.0f ) );
			gl::color( ColorAf::gray( 0.8f ) );
			gl::drawLine( basePos, tipPos );
		}
	}
}

void _TBOX_PREFIX_App::setup()
{	 
	gl::enableDepthRead();
	gl::enableDepthWrite();
	
	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f, 0.01f, 1000.0f );
	mCamera.lookAt( vec3( 0.0f, 125.0f, 500.0f ), vec3( 0.0f, 250.0f, 0.0f ) );
	
	mLeap = LeapMotion::Device::create();
	mLeap->connectEventHandler( [ & ]( Leap::Frame frame )
	{
		mFrame = frame;
	} );
}

CINDER_APP( _TBOX_PREFIX_App, RendererGl )
