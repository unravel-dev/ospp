#pragma once

#include "../../window.h"

#include "config.hpp"
#include "cursor.hpp"

#include <algorithm>
#include <cstdint>
#include <map>
#include <memory>
#include <string>

namespace os
{
namespace detail
{
namespace mml
{
inline auto to_cursor_impl(const cursor& c) -> const cursor_impl&
{
	return *reinterpret_cast<cursor_impl*>(c.get_impl());
}

inline auto get_impl_flags(uint32_t flags) -> uint32_t
{
	uint32_t result = ::mml::style::titlebar | ::mml::style::close;
	if(flags & window::fullscreen)
	{
		result |= ::mml::style::fullscreen;
	}
	if(flags & window::fullscreen_desktop)
	{
		result |= ::mml::style::fullscreen;
		result &= ~::mml::style::titlebar;
	}
	if(flags & window::hidden)
	{
		result |= ::mml::style::hidden;
	}
	if(flags & window::resizable)
	{
		result |= ::mml::style::resize;
	}
	if(flags & window::borderless)
	{
		result = ::mml::style::none;
	}
	if(flags & window::always_on_top)
	{
		result |= ::mml::style::always_on_top;
	}
	if(flags & window::no_taskbar)
	{
		result |= ::mml::style::no_taskbar;
	}
	if(flags & window::minimized)
	{
	}
	if(flags & window::maximized)
	{
	}
	if(flags & window::tooltip)
	{
		result |= ::mml::style::tooltip;
	}
	if(flags & window::popup)
	{
		result |= ::mml::style::popup_menu;
	}
	if(flags & window::utility)
	{
		result |= ::mml::style::utility;
	}

	return result;
}

class window_impl;

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

class window_impl
{

public:
	window_impl(const std::string& title, const point& pos, const area& size, uint32_t flags)
		: impl_(::mml::video_mode{size.w, size.h}, {{pos.x, pos.y}}, title, get_impl_flags(flags))
		, title_(title)
		, style_(get_impl_flags(flags))
	{
		id_ = register_window(this);


		if(flags & window::minimized)
		{
			minimize();
		}
		if(flags & window::maximized)
		{
			maximize();
		}
	}

	~window_impl()
	{
		unregister_window(id_);
	}

	auto get_impl() const -> const ::mml::window&
	{
		return impl_;
	}

	auto get_impl() -> ::mml::window&
	{
		return impl_;
	}

	auto get_native_handle() const -> native_handle
	{
		return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(impl_.native_handle()));
	}
	auto get_native_display() const -> native_display
	{
		return impl_.native_display_handle();
	}

	auto get_display_scale() const -> float
	{
		return 1.0f;
	}

	auto is_open() const noexcept -> bool
	{
		return impl_.is_open();
	}

	auto get_id() const noexcept -> uint32_t
	{
		return id_;
	}

	void set_size(const area& size) noexcept
	{
		impl_.set_size(std::array<uint32_t, 2>{{size.w, size.h}});
	}

	auto get_size() const noexcept -> area
	{
		area result{};
		auto size = impl_.get_size();
		result.w = static_cast<uint32_t>(size[0]);
		result.h = static_cast<uint32_t>(size[1]);
		return result;
	}

	void set_maximum_size(const area&) noexcept
	{
	}

	auto get_maximum_size() const noexcept -> area
	{
		return {};
	}

	void set_minimum_size(const area&) noexcept
	{
	}

	auto get_minimum_size() const noexcept -> area
	{
		return {};
	}

	void set_position(const point& pos) noexcept
	{
		impl_.set_position(std::array<int32_t, 2>{{pos.x, pos.y}});
	}

	auto get_position() const noexcept -> point
	{
		point result;
		auto pos = impl_.get_position();
		result.x = static_cast<int32_t>(pos[0]);
		result.y = static_cast<int32_t>(pos[1]);
		return result;
	}

	auto get_title() const noexcept -> std::string
	{
		return title_;
	}

	void set_title(const std::string& str) noexcept
	{
		title_ = str;
		impl_.set_title(str);
	}

	void show() noexcept
	{
		impl_.set_visible(true);
	}

	void hide() noexcept
	{
		impl_.set_visible(false);
	}

	void maximize() noexcept
	{
		impl_.maximize();
	}

	void minimize() noexcept
	{
		impl_.minimize();
	}

	void restore() noexcept
	{
		impl_.restore();
	}

	void raise() noexcept
	{
        request_focus();
	}

    auto is_minimized() const noexcept -> bool
    {
        return false;
    }

	void set_border(bool /*b*/) noexcept
	{
		// these will invalidate the native_handle

		//		auto sz = get_size();
		//		windowed_mode_.width = sz.w;
		//		windowed_mode_.height = sz.h;

		//		if(b)
		//		{
		//			impl_.create(::mml::video_mode::get_desktop_mode(), title_, style_ &
		//~::mml::style::titlebar);
		//		}
		//		else
		//		{
		//			impl_.create(::mml::video_mode::get_desktop_mode(), title_, style_);
		//		}
	}

	void set_fullscreen(bool /*b*/)
	{
		// these will invalidate the native_handle

		//		if(b)
		//		{
		//			auto sz = get_size();
		//			windowed_mode_.width = sz.w;
		//			windowed_mode_.height = sz.h;
		//			impl_.create(::mml::video_mode::get_desktop_mode(), title_, ::mml::style::none);
		//		}
		//		else
		//		{
		//			impl_.create(windowed_mode_, title_, style_);
		//		}
	}

	void set_opacity(float opacity)
	{
		opacity_ = opacity;
		impl_.set_opacity(opacity);
	}

	auto get_opacity() const -> float
	{
		return opacity_;
	}

	void grab_input(bool b) noexcept
	{
		grabbed_ = b;
		impl_.set_mouse_cursor_grabbed(b);
	}

	bool is_input_grabbed() const noexcept
	{
		return grabbed_;
	}

	void request_focus()
	{
		impl_.request_focus();
	}

	void set_recieved_close_event(bool b) noexcept
	{
		recieved_close_event_ = b;
	}
	auto recieved_close_event() const noexcept -> bool
	{
		return recieved_close_event_;
	}

	void set_cursor(const cursor& c) noexcept
	{
		impl_.set_mouse_cursor(to_cursor_impl(c).get_impl());
	}

	void show_cursor(bool show) noexcept
	{
		impl_.set_mouse_cursor_visible(show);
	}

	void set_icon(const image& img)
	{
		impl_.set_icon(img.size.w, img.size.h, img.pixels.data());
	}

	bool has_focus() const
	{
		return impl_.has_focus();
	}

private:
	::mml::window impl_;
	std::string title_{};
	::mml::video_mode windowed_mode_;
	uint32_t style_{0};
	uint32_t id_{0};
	float opacity_{1.0f};
	bool grabbed_{false};
	bool recieved_close_event_{false};
};
} // namespace mml
} // namespace detail
} // namespace os
