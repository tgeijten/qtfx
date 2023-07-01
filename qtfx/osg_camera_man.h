#pragma once

#include <osgGA/OrbitManipulator>
#include "xo/geometry/angle.h"
#include "xo/container/flat_map.h"

namespace vis
{
	using degree = xo::degreef;
	struct camera_state {
		degree pitch;
		degree yaw;
		osg::Vec3d center_offset;
		double distance;
		bool operator==( const camera_state& o ) { return pitch == o.pitch && yaw == o.yaw && center_offset == o.center_offset && distance == o.distance; }
		bool operator!=( const camera_state& o ) { return !( *this == o ); }
	};

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
		void dollyModel( float d ) { _distance += d; updateRotation(); }
		void setFocusPoint( const osg::Vec3d& p );

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
		bool handleMousePush( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us ) override;
		bool handleMouseRelease( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us ) override;

	private:
		camera_state prev_camera_state_;
		xo::flat_map< int, double > key_state_;
		int mod_key_state_;
		bool animationMode_;
		degree orbit_pitch;
		degree orbit_yaw;
		osg::Vec3d focus_point_;
	};
}
