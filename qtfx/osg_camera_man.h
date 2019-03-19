#pragma once

#include <osgGA/OrbitManipulator>
#include "xo/geometry/angle.h"

namespace vis
{
	using degree = xo::degreef;

	using camera_state = std::tuple< degree, degree, double, double >;

	class osg_camera_man : public osgGA::OrbitManipulator
	{
	public:
		osg_camera_man();
		virtual ~osg_camera_man();

		void setYawPitch( degree yaw, degree pitch ) { orbit_yaw = yaw; orbit_pitch = pitch; updateRotation(); }
		degree getYaw() const { return orbit_yaw; }
		degree getPitch() const { return orbit_pitch; }
		void setCameraState( const camera_state& s );

	protected:
		virtual bool performMovementLeftMouseButton( const double eventTimeDelta, const double dx, const double dy ) override;
		virtual bool performMovementMiddleMouseButton( const double eventTimeDelta, const double dx, const double dy ) override;
		virtual bool performMovementRightMouseButton( const double eventTimeDelta, const double dx, const double dy ) override;
		void updateRotation();
		virtual bool handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us ) override;

	private:
		degree orbit_pitch;
		degree orbit_yaw;

		double pitch_scale = 100;
		double yaw_scale = 100;
		double pan_scale = 0.3;
		double zoom_scale = 1.0;
	};
}
