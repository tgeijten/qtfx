#include "gui_profiler.h"

xo::profiler& getGuiProfiler()
{
	static xo::profiler g_profiler;
	return g_profiler;
}
