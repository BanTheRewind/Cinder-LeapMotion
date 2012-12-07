/******************************************************************************\
* Copyright (C) 2012 Leap Motion, Inc. All rights reserved.                    *
* NOTICE: This developer release of Leap Motion, Inc. software is confidential *
* and intended for very limited distribution. Parties using this software must *
* accept the SDK Agreement prior to obtaining this software and related tools. *
* This software is subject to copyright.                                       *
\******************************************************************************/

#if !defined(__Leap_h__)
#define __Leap_h__

#include <string>
#include <vector>

// Define Leap export macros
#if defined(_WIN32) // Windows
#if LEAP_API_INTERNAL
#define LEAP_EXPORT
#elif LEAP_API_IMPLEMENTATION
#define LEAP_EXPORT __declspec(dllexport)
#else
#define LEAP_EXPORT __declspec(dllimport)
#endif
#define LEAP_EXPORT_CLASS
#define LEAP_EXPORT_PLUGIN __declspec(dllexport)
#elif defined(__APPLE__) // Mac OS
#define LEAP_EXPORT __attribute__((visibility("default")))
#define LEAP_EXPORT_CLASS __attribute__((visibility("default")))
#define LEAP_EXPORT_PLUGIN __attribute__((visibility("default")))
#else // GNU/Linux
#define LEAP_EXPORT
#define LEAP_EXPORT_CLASS
#define LEAP_EXPORT_PLUGIN
#endif

// Define integer types for Visual Studio 2005
#if defined(_MSC_VER) && (_MSC_VER < 1600)
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

namespace Leap {

//
// Private Interface
//

class LEAP_EXPORT_CLASS Interface {
  public:
    struct Implementation {
      LEAP_EXPORT virtual ~Implementation() {}
    };
  protected:
    LEAP_EXPORT Interface(Implementation* pImpl = 0);
    LEAP_EXPORT Interface(const Interface& rhs);
    LEAP_EXPORT Interface& operator=(const Interface& rhs);
    LEAP_EXPORT virtual ~Interface();
    template<typename T> T* get() const { return dynamic_cast<T*>(impl()); }
  private:
    LEAP_EXPORT Implementation* impl() const;
    class SharedObject* m_object;
};

class FingerImplementation;
class HandImplementation;
class FrameImplementation;
class ControllerImplementation;
class Listener;

//
// Public Interface
//

/// The Vector struct represents a three-component mathematical vector or point
/// such as a direction or position in three-dimensional space.
///
/// The Leap software employs a right-handed Cartesian coordinate system.
/// Values given are in units of real-world millimeters. The origin is centered
/// at the center of the Leap device. The x- and z-axes lie in the horizontal
/// plane, with the x-axis running parallel to the long edge of the device.
/// The y-axis is vertical, with positive values increasing upwards (in contrast
/// to the downward orientation of most computer graphics coordinate systems).
/// The z-axis has positive values increasing away from the computer screen.
struct Vector {
  /// Creates a new Vector with all components set to zero.
  Vector() :
    x(0), y(0), z(0) {}
  /// Creates a new Vector with the specified component values.
  Vector(double _x, double _y, double _z) :
    x(_x), y(_y), z(_z) {}
  /// Copies the specified Vector.
  Vector(const Vector& _vector) :
    x(_vector.x), y(_vector.y), z(_vector.z) {}

  /// The horizontal component.
  double x;
  /// The vertical component.
  double y;
  /// The depth component.
  double z;
};

/// The Ray struct represents a three-dimensional physical vector defined by a
/// position and a direction.
struct Ray {
  /// Creates a Ray with the specified position and direction vectors.
  Ray(const Vector& _position, const Vector& _direction) :
    position(_position), direction(_direction) {}
  /// Creates a Ray with the specified coordinates for position and direction.
  Ray(double _position_x, double _position_y, double _position_z,
      double _direction_x, double _direction_y, double _direction_z) :
    position(_position_x, _position_y, _position_z),
    direction(_direction_x, _direction_y, _direction_z) {}

