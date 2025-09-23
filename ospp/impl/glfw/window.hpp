#pragma once

#include "../../window.h"

#include "cursor.hpp"
#include "error.hpp"
#include "hints.hpp"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>

namespace os
{
namespace detail
{
namespace glfw
{

inline auto to_cursor_impl(const cursor& c) -> const cursor_impl&
{
	return *reinterpret_cast<cursor_impl*>(c.get_impl());
}

inline auto get_native_window_handle(GLFWwindow* window) noexcept -> native_handle
{
	(void)window;
#if defined(GLFW_EXPOSE_NATIVE_WIN32)
	return glfwGetWin32Window(window);
#elif defined(GLFW_EXPOSE_NATIVE_X11)
	return (void*)(uintptr_t)glfwGetX11Window(window);
#elif defined(GLFW_EXPOSE_NATIVE_COCOA)
	return glfwGetCocoaWindow(window);
#elif defined(GLFW_EXPOSE_NATIVE_WAYLAND)
	return glfwGetWaylandWindow(window);
#else
	return nullptr;
#endif
}

inline auto get_native_display_handle() noexcept -> native_display
{
#if defined(GLFW_EXPOSE_NATIVE_WIN32)
	return nullptr;
#elif defined(GLFW_EXPOSE_NATIVE_X11)
	return (void*)(uintptr_t)glfwGetX11Display();
#elif defined(GLFW_EXPOSE_NATIVE_COCOA)
	return nullptr;
#elif defined(GLFW_EXPOSE_NATIVE_WAYLAND)
	return glfwGetWaylandDisplay();
#else
	return nullptr;
#endif
}
inline bool glfwSetWindowCenter(GLFWwindow* window)
{
	if(!window)
		return false;

	int sx = 0, sy = 0;
	int px = 0, py = 0;
	int mx = 0, my = 0;
	int monitor_count = 0;
	int best_area = 0;
	int final_x = 0, final_y = 0;

	glfwGetWindowSize(window, &sx, &sy);
	glfwGetWindowPos(window, &px, &py);

	// Iterate throug all monitors
	GLFWmonitor** m = glfwGetMonitors(&monitor_count);
	if(!m)
		return false;

	for(int j = 0; j < monitor_count; ++j)
	{

		glfwGetMonitorPos(m[j], &mx, &my);
		const GLFWvidmode* mode = glfwGetVideoMode(m[j]);
		if(!mode)
			continue;

		// Get intersection of two rectangles - screen and window
		int minX = std::max(mx, px);
		int minY = std::max(my, py);

		int maxX = std::min(mx + mode->width, px + sx);
		int maxY = std::min(my + mode->height, py + sy);

		// Calculate area of the intersection
		int area = std::max(maxX - minX, 0) * std::max(maxY - minY, 0);

		// If its bigger than actual (window covers more space on this monitor)
		if(area > best_area)
		{
			// Calculate proper position in this monitor
			final_x = mx + (mode->width - sx) / 2;
			final_y = my + (mode->height - sy) / 2;

			best_area = area;
		}
	}

	// We found something
	if(best_area)
		glfwSetWindowPos(window, final_x, final_y);

	// Something is wrong - current window has NOT any intersection with any monitors. Move it to the default
	// one.
	else
	{
		GLFWmonitor* primary = glfwGetPrimaryMonitor();
		if(primary)
		{
			const GLFWvidmode* desktop = glfwGetVideoMode(primary);

			if(desktop)
				glfwSetWindowPos(window, (desktop->width - sx) / 2, (desktop->height - sy) / 2);
			else
				return false;
		}
		else
			return false;
	}

	return true;
}

inline auto create_impl(const std::string& title, uint32_t width, uint32_t height, uint32_t flags)
	-> GLFWwindow*
{

	// NO CLIENT API. This library's purpose is to just
	// wrap window and input.
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	auto visible = flags & window::hidden ? GLFW_FALSE : GLFW_TRUE;
	glfwWindowHint(GLFW_VISIBLE, visible);

	auto decorated = flags & window::borderless ? GLFW_FALSE : GLFW_TRUE;
	glfwWindowHint(GLFW_DECORATED, decorated);

	auto resizable = flags & window::resizable ? GLFW_TRUE : GLFW_FALSE;
	glfwWindowHint(GLFW_RESIZABLE, resizable);

	auto maximized = flags & window::maximized ? GLFW_TRUE : GLFW_FALSE;
	glfwWindowHint(GLFW_MAXIMIZED, maximized);

	auto on_top = flags & window::always_on_top ? GLFW_TRUE : GLFW_FALSE;
	glfwWindowHint(GLFW_FLOATING, on_top);

	auto passthrough = get_hint_value(HINT_MOUSE_FOCUS_CLICKTHROUGH);
	glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, passthrough);


