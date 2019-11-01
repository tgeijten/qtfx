#pragma once

#include <osgGA/OrbitManipulator>
#include "xo/geometry/angle.h"
#include "xo/container/flat_map.h"

namespace vis
{
	using degree = xo::degreef;
	using camera_state = std::tuple< degree, degree, osg::Vec3d, double >;

	class osg_camera_man : public osgGA::OrbitManipulator
	{
	public:
		osg_camera_man();
		virtual ~osg_camera_man();

		degree getYaw() const { return orbit_yaw; }
		degree getPitch() const { return orbit_pitch; }

		void setCameraState( const camera_state& s );
		camera_state getCameraState();

		void setOrbit( degree yaw, degree pitch ) { orbit_yaw = yaw; orbit_pitch = pitch; updateRotation(); }
		void orbitModel( degree yaw, degree pitch ) { orbit_yaw += yaw; orbit_pitch += pitch; updateRotation(); }

		bool hasCameraStateChanged();
		void handleKeyboardAnimation();

	protected:
		void updateRotation();
		virtual bool performMovementLeftMouseButton( const double eventTimeDelta, const double dx, const double dy ) override;
		virtual bool performMovementMiddleMouseButton( const double eventTimeDelta, const double dx, const double dy ) override;
		virtual bool performMovementRightMouseButton( const double eventTimeDelta, const double dx, const double dy ) override;
		virtual bool handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us ) override;
		virtual bool handleKeyUp( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us ) override;
		bool handleKeyCommand( int key, int mod_keys, double factor = 1.0 );

	private:
		camera_state prev_camera_state_;
		xo::flat_map< int, double > key_state_;
		int mod_key_state_;
		bool animationMode_;
		degree orbit_pitch;
		degree orbit_yaw;
	};
}