  /// The position of the vector within the Leap field of view, in millimeters.
  Vector position;
  /// The normalized unit vector expressing the direction of the vector.
  Vector direction;
};

/// The Ball struct represents a three-dimensional sphere, defined by a
/// position and a radius.
struct Ball {
  /// Creates a Ball instance at the origin with a radius of zero.
  Ball() :
    position(0, 0, 0), radius(0) {}
  /// Creates a Ball instance with the specified position vector and radius.
  Ball(const Vector& _position, double _radius) :
    position(_position), radius(_radius) {}
  /// Creates a Ball instance with the specified position coordinates and radius.
  Ball(double _x, double _y, double _z, double _radius) :
    position(_x, _y, _z), radius(_radius) {}
  /// Creates a copy of the specified Ball instance.
  Ball(const Ball& _sphere) :
    position(_sphere.position), radius(_sphere.radius) {}

  /// The position of this sphere, in millimeters from the origin of the Leap
  /// coordinate system.
  Vector position;

  /// The radius of this sphere, in millimeters.
  double radius;
};

/// The Finger class reports the physical characteristics of a detected finger or tool.
///
/// Both fingers and tools are classified as Finger objects. Use the Finger::isTool() function
/// to determine whether a Finger object represents a finger or tool. The Leap classifies a
/// detected entity as a tool when it is thinner, straighter, and longer than a typical finger.
///
/// The tracking data includes a tip Ray, a length, and a velocity. The Ray includes
/// a position vector in millimeters (from the center of the Leap device) and a unit
/// vector indicating the direction in which the finger is pointing.
///
/// Get Finger objects from the fingers array of a Hand object. Get Hand objects
/// from the hands array of a Frame object.
class Finger : public Interface {
  public:
    /// Creates a Finger object.
    ///
    /// You do not need to create your own Finger instances. Get Finger instances
    /// representing tracked fingers or tools from a Hand object in the hands array of a Frame.
    Finger(const FingerImplementation&);
#if defined(SWIG)
    Finger() { throw std::logic_error("Finger default constructor should not be used"); }
#endif

    /// A unique ID assigned to this Finger object, whose value remains the same across
    /// consecutive frames while the finger remains visible. If tracking is lost
    /// (for example, when the finger is occluded by another finger or a hand),
    /// the Leap assigns a new ID when it detects the finger in a future frame.
    LEAP_EXPORT int32_t id() const;

    /// A Ray expressing the position of the tip and the direction in
    /// which the finger or tool is pointing.
    LEAP_EXPORT const Ray& tip() const;

    /// The rate of change of the tip position in millimeters per second.
    /// If velocity is not available, then the function returns a null pointer.
    LEAP_EXPORT const Vector* velocity() const;

    /// The estimated width of the finger or tool in millimeters. 
    /// 
    /// The reported width is the average width of the visible portion of the tool 
    /// from the hand to the tip. If the width isn't known, then a value of 0 is returned.
    LEAP_EXPORT double width() const;

    /// The estimated length of the finger or tool in millimeters. 
    ///
    /// The reported length is the visible length of the tool from the hand to 
    /// tip. If the length isn't known, then a value of 0 is returned.
    LEAP_EXPORT double length() const;

    /// Whether or not the finger is believed to be a tool. Tools are generally
    /// longer, thinner, and straighter than fingers.
    LEAP_EXPORT bool isTool() const;
};

/// The Hand class reports the physical characteristics of a detected hand.
///
/// Hand tracking data includes a palm Ray, a velocity, a sphere fit to the hand,
/// and an array containing fingers and tools.
/// The palm Ray includes a vector reporting the position of the center of the hand
/// in millimeters (from the center of the Leap device) and a unit
/// vector indicating the direction from the palm point to the wrist.
///
/// A Hand object always contains at least one finger or a palm Ray. You will never
/// receive a Hand object with both an empty finger list and a NULL palm ray.
///
/// Get Hand objects from the hands array of a Frame object.
class Hand : public Interface {
  public:
    /// Creates a Hand object.
    ///
    /// You do not need to create your own Hand instances. Get Hand instances
    /// representing tracked hands from the hands array of a frame.
    Hand(const HandImplementation&);
#if defined(SWIG)
    Hand() { throw std::logic_error("Hand default constructor should not be used"); }
#endif

