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

inline auto get_cache_path(const std::string& organization, const std::string& application) -> std::string
{
#if defined(SDL_PLATFORM_ANDROID)
	const char* path = SDL_GetAndroidCachePath();
	if(path == nullptr)
	{
		return {};
	}
	return std::string(path) + "/";
#else
	const std::string preferences = get_preferences_path(organization, application);
	if(preferences.empty())
	{
		return {};
	}
	const std::string result = preferences + "cache";
	if(!SDL_CreateDirectory(result.c_str()))
	{
		return {};
	}
	// SDL ends the preferences folder with the platform's own separator.
	return result + preferences.back();
#endif
}
} // namespace sdl
} // namespace detail
} // namespace paths
} // namespace os
