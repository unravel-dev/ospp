#pragma once

#include "../../window.h"

#include "cursor.hpp"
#include "error.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace os
{
namespace detail
{
namespace sdl
{

#if defined(SDL_ENABLE_SYSWM_WINDOWS)
namespace
{
// void set_process_dpi_aware()
// {

// 	// Try SetProcessDpiAwareness first
// 	HINSTANCE shCoreDll = LoadLibraryW(L"Shcore.dll");

// 	if(shCoreDll)
// 	{
// 		enum ProcessDpiAwareness
// 		{
// 			ProcessDpiUnaware = 0,
// 			ProcessSystemDpiAware = 1,
// 			ProcessPerMonitorDpiAware = 2
// 		};

// 		typedef HRESULT(WINAPI * SetProcessDpiAwarenessFuncType)(ProcessDpiAwareness);
// 		SetProcessDpiAwarenessFuncType SetProcessDpiAwarenessFunc =
// 			reinterpret_cast<SetProcessDpiAwarenessFuncType>(
// 				GetProcAddress(shCoreDll, "SetProcessDpiAwareness"));

// 		if(SetProcessDpiAwarenessFunc)
// 		{
// 			// We only check for E_INVALIDARG because we would get
// 			// E_ACCESSDENIED if the DPI was already set previously
// 			// and S_OK means the call was successful
// 			if(SetProcessDpiAwarenessFunc(ProcessSystemDpiAware) == E_INVALIDARG)
// 			{
// 			}
// 			else
// 			{
// 				FreeLibrary(shCoreDll);
// 				return;
// 			}
// 		}

// 		FreeLibrary(shCoreDll);
// 	}

// 	// Fall back to SetProcessDPIAware if SetProcessDpiAwareness
// 	// is not available on this system
// 	HINSTANCE user32Dll = LoadLibraryW(L"user32.dll");

// 	if(user32Dll)
// 	{
// 		typedef BOOL(WINAPI * SetProcessDPIAwareFuncType)(void);
// 		SetProcessDPIAwareFuncType SetProcessDPIAwareFunc =
// 			reinterpret_cast<SetProcessDPIAwareFuncType>(GetProcAddress(user32Dll, "SetProcessDPIAware"));

// 		if(SetProcessDPIAwareFunc)
// 		{
// 			SetProcessDPIAwareFunc();
// 		}

// 		FreeLibrary(user32Dll);
// 	}
// }
} // namespace
#endif

inline auto to_cursor_impl(const cursor& c) -> const cursor_impl&
{
	return *reinterpret_cast<cursor_impl*>(c.get_impl());
}

inline auto get_native_window_handle(SDL_Window* window) noexcept -> native_handle
{
	// 	// (void)wmi;
	// #if defined(SDL_ENABLE_SYSWM_WINDOWS)
	// 	return wmi.info.win.window;
	// #elif defined(SDL_ENABLE_SYSWM_WINRT)
	// 	return wmi.info.winrt.window;
	// #elif defined(SDL_ENABLE_SYSWM_X11)
	// 	return (void*)(uintptr_t)wmi.info.x11.window;
	// #elif defined(SDL_ENABLE_SYSWM_COCOA)
	// 	return wmi.info.cocoa.window;
	// #elif defined(SDL_ENABLE_SYSWM_UIKIT)
	// 	return wmi.info.uikit.window;
	// #elif defined(SDL_ENABLE_SYSWM_WAYLAND)
	// 	return wmi.info.wl.surface;
	// #elif defined(SDL_ENABLE_SYSWM_ANDROID)
	// 	return wmi.info.android.window;
	// #elif defined(SDL_ENABLE_SYSWM_VIVANTE)
	// 	return (void*)(uintptr_t)wmi.info.vivante.window;
	// #else
	// 	return nullptr;
	// #endif

#if defined(SDL_PLATFORM_WIN32)
	return SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
#elif defined(SDL_PLATFORM_MACOS)
	return SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER,
								  NULL);
#elif defined(SDL_PLATFORM_LINUX)
	if(SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0)
	{
		// Display *xdisplay = (Display *)SDL_GetPointerProperty(SDL_GetWindowProperties(window),
		// SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
		return SDL_GetNumberProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
	}
	else if(SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0)
	{
		// struct wl_display *display = (struct wl_display
		// *)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER,
		// NULL);
		return SDL_GetPointerProperty(SDL_GetWindowProperties(window),
									  SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
	}
#elif defined(SDL_PLATFORM_IOS)
	SDL_PropertiesID props = SDL_GetWindowProperties(window);
	return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, NULL);
#else
	return nullptr;
#endif
}

inline auto get_native_display_handle(SDL_Window* window) noexcept -> native_display
{
	// 	(void)wmi;
	// #if defined(SDL_ENABLE_SYSWM_WINDOWS)
	// 	return wmi.info.win.hdc;
	// #elif defined(SDL_ENABLE_SYSWM_WINRT)
	// 	return nullptr;
	// #elif defined(SDL_ENABLE_SYSWM_X11)
	// 	return wmi.info.x11.display;
	// #elif defined(SDL_ENABLE_SYSWM_COCOA)
	// 	return nullptr;
	// #elif defined(SDL_ENABLE_SYSWM_UIKIT)
	// 	return nullptr;
	// #elif defined(SDL_ENABLE_SYSWM_WAYLAND)
	// 	return wmi.info.wl.display;
	// #elif defined(SDL_ENABLE_SYSWM_ANDROID)
	// 	return nullptr;
	// #elif defined(SDL_ENABLE_SYSWM_VIVANTE)
	// 	return (void*)(uintptr_t)wmi.info.vivante.display;
	// #else
	// 	return nullptr;
	// #endif

#if defined(SDL_PLATFORM_WIN32)
	return SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HDC_POINTER, NULL);
#elif defined(SDL_PLATFORM_MACOS)
	return nullptr
#elif defined(SDL_PLATFORM_LINUX)
	if(SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0)
	{
		return SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_DISPLAY_POINTER,
									  NULL);
	}
	else if(SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0)
	{
		return SDL_GetPointerProperty(SDL_GetWindowProperties(window),
									  SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
	}
#elif defined(SDL_PLATFORM_IOS)
	return nullptr;
#else
	return nullptr;
#endif
}

inline auto get_impl_flags(uint32_t flags) -> uint32_t
{
	uint32_t result = 0x00002000; // SDL_WINDOW_ALLOW_HIGHDPI;
	if(flags & window::fullscreen)
	{
		result |= SDL_WINDOW_FULLSCREEN;
	}
	if(flags & window::fullscreen_desktop)
	{
		result |= 0x00001000; // SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	if(flags & window::hidden)
	{
		result |= SDL_WINDOW_HIDDEN;
	}
	if(flags & window::borderless)
	{
		result |= SDL_WINDOW_BORDERLESS;
	}
	if(flags & window::resizable)
	{
		result |= SDL_WINDOW_RESIZABLE;
	}
	if(flags & window::minimized)
	{
		result |= SDL_WINDOW_MINIMIZED;
	}
	if(flags & window::maximized)
	{
		result |= SDL_WINDOW_MAXIMIZED;
	}

	// #if defined(SDL_ENABLE_SYSWM_WINDOWS)
	// 	// due to sdl's current lack of dpi awarenes on windows
	// 	// we have to implement it ourselves
	// 	set_process_dpi_aware();
	// #endif

	return result;
}

struct window_deleter
{
	void operator()(SDL_Window* window) const noexcept
	{
		SDL_DestroyWindow(window);
	}
};

class window_impl
{
	constexpr static const auto npos = SDL_WINDOWPOS_UNDEFINED;
	constexpr static const auto centered = SDL_WINDOWPOS_CENTERED;

public:
	window_impl(const std::string& title, const point& pos, const area& size, uint32_t flags)
		: impl_(SDL_CreateWindow(title.c_str(), static_cast<int>(size.w), static_cast<int>(size.h),
								 get_impl_flags(flags)))
	{
		if(impl_ == nullptr)
		{
			OS_SDL_ERROR_HANDLER_VOID();
		}
		set_position(point(pos.x == window::centered ? centered : pos.x,
						   pos.y == window::centered ? centered : pos.y));
	}

	auto get_impl() const noexcept -> SDL_Window*
	{
		return impl_.get();
	}

	auto get_native_handle() const -> native_handle
	{
		return get_native_window_handle(impl_.get());
	}
	auto get_native_display() const -> native_display
	{
		return get_native_display_handle(impl_.get());
	}

	auto is_open() const noexcept -> bool
	{
		return impl_ != nullptr;
	}

	auto get_id() const noexcept -> uint32_t
	{
		return SDL_GetWindowID(impl_.get());
	}

	auto get_brightness() const noexcept -> float
	{
		return 1.0f;
	}

	void set_brightness(float bright)
	{
	}

	void set_size(const area& size) noexcept
	{
		SDL_SetWindowSize(impl_.get(), static_cast<int>(size.w), static_cast<int>(size.h));
	}

	auto get_size() const noexcept -> area
	{
		area result{};
		int w{};
		int h{};
		SDL_GetWindowSize(impl_.get(), &w, &h);
		result.w = static_cast<uint32_t>(w);
		result.h = static_cast<uint32_t>(h);
		return result;
	}

	void set_maximum_size(const area& size) noexcept
	{
		SDL_SetWindowMaximumSize(impl_.get(), static_cast<int>(size.w), static_cast<int>(size.h));
	}

	auto get_maximum_size() const noexcept -> area
	{
		area result{};
		int w{};
		int h{};
		SDL_GetWindowMaximumSize(impl_.get(), &w, &h);
		result.w = static_cast<uint32_t>(w);
		result.h = static_cast<uint32_t>(h);

		return result;
	}

	void set_minimum_size(const area& size) noexcept
	{
		SDL_SetWindowMinimumSize(impl_.get(), static_cast<int>(size.w), static_cast<int>(size.h));
	}

	auto get_minimum_size() const noexcept -> area
	{
		area result;
		int w{};
		int h{};
		SDL_GetWindowMinimumSize(impl_.get(), &w, &h);
		result.w = static_cast<uint32_t>(w);
		result.h = static_cast<uint32_t>(h);
		return result;
	}

	void set_position(const point& pos) noexcept
	{
		SDL_SetWindowPosition(impl_.get(), static_cast<int>(pos.x), static_cast<int>(pos.y));
	}

	auto get_position() const noexcept -> point
	{
		point result;
		int x{};
		int y{};
		SDL_GetWindowPosition(impl_.get(), &x, &y);
		result.x = static_cast<int32_t>(x);
		result.y = static_cast<int32_t>(y);
		return result;
	}

	auto get_title() const noexcept -> std::string
	{
		return SDL_GetWindowTitle(impl_.get());
	}

	void set_title(const std::string& str) noexcept
	{
		SDL_SetWindowTitle(impl_.get(), str.c_str());
	}

	void show() noexcept
	{
		SDL_ShowWindow(impl_.get());
	}

	void hide() noexcept
	{
		SDL_HideWindow(impl_.get());
	}

	void maximize() noexcept
	{
		SDL_MaximizeWindow(impl_.get());
	}

	void minimize() noexcept
	{
		SDL_MinimizeWindow(impl_.get());
	}

	void restore() noexcept
	{
		SDL_RestoreWindow(impl_.get());
	}

	void raise() noexcept
	{
		SDL_RaiseWindow(impl_.get());
	}

	auto is_minimized() const noexcept -> bool
	{
		return (SDL_GetWindowFlags(impl_.get()) & SDL_WINDOW_MINIMIZED) != 0;
	}

	void set_border(bool b) noexcept
	{
		if(!SDL_SetWindowBordered(impl_.get(), b))
		{
			OS_SDL_ERROR_HANDLER_VOID();
		}
	}

	void set_fullscreen(bool b)
	{
		if(!SDL_SetWindowFullscreen(impl_.get(), b))
		{
			OS_SDL_ERROR_HANDLER_VOID();
		}
	}

	void set_opacity(float opacity)
	{
		if(!SDL_SetWindowOpacity(impl_.get(), opacity))
		{
			OS_SDL_ERROR_HANDLER_VOID();
		}
	}

	auto get_opacity() const -> float
	{
		return SDL_GetWindowOpacity(impl_.get());
	}

	void grab_input(bool b) noexcept
	{
		SDL_SetWindowMouseGrab(impl_.get(), b);
		// SDL_SetWindowKeyboardGrab(impl_.get(), b);
	}

	auto is_input_grabbed() const noexcept -> bool
	{
		return SDL_GetWindowMouseGrab(impl_.get());
	}

	void request_focus()
	{
		if(!SDL_RaiseWindow(impl_.get()))
		{
			OS_SDL_ERROR_HANDLER_VOID();
		}
	}

	auto has_focus() const noexcept -> bool
	{
		auto flags = SDL_GetWindowFlags(impl_.get());
		return (flags & SDL_WINDOW_INPUT_FOCUS) != 0;
	}

	void set_cursor(const cursor& c) noexcept
	{
		SDL_SetCursor(to_cursor_impl(c).get_impl());
	}

	void show_cursor(bool show) noexcept
	{
		if(show)
		{
			SDL_ShowCursor();
		}
		else
		{
			SDL_HideCursor();
		}
	}

	void set_icon(const image& img)
	{
		auto surface =
			SDL_CreateSurfaceFrom(int(img.size.w), int(img.size.h), SDL_PixelFormat::SDL_PIXELFORMAT_RGBA8888,
								  (void*)img.pixels.data(), int(img.size.w * 4));
		if(!surface)
		{
			OS_SDL_ERROR_HANDLER_VOID();
		}

		SDL_SetWindowIcon(impl_.get(), surface);
		SDL_DestroySurface(surface);
	}

private:
	std::unique_ptr<SDL_Window, window_deleter> impl_;
};
} // namespace sdl
} // namespace detail
} // namespace os
