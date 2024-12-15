#pragma once

#include "../../display_mode.h"

#include "error.hpp"
#include <vector>

namespace os
{
namespace detail
{
namespace sdl
{
namespace
{
inline auto from_impl(const SDL_DisplayMode& mode) -> ::os::display::mode
{
	::os::display::mode result;
	result.w = static_cast<uint32_t>(mode.w);
	result.h = static_cast<uint32_t>(mode.h);
	result.refresh_rate = static_cast<uint32_t>(mode.refresh_rate);
	result.bpp = SDL_BITSPERPIXEL(mode.format);
	result.display_scale = mode.pixel_density;
	return result;
}

inline auto to_display_id(int display_index) -> SDL_DisplayID
{
	int count = 0;
	auto displays = SDL_GetDisplays(&count);

	SDL_DisplayID display_id{};
	if(display_index < count)
	{
		display_id = displays[display_index];
	}

	SDL_free(displays);
	return display_id;
}

inline auto to_display_index(SDL_DisplayID display_id) -> int
{
	int count = 0;
	auto displays = SDL_GetDisplays(&count);

	for(int display_index = 0; display_index < count; ++display_index)
	{
		if(display_id == displays[display_index])
		{
			return display_index;
		}
	}

	return 0;
}

} // namespace

inline auto number_of_video_displays() -> int
{
	int result = 0;
	auto displays = SDL_GetDisplays(&result);
	if(result < 1)
	{
		SDL_free(displays);
		OS_SDL_ERROR_HANDLER(result);
	}
	SDL_free(displays);
	return result;
}

inline auto get_primary_display_index() -> int
{
	auto id = SDL_GetPrimaryDisplay();
	if(id == 0)
	{
		OS_SDL_ERROR_HANDLER(0);
	}

	return to_display_index(id);
}

inline auto get_available_modes(int index = 0) -> std::vector<::os::display::mode>
{
    std::vector<::os::display::mode> result;

	auto display_id = to_display_id(index);

	int count{};
	auto modes = SDL_GetFullscreenDisplayModes(display_id, &count);
	for(int i = 0; i < count; ++i)
	{
		auto mode = modes[i];
		if(!mode)
		{
			SDL_free(modes);
			OS_SDL_ERROR_HANDLER(result);
		}
		result.push_back(from_impl(*mode));
	}
	SDL_free(modes);

	return result;
}

inline auto get_desktop_mode(int index = 0) -> ::os::display::mode
{
    auto display_id = to_display_id(index);
	auto mode = SDL_GetDesktopDisplayMode(display_id);
	if(!mode)
	{
		OS_SDL_ERROR_HANDLER({});
	}
	return from_impl(*mode);
}

inline auto get_display_bounds(int index = 0) -> ::os::display::bounds
{
    auto display_id = to_display_id(index);

	SDL_Rect rect{};
	if(SDL_GetDisplayBounds(display_id, &rect))
	{
		return {rect.x, rect.y, static_cast<uint32_t>(rect.w), static_cast<uint32_t>(rect.h)};
	}

	return {};
}

inline auto get_display_usable_bounds(int index = 0) -> ::os::display::bounds
{
	auto display_id = to_display_id(index);

	SDL_Rect rect{};
	if(SDL_GetDisplayUsableBounds(display_id, &rect))
	{
		return {rect.x, rect.y, static_cast<uint32_t>(rect.w), static_cast<uint32_t>(rect.h)};
	}

	return {};
}
} // namespace sdl
} // namespace detail
} // namespace os
