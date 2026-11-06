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
	int count = 0;
	SDL_TouchID* devices = SDL_GetTouchDevices(&count);
	SDL_free(devices);
	return count > 0;
#endif
}
} // namespace sdl
} // namespace detail
} // namespace touch
} // namespace os