    /// A unique ID assigned to this Hand, whose value remains the same across
    /// consecutive frames while the hand remains visible. If tracking is lost
    /// (for example, when the hand is withdrawn from the Leap field of view),
    /// the Leap assigns a new ID when it detects the hand in a future frame.
    LEAP_EXPORT int32_t id() const;

    /// The list of fingers and tools detected in this frame that are attached to this
    /// hand. The fingers in this hand are given in arbitrary order.
    /// The list can be empty if no fingers or tools are detected.
    ///
    /// Use the Finger::isTool() function to determine whether an item in the list
    /// represents a finger or a tool.
    LEAP_EXPORT const std::vector<Finger>& fingers() const;

    /// A pointer to a Ray expressing the hand's palm position and orientation.
    /// The position is the center of the palm.
    /// The direction vector points from the center of the palm to the wrist.
    /// If the palm is not available, then the function returns a null pointer.
    LEAP_EXPORT const Ray* palm() const;

    /// The rate of change of the palm position in millimeters per second.
    /// If velocity is not available, then the function returns a null pointer.
    LEAP_EXPORT const Vector* velocity() const;

    /// The normal vector to the palm. If your hand is flat, this vector will
    /// point downward, or "out" of your palm. If the normal is not available,
    /// the function returns a null pointer;
    LEAP_EXPORT const Vector* normal() const;

    /// A sphere fit to the curvature of this hand. This sphere is placed roughly
    /// as if the hand were holding a ball. Thus the size of the ball decreases as
    /// the fingers are curled into a fist. If a sphere fit is not available,
    /// then the function returns a null pointer.
    LEAP_EXPORT const Ball* ball() const;
};

/// The Frame class represents a set of hand and finger tracking data detected
/// in a single frame.
///
/// The Leap detects hands, fingers and finger-like tools within the tracking area,
/// reporting their positions and orientations in frames generated at the Leap frame rate.
///
/// Access Frame objects through an instance of a Leap Controller. Implement a
/// Listener subclass to receive a callback event when a new Frame is available.
class Frame : public Interface {
  public:
    /// Creates a Frame object.
    ///
    /// You do not need to create your own Frame instances. Get Frame instances
    /// containing hand tracking data from the Controller::frame() method.
    Frame(const FrameImplementation&);
    LEAP_EXPORT Frame();

    /// A unique ID for this Frame. Consecutive frames processed by the Leap
    /// have consecutive increasing values.
    LEAP_EXPORT int64_t id() const;

    /// The frame capture time in microseconds elapsed since the Leap started.
    LEAP_EXPORT int64_t timestamp() const;

    /// The list of hands detected in this frame, given in arbitrary order.
    LEAP_EXPORT const std::vector<Hand>& hands() const;
};

/// The Config class provides access to Leap system configuration information.
///
/// The keys necessary to obtain configuration values are not yet documented.
class Config : public Interface {
  public:
    LEAP_EXPORT Config();

    /// Enumerates the possible data types for configuration values.
    ///
    /// The Config::type() method returns an item from the ValueType enumeration.
    enum ValueType {
      TYPE_UNKNOWN, ///< The data type is unknown.
      TYPE_BOOLEAN, ///< A boolean value.
      TYPE_INT32,   ///< A 32-bit integer.
      TYPE_INT64,   ///< A 64-bit integer.
      TYPE_UINT32,  ///< A 32-bit unsigned integer.
      TYPE_UINT64,  ///< A 64-bit unsigned integer.
      TYPE_FLOAT,   ///< A floating-point number.
      TYPE_DOUBLE,  ///< A double precision floating-point number.
      TYPE_STRING   ///< A string of characters.
    };

