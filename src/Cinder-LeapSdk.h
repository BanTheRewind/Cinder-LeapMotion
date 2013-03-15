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
#include "cinder/Exception.h"
#include "cinder/Matrix.h"
#include "cinder/Thread.h"
#include "cinder/Vector.h"

namespace LeapSdk {

// Forward declarations
class Finger;
class Frame;
class Device;
class Hand;
class Listener;
class Pointable;
class Screen;
class Tool;

//////////////////////////////////////////////////////////////////////////////////////////////

//! Converts a native Leap finger into a LeapSdk one.
Finger			fromLeapFinger( const Leap::Finger& f );
//! Converts a LeapSdk finger into a native Leap one.
Leap::Finger	toLeapFinger( const Finger& f );
//! Converts a native Leap frame into a LeapSdk one.
Frame			fromLeapFrame( const Leap::Frame& f );
//! Converts a LeapSdk frame into a native Leap one.
Leap::Frame		toLeapFrame( const Frame& f );
//! Converts a native Leap hand into a LeapSdk one.
Hand			fromLeapHand( const Leap::Hand& h, const Leap::Frame& frame );
//! Converts a LeapSdk hand into a native Leap one.
Leap::Hand		toLeapHand( const Hand& h );
//! Converts a native Leap 3x3 matrix into a Cinder one.
ci::Matrix33f	fromLeapMatrix33( const Leap::Matrix& m );
//! Converts a Cinder 3x3 matrix into a native Leap one.
Leap::Matrix	toLeapMatrix33( const ci::Matrix33f& m );
//! Converts a native Leap 4x4 matrix into a Cinder one.
ci::Matrix44f	fromLeapMatrix44( const Leap::Matrix& m );
//! Converts a Cinder 4x4 matrix into a native Leap one.
Leap::Matrix	toLeapMatrix44( const ci::Matrix44f m );
//! Converts a native Leap pointable into a LeapSdk one.
Pointable		fromLeapPointable( const Leap::Pointable& p );
//! Converts a LeapSdk pointable into a native Leap one.
Leap::Pointable	toLeapPointable( const Pointable& p );
//! Converts a native Leap screen into a LeapSdk one.
Screen			fromLeapScreen( const Leap::Screen& s );
//! Converts a LeapSdk screen into a native Leap one.
Leap::Screen	toLeapScreen( const Screen& s );
//! Converts a native Leap tool into a LeapSdk one.
Tool			fromLeapTool( const Leap::Tool& t );
//! Converts a LeapSdk tool into a native Leap one.
Leap::Tool		toLeapTool( const Tool& t );
//! Converts a native Leap vector into a Cinder one.
ci::Vec3f		fromLeapVector( const Leap::Vector& v );
//! Converts a Cinder vector into a native Leap one.
Leap::Vector	toLeapVector( const ci::Vec3f& v );

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents a Leap pointable and its physical properties.
class Pointable
{
public:
	Pointable();
	
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
	Pointable( const Leap::Pointable& p );
	Pointable( const Pointable& p );
	
	Leap::Pointable	mPointable;
	
	friend class	Device;
	friend class	Hand;
	friend class	Listener;
	friend class	Screen;
	
	friend Finger			LeapSdk::fromLeapFinger( const Leap::Finger& f );
	friend Leap::Finger		LeapSdk::toLeapFinger( const Finger& f );
	friend Pointable		LeapSdk::fromLeapPointable( const Leap::Pointable& p );
	friend Leap::Pointable	LeapSdk::toLeapPointable( const Pointable& f );
	friend Tool				LeapSdk::fromLeapTool( const Leap::Tool& t );
	friend Leap::Tool		LeapSdk::toLeapTool( const Tool& f );
};
	
//! Represents a Leap finger pointable.
class Finger : public Pointable
{
public:
	Finger();
	Finger( const Pointable& p );
private:
	friend class		Listener;
};

//! Represents a Leap tool pointable.
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

//! Localizes Leap's gesture enumerators.
namespace Gesture {
	typedef Leap::Gesture::State	State;
	typedef Leap::Gesture::Type		Type;
}

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents a Leap hand physical data and a collection of pointables.
class Hand 
{
public:
	Hand();
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
	 change in orientation of this hand since the specified frame \a f. */
	float					getRotationAngle( const Frame& f ) const;
	/*! The rotation axis derived from the change in orientation of this
		hand since the first frame. */
	const ci::Vec3f&		getRotationAxis() const;
	/*! The rotation axis derived from the change in orientation of this
	 hand since the specified frame \a f. */
	ci::Vec3f				getRotationAxis( const Frame& f ) const;
	/*! The rotation derived from the change in orientation of this
	 hand since the first frame. */
	const ci::Matrix44f&	getRotationMatrix() const;
	/*! The rotation derived from the change in orientation of this
	 hand since the specified frame \a f. */
	ci::Matrix44f			getRotationMatrix( const Frame& f ) const;
	//! The scale difference since the first frame.
	float					getScale() const;
	//! The scale difference since the specified frame.
	float					getScale( const Frame& f ) const;
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
	 hand since the specified frame \a f. */
	ci::Vec3f				getTranslation( const Frame& f ) const;
	//! Returns velocity vector of hand in millimeters.
	ci::Vec3f				getVelocity() const;
private:
	Hand( const Leap::Hand& hand, const Leap::Frame& frame );

