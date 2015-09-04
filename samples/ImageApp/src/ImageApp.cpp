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
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "Cinder-LeapMotion.h"

class ImageApp : public ci::app::App
{
public:
	ImageApp();

	void						draw() override;
	void						update() override;
private:
	LeapMotion::DeviceRef		mDevice;
	Leap::Frame					mFrame;

	float						mFrameRate;
	bool						mFullScreen;
	ci::params::InterfaceGlRef	mParams;
	void						screenShot();
};

#include "cinder/app/RendererGl.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace LeapMotion;
using namespace std;

ImageApp::ImageApp()
{
	mFrameRate = 0.0f;
	mFullScreen = false;

	mDevice = Device::create();
	mDevice->getController()->setPolicyFlags( Leap::Controller::POLICY_IMAGES );
	mDevice->connectEventHandler( [ &]( Leap::Frame frame )
	{
		mFrame = frame;
	} );

	mParams = params::InterfaceGl::create( "Params", ivec2( 200, 105 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,				"", true );
	mParams->addParam( "Full screen",	&mFullScreen ).key( "f" );
	mParams->addButton( "Screen shot",	[ & ]() { screenShot(); },	"key=space" );
	mParams->addButton( "Quit",			[ & ]() { quit(); },		"key=q" );

	gl::color( ColorAf::white() );
	gl::enableVerticalSync();
}

void ImageApp::draw()
{
	const gl::ScopedViewport scopedViewport( ivec2( 0 ), getWindowSize() );
	const gl::ScopedMatrices scopedMatrices;
	gl::clear( Colorf::white() );
	gl::setMatricesWindow( getWindowSize() );
	
	const Leap::ImageList& images = mFrame.images();
	int32_t count	= images.count();
	if ( count > 0 ) {
		Rectf bounds	= Rectf( getWindowBounds() ) / (float)count;
		float x			= 0.0f;
		const float y	= getWindowCenter().y - bounds.getHeight() * 0.5f;
		int32_t i		= 0;
		for ( Leap::ImageList::const_iterator iter = images.begin(); iter != images.end(); ++iter, ++i ) {
			const Leap::Image& img	= *iter;
			Channel8uRef channel	= LeapMotion::toChannel8u( img );
			if ( channel ) {
				const gl::ScopedModelMatrix scopedModelMatrix;
				gl::translate( x, y );
				const gl::Texture2dRef tex = gl::Texture::create( *channel );
				gl::draw( tex, tex->getBounds(), bounds );
			}
			x += bounds.getWidth();
		}
	}

	mParams->draw();
}

void ImageApp::screenShot()
{
	writeImage( getAppPath() / fs::path( "frame" + toString( getElapsedFrames() ) + ".png" ), copyWindowSurface() );
}

void ImageApp::update()
{
	mFrameRate = getAverageFps();

	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
	}
}

CINDER_APP( ImageApp, RendererGl, []( App::Settings* settings )
{
	settings->setWindowSize( 1024, 768 );
	settings->disableFrameRate();
} )
