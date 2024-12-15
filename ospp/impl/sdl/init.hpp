#pragma once

#include "error.hpp"

namespace os
{
namespace detail
{
namespace sdl
{
inline void shutdown() noexcept
{
	SDL_Quit();
}

inline auto init() -> bool
{
	SDL_SetHint("SDL_WINDOWS_DPI_SCALING", "false");
	//SDL_SetHint(SDL_HINT_VIDEO_EXTERNAL_CONTEXT, "true");

	auto result =
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMEPAD);
	if(!result)
	{
		OS_SDL_ERROR_HANDLER(result);
		return false;
	}

	return true;
}

inline auto get_current_backend() -> const char*
{
    return "SDL";
}
} // namespace sdl
} // namespace detail
} // namespace os
