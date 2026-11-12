#pragma once

#include "../../permission.h"
#include "config.hpp"

#include <memory>
#include <utility>

namespace os
{
namespace permission
{
namespace detail
{
namespace sdl
{
#if defined(SDL_PLATFORM_ANDROID)

/// Notifications need no permission before Android 13, which does not know the permission's name and would
/// deny it.
constexpr const char* NOTIFICATIONS_PERMISSION = "android.permission.POST_NOTIFICATIONS";
constexpr int NOTIFICATIONS_PERMISSION_SDK_VERSION = 33;

/// Hands the answer to the callback a request left with SDL, and frees it.
inline void SDLCALL on_permission_result(void* userdata, const char* /*permission*/, bool is_granted)
{
	const std::unique_ptr<result_callback> on_result(static_cast<result_callback*>(userdata));
	(*on_result)(is_granted);
}

inline auto is_requested_at_run_time() -> bool
{
	return true;
}

inline auto request(const std::string& name, result_callback on_result) -> bool
{
	if(name == NOTIFICATIONS_PERMISSION && SDL_GetAndroidSDKVersion() < NOTIFICATIONS_PERMISSION_SDK_VERSION)
	{
		on_result(true);
		return true;
	}
	auto holder = std::make_unique<result_callback>(std::move(on_result));
	if(!SDL_RequestAndroidPermission(name.c_str(), on_permission_result, holder.get()))
	{
		return false;
	}
	// SDL keeps the pointer until it answers, and on_permission_result frees it.
	holder.release();
	return true;
}

#else

inline auto is_requested_at_run_time() -> bool
{
	return false;
}

inline auto request(const std::string& /*name*/, result_callback on_result) -> bool
{
	on_result(true);
	return true;
}

#endif
} // namespace sdl
} // namespace detail
} // namespace permission
} // namespace os
