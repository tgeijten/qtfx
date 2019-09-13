#include "osg_camera_man.h"
#include "xo/system/log_sink.h"
#include <functional>

using namespace osg;
using namespace xo::literals;

namespace vis
{
	camera_state default_cam_pos{ -5.0_degf, 0_degf, 1.0, 4.5 };
	camera_state yz_cam_pos{ 0_degf, 0_degf, 1.0, 4.5 };
	camera_state xy_cam_pos{ 0_degf, 90_degf, 1.0, 4.5 };
	camera_state xz_cam_pos{ -90_degf, 90_degf, 1.0, 4.5 };

	constexpr double pitch_scale = 100;
	constexpr double yaw_scale = 100;
	constexpr double pan_scale = 0.3;
	constexpr double zoom_scale = 1.0;
	constexpr degree key_orbit = 5_degf;
	constexpr double key_pan = 0.1;
	constexpr double key_zoom = 0.05;

	osg_camera_man::osg_camera_man() :
	osgGA::OrbitManipulator()
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
		std::tie( orbit_pitch, orbit_yaw, _center.y(), _distance ) = s;
		updateRotation();
	}

	bool osg_camera_man::performMovementLeftMouseButton( const double eventTimeDelta, const double dx, const double dy )
	{
		orbit_pitch += degree( pitch_scale * dy );
		orbit_yaw -= degree( yaw_scale * dx );

		updateRotation();

		return true;
	}

	void osg_camera_man::updateRotation()
	{
		auto yaw = osg::Quat( orbit_yaw.rad_value(), osg::Vec3d( 0, 1, 0 ) );
		auto pitch = osg::Quat( orbit_pitch.rad_value(), osg::Vec3d( 1, 0, 0 ) );
		_rotation = pitch * yaw;
	}

	bool osg_camera_man::handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
	{
		bool shift = ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT;
		if ( shift )
		{
			switch ( ea.getKey() )
			{
			case osgGA::GUIEventAdapter::KEY_Left: panModel( -key_pan, 0 ); return true;
			case osgGA::GUIEventAdapter::KEY_Right:panModel( key_pan, 0 ); return true;
			case osgGA::GUIEventAdapter::KEY_Up:panModel( 0, key_pan ); return true;
			case osgGA::GUIEventAdapter::KEY_Down:panModel( 0, -key_pan ); return true;
			default: return osgGA::OrbitManipulator::handleKeyDown( ea, us );
			}
		} else {
			switch ( ea.getKey() )
			{
			case 'r': setCameraState( default_cam_pos ); return true;
			case 'x': setCameraState( yz_cam_pos ); return true;
			case 'y': setCameraState( xz_cam_pos ); return true;
			case 'z': setCameraState( xy_cam_pos ); return true;
			case osgGA::GUIEventAdapter::KEY_Space: return false; // filter out space key because we don't want it to reset the camera
			case osgGA::GUIEventAdapter::KEY_Left: orbitModel( -key_orbit, 0_degf ); return true;
			case osgGA::GUIEventAdapter::KEY_Right: orbitModel( key_orbit, 0_degf ); return true;
			case osgGA::GUIEventAdapter::KEY_Up: zoomModel( -key_zoom ); return true;
			case osgGA::GUIEventAdapter::KEY_Down: zoomModel( key_zoom ); return true;
			case osgGA::GUIEventAdapter::KEY_Page_Up: orbitModel( 0_degf, -key_orbit ); return true;
			case osgGA::GUIEventAdapter::KEY_Page_Down: orbitModel( 0_degf, key_orbit ); return true;
			default: return osgGA::OrbitManipulator::handleKeyDown( ea, us );
			}
		}
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
}
