#include "osg_camera_man.h"
#include "vis-osg/osg_tools.h"

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
		animationMode_( false )
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
		_center = focus_point_ + to_osg( s.center_offset );
		_distance = s.distance;
		updateRotation();
	}

	camera_state osg_camera_man::getCameraState()
	{
		return camera_state{ orbit_pitch, orbit_yaw, from_osg( _center - focus_point_ ), float( _distance ) };
	}

	void osg_camera_man::setFocusPoint( const osg::Vec3d& p )
	{
		auto center_offset = _center - focus_point_;
		focus_point_ = p;
		setCenter( focus_point_ + center_offset );
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

	void osg_camera_man::updateRotation()
	{
		auto yaw = osg::Quat( orbit_yaw.rad_value(), osg::Vec3d( 0, 1, 0 ) );
		auto pitch = osg::Quat( orbit_pitch.rad_value(), osg::Vec3d( 1, 0, 0 ) );
		_rotation = pitch * yaw;
	}

	bool osg_camera_man::performMovementLeftMouseButton( const double eventTimeDelta, const double dx, const double dy )
	{
		orbit_pitch += degree( pitch_scale * dy );
		orbit_yaw -= degree( yaw_scale * dx );
		updateRotation();
		return true;
	}

	bool osg_camera_man::performMovementMiddleMouseButton( const double eventTimeDelta, const double dx, const double dy )
	{
		zoomModel( dy * zoom_scale, false );
		return true;
	}

	bool osg_camera_man::performMovementRightMouseButton( const double eventTimeDelta, const double dx, const double dy )
	{
		// pan model
		float scale = -pan_scale * _distance;
		panModel( dx * scale, dy * scale );
		return true;
	}

	bool osg_camera_man::handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
	{
		mod_key_state_ = ea.getModKeyMask();
		if ( !key_state_.contains( ea.getKey() ) )
			key_state_[ea.getKey()] = ea.getTime();

		// handle 'normal' keys
		if ( ea.getModKeyMask() == 0 )
		{
			switch ( ea.getKey() )
			{
			case 'r': setCameraState( camera_state::default_state() ); return true;
			case 'x': setCameraState( camera_state::yz() ); return true;
			case 'y': setCameraState( camera_state::xz() ); return true;
			case 'z': setCameraState( camera_state::xy() ); return true;
			case osgGA::GUIEventAdapter::KEY_Space: return false; // filter out space key because we don't want it to reset the camera
			}
		}

		// handle camera keys
		if ( ea.getKey() >= '1' && ea.getKey() <= '9' ) {
			index_t idx = ea.getKey() - '1';
			if ( ea.getModKeyMask() == 0 && idx < cameras_.size() )
				setCameraState( cameras_[idx] );
			else if ( ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL ) {
				cameras_.resize( std::max( idx + 1, cameras_.size() ) );
				cameras_[idx] = getCameraState();
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
