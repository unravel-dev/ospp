#include "display_mode.h"
#include <iostream>
#include <sstream>

#if defined(SDL_BACKEND)
#include "impl/sdl/display_mode.hpp"
#ifndef impl
#define impl detail::sdl
#endif
#elif defined(GLFW_BACKEND)
#include "impl/glfw/display_mode.hpp"
#define impl detail::glfw
#elif defined(MML_BACKEND)
#include "impl/mml/display_mode.hpp"
#define impl detail::mml
#else
#error "unsupported backend"
#endif

namespace os
{
namespace display
{
auto get_available_displays_count() -> int
{
	return impl::number_of_video_displays();
}

auto get_primary_display_index() -> int
{
	return impl::get_primary_display_index();
}

auto get_available_modes(int index) -> std::vector<mode>
{
	return impl::get_available_modes(index);
}

auto get_desktop_mode(int index) -> mode
{
	return impl::get_desktop_mode(index);
}

auto get_closest_mode(uint32_t width, uint32_t height, int index) -> mode
{
	mode to;
	to.w = width;
	to.h = height;
	return get_closest_mode(to, index);
}

auto get_closest_mode(const mode& to, int index) -> mode
{
	auto desktop_mode = get_desktop_mode(index);
	auto modes = get_available_modes(index);

	uint32_t target_bpp{};
	if(to.bpp)
	{
		target_bpp = to.bpp;
	}
	else
	{
		target_bpp = desktop_mode.bpp;
	}

	uint32_t target_refresh_rate{};
	if(to.refresh_rate)
	{
		target_refresh_rate = to.refresh_rate;
	}
	else
	{
		target_refresh_rate = desktop_mode.refresh_rate;
	}

	const mode* match = nullptr;

	for(const auto& current : modes)
	{
		if(current.w && (current.w < to.w))
		{
			/* Out of sorted modes large enough here */
			break;
		}
		if(current.h && (current.h < to.h))
		{
			if(current.w && (current.w == to.w))
			{
				/* Out of sorted modes large enough here */
				break;
			}
			/* Wider, but not tall enough, due to a different
			   aspect ratio. This mode must be skipped, but closer
			   modes may still follow. */
			continue;
		}
		if(!match || current.w < match->w || current.h < match->h)
		{
			match = &current;
			continue;
		}
		if(current.bpp != match->bpp)
		{
			/* Sorted highest depth to lowest */
			if(current.bpp >= target_bpp)
			{
				match = &current;
			}
			continue;
		}
		if(current.refresh_rate != match->refresh_rate)
		{
			/* Sorted highest refresh to lowest */
			if(current.refresh_rate >= target_refresh_rate)
			{
				match = &current;
			}
		}
	}

	mode closest{};
	if(match)
	{
		if(match->bpp)
		{
			closest.bpp = match->bpp;
		}
		else
		{
			closest.bpp = to.bpp;
		}
		if(match->w && match->h)
		{
			closest.w = match->w;
			closest.h = match->h;
		}
		else
		{
			closest.w = to.w;
			closest.h = to.h;
		}
		if(match->refresh_rate)
		{
			closest.refresh_rate = match->refresh_rate;
		}
		else
		{
			closest.refresh_rate = to.refresh_rate;
		}

		/*
		 * Pick some reasonable defaults if the app and driver don't
		 * care
		 */
		if(!closest.bpp)
		{
			closest.bpp = 24;
		}
		if(!closest.w)
		{
			closest.w = 640;
		}
		if(!closest.h)
		{
			closest.h = 480;
		}
	}
	return closest;
}

auto get_bounds(int index) -> bounds
{
	return impl::get_display_bounds(index);
}

auto get_usable_bounds(int index) -> bounds
{
	auto usable = impl::get_display_usable_bounds(index);
	if(usable.w == 0 || usable.h == 0)
	{
		return get_bounds(index);
	}

	return usable;
}

auto get_content_scale(int index) -> float
{
	return impl::get_content_scale(index);
}

auto to_string(const mode& m) -> std::string
{
	std::stringstream ss;
	ss << "mode.w             = " << m.w << "\n";
	ss << "mode.h             = " << m.h << "\n";
	ss << "mode.bpp           = " << m.bpp << "\n";
	ss << "mode.refresh_rate  = " << m.refresh_rate << "\n";
    ss << "mode.display_scale = " << m.display_scale;

	return ss.str();
}

auto to_string(const bounds& b) -> std::string
{
	std::stringstream ss;
	ss << "bounds.x = " << b.x << "\n";
	ss << "bounds.y = " << b.y << "\n";
	ss << "bounds.w = " << b.w << "\n";
	ss << "bounds.h = " << b.h;
	return ss.str();
}

void enumerate_caps()
{
	auto count = get_available_displays_count();
	std::cout << "displays : " << count << std::endl;
	for(int i = 0; i < count; ++i)
	{
		std::cout << "--------------------------------------" << std::endl;
		auto bounds = get_bounds(i);
		std::cout << "bounds" << std::endl;
		std::cout << "--------------------------------------" << std::endl;
		std::cout << to_string(bounds) << std::endl;

		std::cout << "--------------------------------------" << std::endl;
		auto usable_bounds = get_usable_bounds(i);
		std::cout << "usable_bounds" << std::endl;
		std::cout << "--------------------------------------" << std::endl;
		std::cout << to_string(usable_bounds) << std::endl;

		std::cout << "--------------------------------------" << std::endl;
		auto mode = get_desktop_mode(i);
		std::cout << "mode" << std::endl;
		std::cout << "--------------------------------------" << std::endl;
		std::cout << to_string(mode) << std::endl;

		std::cout << "--------------------------------------" << std::endl;
		auto modes = get_available_modes(i);
		std::cout << "available modes : " << modes.size() << std::endl;
		for(const auto& av_mode : modes)
		{
			std::cout << "--------------------------------------" << std::endl;
			std::cout << to_string(av_mode) << std::endl;
		}
	}
}
} // namespace display
} // namespace os
