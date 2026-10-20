#pragma once

#include "config.hpp"

#if defined(SDL_PLATFORM_ANDROID)
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <jni.h>

namespace os
{
namespace detail
{
namespace sdl
{
/// A new reference to the ANativeWindow of SDL's current surface, or null when there is no surface.
/// It goes through the Java Surface, which Android releases under its own lock, so the window cannot
/// be freed while the reference is taken.
inline auto acquire_android_window() -> ANativeWindow*
{
	auto* env = static_cast<JNIEnv*>(SDL_GetAndroidJNIEnv());
	auto activity = static_cast<jobject>(SDL_GetAndroidActivity());
	if(env == nullptr || activity == nullptr)
	{
		return nullptr;
	}
	ANativeWindow* window = nullptr;
	jclass activity_class = env->GetObjectClass(activity);
	jmethodID get_surface = env->GetStaticMethodID(activity_class, "getNativeSurface", "()Landroid/view/Surface;");
	jobject surface = get_surface == nullptr ? nullptr : env->CallStaticObjectMethod(activity_class, get_surface);
	if(env->ExceptionCheck())
	{
		env->ExceptionClear();
	}
	if(surface != nullptr)
	{
		window = ANativeWindow_fromSurface(env, surface);
		env->DeleteLocalRef(surface);
	}
	env->DeleteLocalRef(activity_class);
	env->DeleteLocalRef(activity);
	return window;
}

/// SDL keeps the ANativeWindow of its surface in a window property. When Android destroys the
/// surface, SDL releases that window but the property keeps pointing at it, and a renderer may still
/// use the pointer: Vulkan recreates its surface from it after VK_ERROR_SURFACE_LOST_KHR. So the
/// windows handed out hold references of their own: the current one, and the one before it until
/// the next surface change, by which time the renderer has moved to the newer window.
class android_window_references
{
public:
	android_window_references() = default;
	android_window_references(const android_window_references&) = delete;
	auto operator=(const android_window_references&) -> android_window_references& = delete;

	~android_window_references()
	{
		release(previous_);
		release(current_);
	}

	/// The window to render to, given the one SDL reports. Without a live surface it keeps the
	/// window it holds.
	auto update(void* reported) -> ANativeWindow*
	{
		if(reported == nullptr || reported == current_)
		{
			return current_;
		}
		ANativeWindow* acquired = acquire_android_window();
		if(acquired == nullptr)
		{
			return current_;
		}
		if(acquired == current_)
		{
			ANativeWindow_release(acquired);
			return current_;
		}
		release(previous_);
		previous_ = current_;
		current_ = acquired;
		return current_;
	}

private:
	static void release(ANativeWindow* window)
	{
		if(window != nullptr)
		{
			ANativeWindow_release(window);
		}
	}

	ANativeWindow* current_{};
	ANativeWindow* previous_{};
};

inline auto get_android_window(SDL_Window* window) -> ANativeWindow*
{
	static android_window_references references;
	return references.update(
		SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, NULL));
}
} // namespace sdl
} // namespace detail
} // namespace os
#endif