	FingerMap				mFingers;
	Leap::Hand				mHand;
	float					mRotationAngle;
	ci::Vec3f				mRotationAxis;
	ci::Matrix44f			mRotationMatrix;
	float					mScale;
	ToolMap					mTools;
	ci::Vec3f				mTranslation;
	
	friend class			Frame;
	
	friend Hand				fromLeapHand( const Leap::Hand& h, const Leap::Frame& f );
	friend Leap::Hand		toLeapHand( const Hand& h );
};

typedef std::map<int32_t, Hand> HandMap;

//////////////////////////////////////////////////////////////////////////////////////////////

//! Contains all available data in a Leap frame.
class Frame
{
public:
	Frame();
	~Frame();
	
	//! Returns vector of native Leap::Gesture objects.
	const std::vector<Leap::Gesture>&	getGestures() const;
	//! Returns map of hands.
	const HandMap&						getHands() const;
	// Returns frame ID.
	int64_t								getId() const;
	// Return time stamp.
	int64_t								getTimestamp() const;
private:
	Frame( const Leap::Frame& frame );
	
	Leap::Frame							mFrame;
	std::vector<Leap::Gesture>			mGestures;
	HandMap								mHands;
	
	friend class						Hand;
	friend class						Listener;
	
	friend Frame						LeapSdk::fromLeapFrame( const Leap::Frame& f );
	friend Leap::Frame					LeapSdk::toLeapFrame( const Frame& f );
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! Represents a Leap calibrated screen.
class Screen
{
public:
	Screen();
	
	/*! The shortest distance from the specified point \a v to the plane in which this
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
	bool			intersects( const Pointable& p, ci::Vec3f* result,
							   bool normalize = false, float clampRatio = 1.0f ) const;
private:
	Screen( const Leap::Screen& screen );
	
	Leap::Screen	mScreen;
	friend class	Device;
	
	friend Screen		LeapSdk::fromLeapScreen( const Leap::Screen& f );
	friend Leap::Screen	LeapSdk::toLeapScreen( const Screen& f );
};

typedef std::map<int32_t, Screen> ScreenMap;

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

//! A class representing and managing a Leap controller and listener.
class Device
{
public:
	//! Creates and returns device instance.
	static DeviceRef	create();
	~Device();
	
	//! Must be called to trigger frame events.
	void				update();

	//! Enable a specific type of gesture. 
	void				enableGesture( Gesture::Type t );
	
	//! Returns calibrated screen closest to Pointable \a p.
	const Screen&		getClosestScreen( const Pointable& p ) const;
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
	
//////////////////////////////////////////////////////////////////////////////////////////////

//! Base class for LeapSdk exceptions.
class Exception : public cinder::Exception
{
};

//! Exception expressing inability to locate a calibrated screen near a pointable.
class ExcNoClosestScreen : public Exception {
public:
	ExcNoClosestScreen() throw()
	{
	}
	
	virtual const char* what() const throw()
	{
		return "Unable to locate calibrated screen near pointable.";
	}
};

//! Exception expressing the absence of calibrated screens
class ExcNoCalibratedScreens : public Exception {
public:
	ExcNoCalibratedScreens() throw()
	{
	}
	
	virtual const char* what() const throw()
	{
		return "No calibrated screens are available.";
	}
};

}
