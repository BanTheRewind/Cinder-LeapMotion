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

#pragma once

#include "Leap.h"
#include "cinder/Channel.h"
#include "cinder/Matrix.h"
#include "cinder/Thread.h"
#include "cinder/Vector.h"
#include <functional>

namespace LeapMotion {

/*! Converts a native Leap image into a Cinder channel.
    Set \a copyData to true makes channel own data (slower). */
ci::Channel8u	toChannel8u( const Leap::Image& img, bool copyData = false );
//! Converts a native Leap 3x3 matrix into a Cinder one.
ci::mat3		toMat3( const Leap::Matrix& m );
//! Converts a native Leap 4x4 matrix into a Cinder one.
ci::mat4		toMat4( const Leap::Matrix& m );
//! Converts a Cinder 3x3 matrix into a native Leap one.
Leap::Matrix	toLeapMatrix( const ci::mat3& m );
//! Converts a Cinder 4x4 matrix into a native Leap one.
Leap::Matrix	toLeapMatrix( const ci::mat4 m );
//! Converts a Cinder vector into a native Leap one.
Leap::Vector	toLeapVector( const ci::vec3& v );
//! Converts a native Leap vector into a Cinder one.
ci::vec3		toVec3( const Leap::Vector& v );

//////////////////////////////////////////////////////////////////////////////////////////////

//! Receives and manages Leap controller data.
class Listener : public Leap::Listener
{
protected:
	Listener();

    virtual void	onConnect( const Leap::Controller& controller );
    virtual void	onDisconnect( const Leap::Controller& controller );
	virtual void	onExit( const Leap::Controller& controller );
    virtual void	onFrame( const Leap::Controller& controller );
	virtual void	onFocusGained( const Leap::Controller& controller );
	virtual void	onFocusLost( const Leap::Controller& controller );
	virtual void	onInit( const Leap::Controller& controller );
	
	volatile bool	mConnected;
	volatile bool	mExited;
	volatile bool	mFocused;
	volatile bool	mInitialized;
	std::mutex		*mMutex;
	volatile bool	mNewFrame;

	Leap::Frame		mFrame;

	friend class	Device;
};

//////////////////////////////////////////////////////////////////////////////////////////////

typedef std::shared_ptr<class Device> DeviceRef;
	
//! A class representing and managing a Leap device, controller and listener.
class Device
{
public:
	//! Creates and returns device instance.
	static DeviceRef	create();
	~Device();
	
	//! Returns LEAP controller associated with this device's listener.
	Leap::Controller*	getController() const;

	//! Returns true if app is focused for this device.
	virtual bool		hasFocus() const;
	//! Returns true if the device has exited.
	virtual bool		hasExited() const;
	//! Returns true if the device is connected.
	virtual bool		isConnected() const;
	//! Returns true if LEAP application is initialized.
	virtual bool		isInitialized() const;

	/*! Sets frame event handler. \a eventHandler has the signature \a void(Frame). 
		\a obj is the instance receiving the event. */
	template<typename T, typename Y> 
	inline void			connectEventHandler( T eventHandler, Y *obj )
	{
		connectEventHandler( std::bind( eventHandler, obj, std::placeholders::_1 ) );
	}
	
	//! Sets frame event callback to \a eventHandler.
	void				connectEventHandler( const std::function<void( Leap::Frame )>& eventHandler );
protected:
	Device();

	std::function<void ( Leap::Frame )>	mEventHandler;

	virtual void		update();

	Leap::Controller*	mController;
	Leap::Device		mDevice;
	Listener			mListener;
	std::mutex			mMutex;
};

}
