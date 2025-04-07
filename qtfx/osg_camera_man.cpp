#include "osg_camera_man.h"
#include "vis-osg/osg_tools.h"
#include "xo/system/log.h"

using namespace osg;
using namespace xo::angle_literals;

namespace vis
{
	constexpr double pitch_scale = 100;
	constexpr double yaw_scale = 100;
	constexpr double pan_scale = 0.3;
	constexpr double zoom_scale = 1.0;
	constexpr degree key_orbit = 5_degf;
	constexpr double key_pan = 0.05;
	constexpr double key_zoom = 0.05;

	osg_camera_man::osg_camera_man() :
		osgGA::OrbitManipulator(),
		prev_camera_state_(),
		animationMode_( false ),
		playbackMode_( false ),
		enableCameraManipulation_( true ),
		animationTime_( 0.0 ),
		yawAnimationVelocity( 0 ),
		pitchAnimationVelocity( 0 ),
		dollyAnimationVelocity( 0 )
	{
		setAllowThrow( false );
		_minimumDistance = 0.001;

		setCameraState( camera_state::default_state() );

		osg::Vec3d eye, center, up;
		getTransformation( eye, center, up );
		setHomePosition( eye, center, up );
	}

	osg_camera_man::~osg_camera_man() {}

	void osg_camera_man::setCameraState( const camera_state& s )
	{
		orbit_pitch = s.pitch;
		orbit_yaw = s.yaw;
		_center = to_osg( focus_point_ + s.center_offset );
		_distance = s.distance;
		updateRotation();
	}

	camera_state osg_camera_man::getCameraState()
	{
		return camera_state{ orbit_pitch, orbit_yaw, from_osg( _center ) - focus_point_, float( _distance ) };
	}

	void osg_camera_man::setTransition( const camera_state& s )
	{
		if ( playbackMode_ ) {
			transition_.clear();
			transition_.insert( animationTime_, getCameraState() );
			transition_.insert( animationTime_ + transitionDuration_, s );
		}
		else setCameraState( s );
	}

	void osg_camera_man::setFocusPoint( const vec3f& p )
	{
		auto center_offset = _center - to_osg( focus_point_ );
		focus_point_ = p;
		setCenter( to_osg( focus_point_ ) + center_offset );
	}

	void osg_camera_man::setCameraPosition( const vec3f& p )
	{
		auto v = from_osg( _center ) - p;
		_distance = xo::length( v );
		orbit_yaw = degree( radianf( -std::atan2( v.x, -v.z ) ) );
		orbit_pitch = degree( radianf( std::atan2( v.y, std::sqrt( v.x * v.x + v.z * v.z ) ) ) );
		updateRotation();
	}

	vec3f osg_camera_man::getCameraPosition()
	{
		return from_osg( _center - _rotation * osg::Vec3d( 0.0, 0.0, -_distance ) );
	}

	void osg_camera_man::setOrbitAnimation( degree yps, degree pps, float dps )
	{
		yawAnimationVelocity = yps;
		pitchAnimationVelocity = pps;
		dollyAnimationVelocity = dps;
	}

	bool osg_camera_man::hasCameraStateChanged()
	{
		auto cs = getCameraState();
		if ( cs != prev_camera_state_ )
		{
			prev_camera_state_ = cs;
			return true;
		}
		else return false;
	}

	void osg_camera_man::handleKeyboardAnimation()
	{
		if ( animationMode_ )
		{
			// #todo: use delta time instead of 0.25
			for ( const auto& [key, time] : key_state_ )
				handleKeyCommand( key, mod_key_state_, 0.25 );
		}
	}

	void osg_camera_man::handleAnimation( double t, float dt )
	{
		if ( yawAnimationVelocity.value != 0 || pitchAnimationVelocity.value != 0 )
			orbitModel( dt * yawAnimationVelocity, dt * pitchAnimationVelocity );
		if ( dollyAnimationVelocity != 0 )
			dollyModel( dt * dollyAnimationVelocity );
		if ( !transition_.empty() ) {
			setCameraState( transition_.get( t ) );
			if ( !transition_.active( t ) )
				transition_.clear(); // clear transition when finished
		}
		animationTime_ = t;
	}

	void osg_camera_man::updateRotation()
	{
		auto yaw = osg::Quat( orbit_yaw.rad_value(), osg::Vec3d( 0, 1, 0 ) );
		auto pitch = osg::Quat( orbit_pitch.rad_value(), osg::Vec3d( 1, 0, 0 ) );
		_rotation = pitch * yaw;
	}

