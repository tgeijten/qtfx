#pragma once

#include "xo/system/profiler.h"

#define GUI_PROFILE_FUNCTION xo::scoped_profiler_section scoped_profile_var( __FUNCTION__, getGuiProfiler() )
#define GUI_PROFILE_SCONE( scope_name_arg ) xo::scoped_profiler_section scoped_profile_var( scope_name_arg, getGuiProfiler() )

xo::profiler& getGuiProfiler();
