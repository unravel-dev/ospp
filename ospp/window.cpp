#include "window.h"
#include "event.h"

#if defined(SDL_BACKEND)
#include "impl/sdl/window.hpp"
using impl_type = os::detail::sdl::window_impl;
#elif defined(MML_BACKEND)
#include "impl/mml/window.hpp"
using impl_type = os::detail::mml::window_impl;
#elif defined(GLFW_BACKEND)
#include "impl/glfw/window.hpp"
using impl_type = os::detail::glfw::window_impl;
#else
#error "unsupported backend"
#endif

#define this_impl to_impl(impl_.get())

namespace os
{

inline auto to_impl(void* window) -> impl_type*
{
	return reinterpret_cast<impl_type*>(window);
}

window::window(const std::string& title, const display_mode& display, uint32_t flags)
	: window(title, {centered, centered}, {display.w, display.h}, flags)
{
}
window::window(const std::string& title, int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t flags)
	: window(title, {x, y}, {w, h}, flags)
{
}

window::window(const std::string& title, const point& pos, const area& size, uint32_t flags)
	: impl_(std::make_shared<impl_type>(title, pos, size, flags))
{
}

auto window::get_native_handle() const -> native_handle
{
	return this_impl->get_native_handle();
}
auto window::get_native_display() const -> native_display
{
	return this_impl->get_native_display();
}

auto window::is_open() const noexcept -> bool
{
	return impl_ != nullptr;
}

auto window::get_brightness() const noexcept -> float
{
	return this_impl->get_brightness();
}

void window::set_brightness(float bright)
{
	this_impl->set_brightness(bright);
}

auto window::get_id() const noexcept -> uint32_t
{
	return this_impl->get_id();
}

void window::set_size(uint32_t width, uint32_t height) noexcept
{
	set_size({width, height});
}

void window::set_size(const area& size) noexcept
{
	this_impl->set_size(size);
}

auto window::get_size() const noexcept -> area
{
	return this_impl->get_size();
}

void window::set_maximum_size(uint32_t width, uint32_t height) noexcept
{
	set_maximum_size({width, height});
}

void window::set_maximum_size(const area& size) noexcept
{
	this_impl->set_maximum_size(size);
}

auto window::get_maximum_size() const noexcept -> area
{
	return this_impl->get_maximum_size();
}

void window::set_minimum_size(uint32_t width, uint32_t height) noexcept
{
	set_minimum_size({width, height});
}

void window::set_minimum_size(const area& size) noexcept
{
	this_impl->set_minimum_size(size);
}

auto window::get_minimum_size() const noexcept -> area
{
	return this_impl->get_minimum_size();
}

void window::set_position(int32_t x, int32_t y) noexcept
{
	set_position({x, y});
}

void window::set_position(const point& pos) noexcept
{
	this_impl->set_position(pos);
}

auto window::get_position() const noexcept -> point
{
	return this_impl->get_position();
}

auto window::get_title() const noexcept -> std::string
{
	return this_impl->get_title();
}

void window::set_title(const std::string& str) noexcept
{
	this_impl->set_title(str);
}

void window::show() noexcept
{
	this_impl->show();
}

void window::hide() noexcept
{
	this_impl->hide();
}

void window::maximize() noexcept
{
	this_impl->maximize();
}

void window::minimize() noexcept
{
	this_impl->minimize();
}

void window::restore() noexcept
{
	this_impl->restore();
}

void window::raise() noexcept
{
	this_impl->raise();
}

void window::set_border(bool b) noexcept
{
	this_impl->set_border(b);
}

void window::set_fullscreen(bool b)
{
	this_impl->set_fullscreen(b);
}

void window::set_opacity(float opacity)
{
	this_impl->set_opacity(opacity);
}

auto window::get_opacity() const -> float
{
	return this_impl->get_opacity();
}

void window::grab_input(bool grab) noexcept
{
	this_impl->grab_input(grab);
}

bool window::is_input_grabbed() const noexcept
{
	return this_impl->is_input_grabbed();
}

void window::request_focus()
{
	this_impl->request_focus();
}

void window::request_close() noexcept
{
	event e;
	e.type = events::window;
	e.window.type = window_event_id::close;
	e.window.window_id = get_id();
	push_event(e);
}

void window::set_cursor(const cursor& c) noexcept
{
	this_impl->set_cursor(c);
}

void window::show_cursor(bool show) noexcept
{
	this_impl->show_cursor(show);
}

void window::set_icon(const image& img)
{
	this_impl->set_icon(img);
}

auto window::get_impl() const noexcept -> void*
{
	return this_impl;
}
}
