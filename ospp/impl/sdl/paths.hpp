#pragma once

#include "config.hpp"

#include <string>

namespace os
{
namespace paths
{
namespace detail
{
namespace sdl
{
inline auto get_preferences_path(const std::string& organization, const std::string& application) -> std::string
{
	char* path = SDL_GetPrefPath(organization.c_str(), application.c_str());
	if(path == nullptr)
	{
		return {};
	}
	std::string result = path;
	SDL_free(path);
	return result;
}
} // namespace sdl
} // namespace detail
} // namespace paths
} // namespace os