    /// Reports the natural data type for the value related to the specified key.
    ///
    /// @param key The key for the looking up the value in the configuration dictionary.
    /// @returns The native data type of the value, that is, the type that does not
    /// require a data conversion.
    LEAP_EXPORT ValueType type(const std::string& key) const;

    /// Reports whether the value is an array of homogeneous objects. The type
    /// of the objects in the array is returned using the Config::type() method.
    LEAP_EXPORT bool isArray(const std::string& key) const;

    /// Gets the boolean representation for the specified key.
    LEAP_EXPORT bool getBool(const std::string& key) const;

    /// Gets the 32-bit integer representation for the specified key.
    LEAP_EXPORT int32_t getInt32(const std::string& key) const;

    /// Gets the 64-bit integer representation for the specified key.
    LEAP_EXPORT int64_t getInt64(const std::string& key) const;

    /// Gets the unsigned 32-bit integer representation for the specified key.
    LEAP_EXPORT uint32_t getUInt32(const std::string& key) const;

    /// Gets the unsigned 64-bit integer representation for the specified key.
    LEAP_EXPORT uint64_t getUInt64(const std::string& key) const;

    /// Gets the floating point representation for the specified key.
    LEAP_EXPORT float getFloat(const std::string& key) const;

    /// Gets the double precision representation for the specified key.
    LEAP_EXPORT double getDouble(const std::string& key) const;

    /// Gets the string representation for the specified key.
    LEAP_EXPORT std::string getString(const std::string& key) const;

    /// Gets the boolean array representation for the specified key.
    LEAP_EXPORT std::vector<bool> getBoolArray(const std::string& key) const;

    /// Gets the 32-bit integer array representation for the specified key.
    LEAP_EXPORT std::vector<int32_t> getInt32Array(const std::string& key) const;

    /// Gets the 64-bit integer array representation for the specified key.
    LEAP_EXPORT std::vector<int64_t> getInt64Array(const std::string& key) const;

    /// Gets the unsigned 32-bit integer array representation for the specified key.
    LEAP_EXPORT std::vector<uint32_t> getUInt32Array(const std::string& key) const;

    /// Gets the unsigned 64-bit integer array representation for the specified key.
    LEAP_EXPORT std::vector<uint64_t> getUInt64Array(const std::string& key) const;

    /// Gets the floating point array representation for the specified key.
    LEAP_EXPORT std::vector<float> getFloatArray(const std::string& key) const;

    /// Gets the double precision array representation for the specified key.
    LEAP_EXPORT std::vector<double> getDoubleArray(const std::string& key) const;

    /// Gets the string array representation for the specified key.
    LEAP_EXPORT std::vector<std::string> getStringArray(const std::string& key) const;
};

/// The Controller class is your main interface to the Leap device.
///
/// Create an instance of this Controller class to access Leap motion tracking
/// data and configuration information. Frame data can be polled at any time
/// using the Controller::frame() method. Call frame() or frame(0) to get the
/// most recent frame. Set the history parameter to a positive integer to access
/// previous frames. A controller stores up to 60 frames in its frame history.
///
/// Polling is an appropriate strategy for applications which already have an
/// intrinsic update loop, such as a game. You can also assign an instance of a 
/// Listener subclass to the controller to handle events as they occur. The Leap 
/// dispatches events to the listener on initialization, on connection changes, 
/// and when a new frame of motion tracking data is available. When these events 
/// occur, the controller object invokes the appropriate callback function defined 
/// in your Listener subclass.
///
/// To access frames of motion tracking data as they become available:
/// 1. Implement a subclass of the Listener class and override the
///    Listener::onFrame() method.
/// 2. In your Listener::onFrame() method, call the Controller::frame() method
///    to access the newest frame of hand tracking data.
/// 3. To start receiving frames, create a Controller object, passing a pointer
///    to an instance of the Listener subclass to the Controller constructor.
///
/// When you create a Controller object with an instance of a Listener subclass,
/// it calls the Listener::onInit() method when the controller is initialized.
/// The controller then attempts to connect to the Leap software. When the
/// connection is established, the controller calls the Listener::onConnect()
/// method. At this point, your application will start receiving frames of data.
/// The controller calls the Listener::onFrame() method each time a new frame is
/// available. If the controller loses its connection with the Leap software or
/// device for any reason, it calls the Listener::onDisconnect() method.
class Controller : public Interface {
  public:
#if !defined(SWIG)
    Controller(const ControllerImplementation&);
#endif
    /// Creates a Controller instance.
    ///
    /// When creating a Controller object, you may optionally pass in a
    /// reference to an instance of the Listener class. The controller calls
    /// the methods in this listener object when Leap events occur.
    ///
    /// @param listener An instance of a subclass of Leap::Listener implementing
    /// the callback functions for events you want to handle in your application.
    LEAP_EXPORT Controller(Listener* listener = 0);
    LEAP_EXPORT virtual ~Controller();

