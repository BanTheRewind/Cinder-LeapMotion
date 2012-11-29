/*
* 
* Copyright (c) 2012, Ban the Rewind
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

#include "Leap.h"
#include "boost/signals2.hpp"
#include "cinder/Thread.h"
#include "cinder/Vector.h"

namespace LeapSdk {

typedef boost::signals2::connection				Callback;
typedef std::shared_ptr<Callback>				CallbackRef;
typedef std::map<uint32_t, CallbackRef>			CallbackList;
typedef std::shared_ptr<class Device>			DeviceRef;
typedef std::map<int32_t, class Hand>			HandMap;
typedef std::map<int32_t, class Finger>			FingerMap;
class Listener;

//////////////////////////////////////////////////////////////////////////////////////////////

class Finger
{
public:
	Finger( const ci::Vec3f& position = ci::Vec3f::zero(), const ci::Vec3f& direction = ci::Vec3f::zero(),
		   const ci::Vec3f& velocity = ci::Vec3f::zero(), float length = 0.0f, float width = 0.0f,
		   bool isTool = false );
	
	const ci::Vec3f&		getDirection();
	const ci::Vec3f&		getDirection() const;
	float					getLength();
	float					getLength() const;
	const ci::Vec3f&		getPosition();
	const ci::Vec3f&		getPosition() const;
	const ci::Vec3f&		getVelocity();
	const ci::Vec3f&		getVelocity() const;
	float					getWidth();
	float					getWidth() const;
	
	bool					isTool();
	bool					isTool() const;
private:
	ci::Vec3f				mDirection;
	bool					mIsTool;
	float					mLength;
	ci::Vec3f				mPosition;
	ci::Vec3f				mVelocity;
	float					mWidth;

	friend class			Listener;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class Hand 
{
public:
	Hand( const FingerMap& fingerMap = FingerMap(), const ci::Vec3f& position = ci::Vec3f::zero(),
		 const ci::Vec3f& direction = ci::Vec3f::zero(), const ci::Vec3f& velocity = ci::Vec3f::zero(),
		 const ci::Vec3f& normal = ci::Vec3f::zero(), const ci::Vec3f& ballPosition = ci::Vec3f::zero(),
		 float ballRadius = 0.0f );
	~Hand();

	const ci::Vec3f&		getBallPosition();
	const ci::Vec3f&		getBallPosition() const;
	float					getBallRadius();
	float					getBallRadius() const;
	const ci::Vec3f&		getDirection();
	const ci::Vec3f&		getDirection() const;
	const FingerMap&		getFingers();
	const FingerMap&		getFingers() const;
	const ci::Vec3f&		getNormal();
	const ci::Vec3f&		getNormal() const;
	const ci::Vec3f&		getPosition();
	const ci::Vec3f&		getPosition() const;
	const ci::Vec3f&		getVelocity();
	const ci::Vec3f&		getVelocity() const;
private:
	ci::Vec3f				mBallPosition;
	float					mBallRadius;
	ci::Vec3f				mDirection;
	FingerMap				mFingers;
	ci::Vec3f				mNormal;
	ci::Vec3f				mPosition;
	ci::Vec3f				mVelocity;

	friend class			Listener;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class Frame
{
public:
	~Frame();

	const HandMap&			getHands();
	const HandMap&			getHands() const;
	int64_t					getId();
	int64_t					getId() const;
	int64_t					getTimestamp();
	int64_t					getTimestamp() const;
private:
	Frame( const HandMap& handMap = HandMap(), int64_t id = 0, int64_t timestamp = 0 );
	HandMap					mHands;
	int64_t					mId;
	int64_t					mTimestamp;

	friend class			Listener;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class Listener : public Leap::Listener
{
protected:
	Listener();
    virtual void			onConnect( const Leap::Controller& controller );
    virtual void			onDisconnect( const Leap::Controller& controller );
    virtual void			onFrame( const Leap::Controller& controller );
	virtual void			onInit( const Leap::Controller& controller );

	volatile bool			mConnected;
	volatile bool			mInitialized;
	volatile bool			mNewFrame;

	Frame					mFrame;

	friend class			Device;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class Device
{
public:
	static DeviceRef		create();
	~Device();
	
	void					start();
	void					stop();
	void					update();

	bool					isConnected();
	bool					isConnected() const;
	bool					isInitialized();
	bool					isInitialized() const;

	template<typename T, typename Y> 
	inline uint32_t			addCallback( T callback, Y *callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignal.connect( std::bind( callback, callbackObject, std::placeholders::_1 ) ) ) ) ) );
		return id;
	}
	void					removeCallback( uint32_t id );
private:
	Device();

	CallbackList							mCallbacks;
	boost::signals2::signal<void ( Frame )>	mSignal;

	typedef std::shared_ptr<Leap::Controller> ControllerRef;
	
	ControllerRef			mController;
	Listener*				mListener;
	std::mutex				mMutex;
};

}
