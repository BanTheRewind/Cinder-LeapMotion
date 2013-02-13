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

#pragma once

#include "Leap.h"
#include "boost/signals2.hpp"
#include "cinder/Matrix.h"
#include "cinder/Thread.h"
#include "cinder/Vector.h"

namespace LeapSdk {
	
class Frame;
class Device;
class Listener;
class Screen;

//////////////////////////////////////////////////////////////////////////////////////////////

class Pointable
{
public:
	//! Returns normalized vector of pointing direction.
	ci::Vec3f		getDirection() const;
	//! Returns length in millimeters.
	float			getLength() const;
	//! Returns position vector in millimeters.
	ci::Vec3f		getPosition() const;
	//! Returns velocity vector in millimeters.
	ci::Vec3f		getVelocity() const;
	//! Returns width in millimeters.
	float			getWidth() const;
protected:
	Pointable();
	Pointable( const Leap::Pointable& pointable );
	Pointable( const Pointable& p );
	
	Leap::Pointable	mPointable;
	
	friend class	Listener;
	friend class	Screen;
};
	
class Finger : public Pointable
{
public:
	Finger();
	Finger( const Pointable& p );
private:
	friend class		Listener;
};
	
class Tool : public Pointable
{
public:
	Tool();
	Tool( const Pointable& p );
private:
	friend class		Listener;
};

typedef std::map<int32_t, Finger>	FingerMap;
typedef std::map<int32_t, Tool>		ToolMap;

//////////////////////////////////////////////////////////////////////////////////////////////

class Hand 
{
public:
	Hand();
	Hand( const Leap::Hand& hand, const FingerMap& fingerMap, const ToolMap& toolMap, float rotAngle,
		 const ci::Vec3f& rotAxis, const ci::Matrix44f& rotMatrix, float scale, const ci::Vec3f& translation );
	~Hand();

	//! Returns normalized vector of palm face direction.
	ci::Vec3f				getDirection() const;
	//! Returns map of fingers.
	const FingerMap&		getFingers() const;
	//! Returns normalized vector of palm face normal.
	ci::Vec3f				getNormal() const;
	//! Returns position vector of hand in millimeters.
	ci::Vec3f				getPosition() const;
	/*! The angle of rotation around the rotation axis derived from the
		change in orientation of this hand since the first frame. */
	float					getRotationAngle() const;
	/*! The angle of rotation around the rotation axis derived from the
	 change in orientation of this hand since the specified frame. */
	float					getRotationAngle( const Frame& frame ) const;
	/*! The rotation axis derived from the change in orientation of this
		hand since the first frame. */
	const ci::Vec3f&		getRotationAxis() const;
	/*! The rotation axis derived from the change in orientation of this
	 hand since the specified frame. */
	ci::Vec3f				getRotationAxis( const Frame& frame ) const;
	/*! The rotation derived from the change in orientation of this
	 hand since the first frame. */
	const ci::Matrix44f&	getRotationMatrix() const;
	/*! The rotation derived from the change in orientation of this
	 hand since the specified frame. */
	ci::Matrix44f			getRotationMatrix( const Frame& frame ) const;
	//! The scale difference since the first frame.
	float					getScale() const;
	//! The scale difference since the specified frame.
	float					getScale( const Frame& frame ) const;
	//! Returns position vector of hand sphere in millimeters.
	ci::Vec3f				getSpherePosition() const;
	//! Returns radius of hand sphere in millimeters.
	float					getSphereRadius() const;
	//! Returns map of tools.
	const ToolMap&			getTools() const;
	/*! The translation derived from the change of position of this
		hand since the previous frame. */
	const ci::Vec3f&		getTranslation() const;
	/*! The translation derived from the change of position of this
	 hand since the specified frame. */
	ci::Vec3f				getTranslation( const Frame& frame ) const;
	//! Returns velocity vector of hand in millimeters.
	ci::Vec3f				getVelocity() const;
private:
	FingerMap				mFingers;
	Leap::Hand				mHand;
	float					mRotationAngle;
	ci::Vec3f				mRotationAxis;
	ci::Matrix44f			mRotationMatrix;
	float					mScale;
	ToolMap					mTools;
	ci::Vec3f				mTranslation;
};

typedef std::map<int32_t, Hand> HandMap;

//////////////////////////////////////////////////////////////////////////////////////////////

class Frame
{
public:
	~Frame();

	//! Returns map of hands.
	const HandMap&	getHands() const;
	// Returns frame ID.
	int64_t			getId() const;
	// Return time stamp.
	int64_t			getTimestamp() const;
private:
	Frame();
	Frame( const Leap::Frame& frame, const HandMap& handMap );
	
