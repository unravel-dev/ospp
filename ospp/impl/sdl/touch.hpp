#pragma once
#include "../../touch.h"
#include "config.hpp"

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif

namespace os
{
namespace touch
{
namespace detail
{
namespace sdl
{
#if defined(__EMSCRIPTEN__)
EM_JS(int, ospp_get_max_touch_points, (), { return navigator.maxTouchPoints || 0; });
#endif

inline auto has_screen() noexcept -> bool
{
#if defined(__EMSCRIPTEN__)
	return ospp_get_max_touch_points() > 0;
#else
	// A touchpad is a touch device too, but an indirect one: only a screen the fingers touch counts.
	int count = 0;
	SDL_TouchID* devices = SDL_GetTouchDevices(&count);
	bool has_direct = false;
	for(int i = 0; i < count && !has_direct; ++i)
	{
		has_direct = SDL_GetTouchDeviceType(devices[i]) == SDL_TOUCH_DEVICE_DIRECT;
	}
	SDL_free(devices);
	return has_direct;
#endif
}
} // namespace sdl
} // namespace detail
} // namespace touch
} // namespace os
