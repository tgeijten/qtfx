#pragma once

#include <osgGA/OrbitManipulator>
#include "xo/container/flat_map.h"
#include "vis/camera_state.h"
#include "vis/camera_animation.h"

namespace vis
{
	using degree = vis::degreef; // #todo: this should be in vis/types.h
	class osg_camera_man : public osgGA::OrbitManipulator
	{
	public:
		osg_camera_man();
		virtual ~osg_camera_man();

		degree getYaw() const { return orbit_yaw; }
		degree getPitch() const { return orbit_pitch; }

		void setCameraState( const camera_state& s );
		camera_state getCameraState();
		void setTransition( const camera_state& s );

		void setOrbit( degree yaw, degree pitch ) { orbit_yaw = yaw; orbit_pitch = pitch; updateRotation(); }
		void orbitModel( degree yaw, degree pitch ) { orbit_yaw += yaw; orbit_pitch += pitch; updateRotation(); }
		void dollyModel( float d ) { _distance += d; updateRotation(); }
		void setFocusPoint( const osg::Vec3d& p );
		const osg::Vec3d& getFocusPoint() const { return focus_point_; }
		void setOrbitAnimation( degree yps, degree pps, float dps );
		void setTransitionDuration( double t ) { transitionDuration_ = t; }

		void setPlaybackMode( bool b ) { playbackMode_ = b; }
		void setEnableCameraManipulation( bool b ) { enableCameraManipulation_ = b; }

		bool hasCameraStateChanged();
		void handleKeyboardAnimation();
		void handleAnimation( double t, float dt );

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
		bool playbackMode_;
		bool enableCameraManipulation_;
		degree orbit_pitch;
		degree orbit_yaw;
		osg::Vec3d focus_point_;

		// orbit animation parameters
		double animationTime_;
		degree yawAnimationVelocity;
		degree pitchAnimationVelocity;
		float dollyAnimationVelocity = 0.0f;

		// camera transition animation
		double transitionDuration_ = 0.5;
		camera_animation transition_;

		// camera states for 1-9 shortcuts
		std::vector<camera_state> cameras_;
	};
}