	GLFWmonitor* monitor = nullptr;
	if(flags & window::fullscreen)
	{
		monitor = glfwGetPrimaryMonitor();
	}

	if(flags & window::fullscreen_desktop)
	{
		monitor = glfwGetPrimaryMonitor();
		auto mode = glfwGetVideoMode(monitor);

		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	}

	auto window =
		glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title.c_str(), monitor, nullptr);

	glfwDefaultWindowHints();

	return window;
}
class window_impl;

inline auto get_focused_win() noexcept -> window_impl*&
{
	static window_impl* win{nullptr};
	return win;
}

struct window_deleter
{
	void operator()(GLFWwindow* window) const noexcept
	{
		glfwDestroyWindow(window);
	}
};

inline auto get_windows() noexcept -> std::vector<window_impl*>&
{
	static std::vector<window_impl*> windows;
	return windows;
}

inline auto register_window(window_impl* impl) -> uint32_t
{
	static uint32_t id{0};
	auto& windows = get_windows();
	windows.emplace_back(impl);
	return ++id;
}

inline void unregister_window(uint32_t id)
{
	auto& windows = get_windows();
	windows.erase(std::remove_if(std::begin(windows), std::end(windows),
								 [id](const auto& win) { return win->get_id() == id; }),
				  std::end(windows));
}

using on_window_create = void (*)(GLFWwindow*);
inline auto on_win_create() -> on_window_create&
{
	static on_window_create callback = nullptr;
	return callback;
}

class window_impl
{

public:
	window_impl(const std::string& title, const point& pos, const area& size, uint32_t flags)
		: impl_(create_impl(title, size.w, size.h, flags))
	{
		if(impl_ == nullptr)
		{
			OS_GLFW_ERROR_HANDLER_VOID();
		}
		title_ = title;
		id_ = register_window(this);

		glfwSetWindowUserPointer(impl_.get(), this);

		auto callback = on_win_create();
		if(callback)
		{
			callback(impl_.get());
		}

		set_position(pos);

        if(has_focus())
        {
            get_focused_win() = this;
        }
	}

	~window_impl()
	{
		unregister_window(id_);

		auto& focused = get_focused_win();
		if(focused == this)
		{
			focused = nullptr;
		}
	}

	auto get_impl() const noexcept -> GLFWwindow*
	{
		return impl_.get();
	}

	auto get_native_handle() const noexcept -> native_handle
	{
		return get_native_window_handle(impl_.get());
	}
	auto get_native_display() const noexcept -> native_display
	{
		return get_native_display_handle();
	}

	auto get_display_scale() const -> float
	{
		float xscale = 1.0f;
		float yscale = 1.0f;
		glfwGetWindowContentScale(impl_.get(), &xscale, &yscale);
		return std::max(xscale, yscale);
	}

	auto is_open() const noexcept -> bool
	{
		return impl_ != nullptr;
	}

	auto get_id() const noexcept -> uint32_t
	{
		return id_;
	}

	void set_size(const area& size) noexcept
	{
		glfwSetWindowSize(impl_.get(), static_cast<int>(size.w), static_cast<int>(size.h));
	}

	auto get_size() const noexcept -> area
	{
		area result{};
		int w{};
		int h{};
		glfwGetWindowSize(impl_.get(), &w, &h);
		result.w = static_cast<uint32_t>(w);
		result.h = static_cast<uint32_t>(h);
		return result;
	}

	void set_maximum_size(const area& size) noexcept
	{
		max_size_ = size;
		glfwSetWindowSizeLimits(impl_.get(), GLFW_DONT_CARE, GLFW_DONT_CARE, static_cast<int>(size.w),
								static_cast<int>(size.h));
	}

	auto get_maximum_size() const noexcept -> area
	{
		return max_size_;
	}

