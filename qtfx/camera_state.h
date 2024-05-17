#pragma once

#include "xo/geometry/angle.h"
#include "vis/types.h"
#include "xo/container/prop_node_tools.h"
#include "xo/numerical/interpolation.h"

namespace vis
{
	struct camera_state {
		degreef pitch = -5_degf;
		degreef yaw = 0_degf;
		vec3f center_offset = vec3f( 0.0f, 1.0f, 0.0f );
		float distance = 4.5f;

		bool operator==( const camera_state& o ) { return pitch == o.pitch && yaw == o.yaw && center_offset == o.center_offset && distance == o.distance; }
		bool operator!=( const camera_state& o ) { return !( *this == o ); }

		static camera_state default_state() { return camera_state{ -5.0_degf, 0_degf, vec3f( 0, 1, 0 ), 4.5 }; };
		static camera_state yz() { return camera_state{ 0_degf, 0_degf, vec3f( 0, 1, 0 ), 4.5 }; };
		static camera_state xy() { return camera_state{ 0_degf, 90_degf, vec3f( 0, 1, 0 ), 4.5 }; };
		static camera_state xz() { return camera_state{ -90_degf, 90_degf, vec3f( 0, 1, 0 ), 4.5 }; };
	};

	inline camera_state lerp( const camera_state& c1, const camera_state& c2, float t ) {
		return camera_state{
			xo::lerp( c1.pitch, c2.pitch, t ),
			xo::lerp( c1.yaw, c2.yaw, t ),
			xo::lerp( c1.center_offset, c2.center_offset, t ),
			xo::lerp( c1.distance, c2.distance, t )
		};
	}
}

namespace xo
{
	inline bool from_prop_node( const prop_node& pn, vis::camera_state& c ) {
		INIT_PROP_REQUIRED( pn, c.pitch );
		INIT_PROP_REQUIRED( pn, c.yaw );
		INIT_PROP_REQUIRED( pn, c.center_offset );
		INIT_PROP_REQUIRED( pn, c.distance );
		return true;
	};

	/// convert to prop_node
	inline prop_node to_prop_node( const vis::camera_state& c ) {
		prop_node pn;
		SET_PROP( pn, c.pitch );
		SET_PROP( pn, c.yaw );
		SET_PROP( pn, c.center_offset );
		SET_PROP( pn, c.distance );
		return pn;
	}
}
