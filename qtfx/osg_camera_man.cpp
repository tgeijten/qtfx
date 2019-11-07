#include "osg_camera_man.h"
#include "xo/system/log_sink.h"
#include <functional>
#include "xo/system/log.h"

using namespace osg;
using namespace xo::literals;

namespace vis
{
	camera_state default_cam_pos{ -5.0_degf, 0_degf, Vec3d( 0, 1, 0 ), 4.5 };
	camera_state yz_cam_pos{ 0_degf, 0_degf, Vec3d( 0, 1, 0 ), 4.5 };
	camera_state xy_cam_pos{ 0_degf, 90_degf, Vec3d( 0, 1, 0 ), 4.5 };
	camera_state xz_cam_pos{ -90_degf, 90_degf, Vec3d( 0, 1, 0 ), 4.5 };

	constexpr double pitch_scale = 100;
	constexpr double yaw_scale = 100;
	constexpr double pan_scale = 0.3;
	constexpr double zoom_scale = 1.0;
	constexpr degree key_orbit = 5_degf;
	constexpr double key_pan = 0.0873; // 5/360 * 2*pi
	constexpr double key_zoom = 0.0873;

	osg_camera_man::osg_camera_man() :
		osgGA::OrbitManipulator(),
		prev_camera_state_(),
		animationMode_( false )
	{
		setAllowThrow( false );
		_minimumDistance = 0.001;

		setCameraState( default_cam_pos );

		osg::Vec3d eye, center, up;
		getTransformation( eye, center, up );
		setHomePosition( eye, center, up );
	}

	osg_camera_man::~osg_camera_man() {}

	void osg_camera_man::setCameraState( const camera_state& s )
	{
		std::tie( orbit_pitch, orbit_yaw, _center, _distance ) = s;
		updateRotation();
	}

	camera_state osg_camera_man::getCameraState()
	{
		return camera_state( orbit_pitch, orbit_yaw, _center, _distance );
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
			key_state_[ ea.getKey() ] = ea.getTime();

		// handle 'normal' keys
		if ( ea.getModKeyMask() == 0 )
		{
			switch ( ea.getKey() )
			{
			case 'r': setCameraState( default_cam_pos ); return true;
			case 'x': setCameraState( yz_cam_pos ); return true;
			case 'y': setCameraState( xz_cam_pos ); return true;
			case 'z': setCameraState( xy_cam_pos ); return true;
			case osgGA::GUIEventAdapter::KEY_Space: return false; // filter out space key because we don't want it to reset the camera
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
}