	void set_minimum_size(const area& size) noexcept
	{
		min_size_ = size;
		glfwSetWindowSizeLimits(impl_.get(), static_cast<int>(size.w), static_cast<int>(size.h),
								GLFW_DONT_CARE, GLFW_DONT_CARE);
	}

	auto get_minimum_size() const noexcept -> area
	{
		return min_size_;
	}

	void set_position(const point& pos) noexcept
	{
		if(pos.x == window::centered && pos.y == window::centered)
		{
			glfwSetWindowCenter(impl_.get());
		}
		else
		{
			glfwSetWindowPos(impl_.get(), static_cast<int>(pos.x), static_cast<int>(pos.y));
		}
	}

	auto get_position() noexcept -> point
	{
		point result;
		int x{};
		int y{};
		glfwGetWindowPos(impl_.get(), &x, &y);
		result.x = static_cast<int32_t>(x);
		result.y = static_cast<int32_t>(y);
		return result;
	}

	auto get_title() const noexcept -> std::string
	{
		return title_;
	}

	void set_title(const std::string& str) noexcept
	{
		title_ = str;
		glfwSetWindowTitle(impl_.get(), str.c_str());
	}

	void show() noexcept
	{
		glfwShowWindow(impl_.get());
	}

	void hide() noexcept
	{
		glfwHideWindow(impl_.get());
	}

	void maximize() noexcept
	{
		glfwMaximizeWindow(impl_.get());
	}

	void minimize() noexcept
	{
        glfwIconifyWindow(impl_.get());
	}

	void restore() noexcept
	{
		glfwRestoreWindow(impl_.get());
	}

	void raise() noexcept
	{
        glfwRequestWindowAttention(impl_.get());
	}

    auto is_minimized() const noexcept -> bool
	{
		return glfwGetWindowAttrib(impl_.get(), GLFW_ICONIFIED) != 0;
    }

	void set_border(bool) noexcept
	{
	}

	void set_fullscreen(bool b)
	{
		auto monitor = glfwGetPrimaryMonitor();
		auto mode = glfwGetVideoMode(monitor);
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

		if(b)
		{
			size_before_fullscreen_ = get_size();
			pos_before_fullscreen_ = get_position();
			glfwSetWindowMonitor(impl_.get(), monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		}
		else
		{
			glfwSetWindowMonitor(impl_.get(), nullptr, pos_before_fullscreen_.x, pos_before_fullscreen_.y,
								 int(size_before_fullscreen_.w), int(size_before_fullscreen_.h),
								 mode->refreshRate);
		}
	}

	void set_opacity(float opacity)
	{
        glfwSetWindowOpacity(impl_.get(), opacity);
	}

	auto get_opacity() const -> float
    {
        return glfwGetWindowOpacity(impl_.get());
	}

	void grab_input(bool b) noexcept
	{
		input_grabbed_ = b;
		glfwSetInputMode(impl_.get(), GLFW_CURSOR, b ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	}

	bool is_input_grabbed() const noexcept
	{
		return glfwGetInputMode(impl_.get(), GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
	}

	void request_focus()
	{
		glfwFocusWindow(impl_.get());
	}

    bool has_focus()
    {
        return glfwGetWindowAttrib(impl_.get(), GLFW_FOCUSED) != 0;
    }

	void set_cursor(const cursor& c) noexcept
	{
		glfwSetCursor(impl_.get(), to_cursor_impl(c).get_impl());
	}

	void show_cursor(bool show) noexcept
	{
		if(input_grabbed_)
		{
			return;
		}
		glfwSetInputMode(impl_.get(), GLFW_CURSOR, show ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
	}

	void set_icon(const image& img) noexcept
	{
		GLFWimage image{};
		image.pixels = (unsigned char*)img.pixels.data();
		image.width = static_cast<int>(img.size.w);
		image.height = static_cast<int>(img.size.h);
		glfwSetWindowIcon(impl_.get(), 1, &image);
	}

private:
	uint32_t id_{};
	area min_size_{};
	area max_size_{};
	point pos_before_fullscreen_{};
	area size_before_fullscreen_{};
	bool input_grabbed_{};

	std::string title_{};
	std::unique_ptr<GLFWwindow, window_deleter> impl_;
};
}
}
}