	Leap::Frame		mFrame;
	HandMap			mHands;

	friend class	Hand;
	friend class	Listener;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class Screen
{
public:
	Screen();
	Screen( const Leap::Screen& screen );
	
	/*! The shortest distance from the specified point to the plane in which this
		screen lies. */
	float			distanceToPoint( const ci::Vec3f& v ) const;
	/*! A vector representing the bottom left corner of this screen within the
	 Leap coordinate system. */
	ci::Vec3f		getBottomLeft() const;
	//! A string containing a human readable description of the screen object.
	std::string		getDescription() const;
	//! Height of the screen in pixels.
	int32_t			getHeight() const;
	/*! A vector representing the horizontal axis of this screen within the
		Leap coordinate system. */
	ci::Vec3f		getHorizontalAxis() const;
	/// A vector normal to the plane in which this screen lies.
	ci::Vec3f		getNormal() const;
	//! Size of the screen in pixels.
	ci::Vec2i		getSize() const;
	/*! A vector representing the vertical axis of this screen within the
	 Leap coordinate system. */
	ci::Vec3f		getVerticalAxis() const;
	//! Width of the screen in pixels.
	int32_t			getWidth() const;
	/*! Returns the intersection between this screen and a ray projecting from a
		Pointable \a pointable object. Returns true if intersection occurs and sets
		the intersection to \a result. Set \a normalize to true for \a result to 
		represent a percentage of screen size. Default is false. \a clampRatio 
		adjusts the screen border. */
	bool			intersects( const Pointable& pointable, ci::Vec3f* result,
							   bool normalize = false, float clampRatio = 1.0f ) const;
private:
	Leap::Screen	mScreen;
};

typedef std::map<int32_t, Screen> ScreenMap;

//////////////////////////////////////////////////////////////////////////////////////////////

class Listener : public Leap::Listener
{
protected:
	Listener();
    virtual void	onConnect( const Leap::Controller& controller );
    virtual void	onDisconnect( const Leap::Controller& controller );
	virtual void	onExit( const Leap::Controller& controller );
    virtual void	onFrame( const Leap::Controller& controller );
	virtual void	onInit( const Leap::Controller& controller );

	volatile bool	mConnected;
	volatile bool	mExited;
	volatile bool	mFirstFrameReceived;
	volatile bool	mInitialized;
	std::mutex		*mMutex;
	volatile bool	mNewFrame;

	Frame			mFirstFrame;
	Frame			mFrame;

	friend class	Device;
};

//////////////////////////////////////////////////////////////////////////////////////////////

typedef std::shared_ptr<class Device> DeviceRef;

class Device
{
public:
	//! Creates and returns device instance.
	static DeviceRef	create();
	~Device();
	
	//! Must be called to trigger frame events.
	void				update();

	/*! Returns a LEAP::Config object, which you can use to query the Leap 
		system for configuration information. */
	Leap::Config		getConfig() const;
	//! Return map of calibrated screens.
	const ScreenMap&	getScreens() const;
	
	//! Returns true if the device has exited.
	bool				hasExited() const;
	//! Returns true if the device is connected.
	bool				isConnected() const;
	//! Returns true if LEAP application is initialized.
	bool				isInitialized() const;

	/*! Adds frame event callback. \a callback has the signature \a void(Frame). 
		\a callbackObject is the instance receiving the event. Returns callback ID. */
	template<typename T, typename Y> 
	inline uint32_t		addCallback( T callback, Y *callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignal.connect( std::bind( callback, callbackObject, std::placeholders::_1 ) ) ) ) ) );
		return id;
	}
	//! Remove callback by ID.
	void				removeCallback( uint32_t id );
private:
	Device();

	typedef boost::signals2::connection		Callback;
	typedef std::shared_ptr<Callback>		CallbackRef;
	typedef std::map<uint32_t, CallbackRef>	CallbackList;

	CallbackList							mCallbacks;
	boost::signals2::signal<void ( Frame )>	mSignal;

	Leap::Controller*	mController;
	Listener			mListener;
	std::mutex			mMutex;
	ScreenMap			mScreens;
};

ci::Matrix33f	toMatrix33f( const Leap::Matrix& m );
ci::Matrix44f	toMatrix44f( const Leap::Matrix& m );
ci::Vec3f		toVec3f( const Leap::Vector& v );
	
//////////////////////////////////////////////////////////////////////////////////////////////

}