    /// Returns a frame of motion tracking data from the Leap. Use the optional
    /// history parameter to specify which frame to retrieve. Call frame() or
    /// frame(0) to access the most recent frame; call frame(1) to access the
    /// previous frame, and so on. If you use a history value greater than the
    /// number of stored frames, then the controller returns an empty frame.
    ///
    /// @param history The Frame to return, counting backwards in sequence.
    /// @returns The specified frame; or, if no history parameter is specified,
    /// the newest frame. If an invalid history value is given, an empty frame.
    LEAP_EXPORT Frame frame(int history = 0) const;

    /// Returns a Config object, which you can use to query the Leap system for
    /// configuration information.
    LEAP_EXPORT Config config() const;

    /// Returns a pointer to the Listener instance associated with this
    /// controller, or a null pointer if no listener is associated.
    LEAP_EXPORT Listener* listener() const;
};

/// The Listener class defines a set of callback functions that you can
/// override in a subclass to respond to events dispatched by the Leap.
///
/// To handle Leap events, create an instance of a Listener subclass and assign
/// it to the Controller instance. The Controller calls the relevant Listener
/// callback function when an event occurs, passing in a reference to itself.
/// You do not have to implement callbacks for events you do not want to handle.
///
/// When using a listener, make sure that the Listener instance outlives all
/// instances of Controller objects to which it has been assigned as a listener.
/// You can assign a Listener instance to more than one Controller object.
class Listener {
  public:
    LEAP_EXPORT virtual ~Listener() {}

    /// Called once, when the Controller object is fully initialized.
    /// @param controller The Controller object invoking this callback function.
    LEAP_EXPORT virtual void onInit(const Controller&) {}

    /// Called when the Controller object connects to the Leap software.
    /// @param controller The Controller object invoking this callback function.
    LEAP_EXPORT virtual void onConnect(const Controller&) {}

    /// Called when the Controller object disconnects from the Leap software.
    /// The controller can disconnect when the Leap device is unplugged, the
    /// user shuts the Leap software down, or the Leap software encounters an
    /// unrecoverable error.
    /// @param controller The Controller object invoking this callback function.
    LEAP_EXPORT virtual void onDisconnect(const Controller&) {}

    /// Called when a new frame of hand and finger tracking data is available.
    /// Access the new frame data using the Controller::frame() method.
    ///
    /// Note, the Controller skips any pending onFrame events while your
    /// onFrame handler executes. If your implementation takes too long to return,
    /// one or more frames can be skipped. The Controller still inserts the skipped
    /// frames into the frame history. You can access recent frames by setting
    /// the history parameter when calling the Controller::frame() method.
    ///
    /// @param controller The Controller object invoking this callback function.
    LEAP_EXPORT virtual void onFrame(const Controller&) {}
};

}

#endif // __Leap_h__