	bool osg_camera_man::performMovementLeftMouseButton( const double eventTimeDelta, const double dx, const double dy )
	{
		if ( enableCameraManipulation_ ) {
			orbit_pitch += degree( pitch_scale * dy );
			orbit_yaw -= degree( yaw_scale * dx );
			updateRotation();
		}
		return true;
	}

	bool osg_camera_man::performMovementMiddleMouseButton( const double eventTimeDelta, const double dx, const double dy )
	{
		if ( enableCameraManipulation_ )
			zoomModel( dy * zoom_scale, false );
		return true;
	}

	bool osg_camera_man::performMovementRightMouseButton( const double eventTimeDelta, const double dx, const double dy )
	{
		if ( enableCameraManipulation_ ) {
			float scale = -pan_scale * _distance;
			panModel( dx * scale, dy * scale );
		}
		return true;
	}

	bool osg_camera_man::handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
	{
		mod_key_state_ = ea.getModKeyMask();
		if ( !key_state_.contains( ea.getKey() ) )
			key_state_[ea.getKey()] = ea.getTime();

		// handle 'normal' keys or when only shift is down
		if ( ( ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT ) == ea.getModKeyMask() )
		{
			switch ( ea.getKey() )
			{
			case 'r': setTransition( camera_state::default_state() ); return true;
			case 'x': setTransition( camera_state::front_plane() ); return true;
			case 'y': setTransition( camera_state::top_plane() ); return true;
			case 'z': setTransition( camera_state::right_plane() ); return true;
			case 'X': setTransition( camera_state::back_plane() ); return true;
			case 'Y': setTransition( camera_state::bottom_plane() ); return true;
			case 'Z': setTransition( camera_state::left_plane() ); return true;
			case osgGA::GUIEventAdapter::KEY_Space: return false; // filter out space key because we don't want it to reset the camera
			}
		}

		// handle camera keys
		if ( ea.getKey() >= '1' && ea.getKey() <= '9' ) {
			index_t idx = ea.getKey() - '1';
			if ( ea.getModKeyMask() == 0 && idx < cameras_.size() ) {
				setTransition( cameras_[idx] );
				xo::log::info( "Loaded camera ", idx + 1 );
			}
			else if ( ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL ) {
				cameras_.resize( std::max( idx + 1, cameras_.size() ) );
				cameras_[idx] = getCameraState();
				xo::log::info( "Saved camera ", idx + 1 );
			}
		}

		if ( !animationMode_ )
		{
			if ( handleKeyCommand( ea.getKey(), ea.getModKeyMask() ) )
				return true;
		}

		return osgGA::OrbitManipulator::handleKeyDown( ea, us );
	}

	bool osg_camera_man::handleKeyUp( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
	{
		key_state_.erase( ea.getKey() );
		mod_key_state_ = ea.getModKeyMask();

		return osgGA::OrbitManipulator::handleKeyUp( ea, us );
	}

	bool osg_camera_man::handleKeyCommand( int key, int mod_keys, double factor )
	{
		if ( mod_keys & osgGA::GUIEventAdapter::MODKEY_SHIFT )
		{
			switch ( key )
			{
			case osgGA::GUIEventAdapter::KEY_Left: panModel( -key_pan * factor, 0 ); return true;
			case osgGA::GUIEventAdapter::KEY_Right: panModel( key_pan * factor, 0 ); return true;
			case osgGA::GUIEventAdapter::KEY_Up: panModel( 0, key_pan * factor ); return true;
			case osgGA::GUIEventAdapter::KEY_Down: panModel( 0, -key_pan * factor ); return true;
			default: return false;
			}
		}
		else if ( mod_keys == 0 )
		{
			switch ( key )
			{
			case osgGA::GUIEventAdapter::KEY_Left: orbitModel( -key_orbit * factor, 0_degf ); return true;
			case osgGA::GUIEventAdapter::KEY_Right: orbitModel( key_orbit * factor, 0_degf ); return true;
			case osgGA::GUIEventAdapter::KEY_Up: zoomModel( -key_zoom * factor ); return true;
			case osgGA::GUIEventAdapter::KEY_Down: zoomModel( key_zoom * factor ); return true;
			case osgGA::GUIEventAdapter::KEY_Page_Up: orbitModel( 0_degf, -key_orbit * factor ); return true;
			case osgGA::GUIEventAdapter::KEY_Page_Down: orbitModel( 0_degf, key_orbit * factor ); return true;
			default: return false;
			}
		}
		else return false;
	}

	bool osg_camera_man::handleMousePush( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
	{
		return osgGA::OrbitManipulator::handleMousePush( ea, us );
	}

	bool osg_camera_man::handleMouseRelease( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
	{
		return osgGA::OrbitManipulator::handleMouseRelease( ea, us );
	}
}
