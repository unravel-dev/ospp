#include "window.h"
#include "event.h"


namespace os
{
    namespace
    {
        struct impl_type
        {
            std::string title {};
            point p {};
            area a {};
            uint32_t flags {};
        };

        inline auto to_impl(void* window) -> impl_type*
        {
            return reinterpret_cast<impl_type*>(window);
        }

        #define this_impl to_impl(impl_.get())
    }

    window::window(const std::string& title, int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t flags)
        : window(title, point{x, y}, area{w, h}, flags)
    {
    }

    window::window(const std::string& title, const point& pos, const area& size, uint32_t flags)
        : impl_(std::make_shared<impl_type>(impl_type{title, pos, size, flags}))
    {
    }

    auto window::get_native_handle() const -> native_handle
    {
        return {};
    }
    auto window::get_native_display() const -> native_display
    {
        return {};
    }

    auto window::is_open() const noexcept -> bool
    {
        return impl_ != nullptr;
    }

    auto window::get_id() const noexcept -> uint32_t
    {
        return {};
    }

    void window::set_size(uint32_t width, uint32_t height) noexcept
    {
        set_size(area{width, height});
    }

    void window::set_size(const area& size) noexcept
    {
        this_impl->a = size;
    }

    auto window::get_size() const noexcept -> area
    {
        return this_impl->a;
    }

    void window::set_maximum_size(uint32_t width, uint32_t height) noexcept
    {
        set_maximum_size(area{width, height});
    }

    void window::set_maximum_size(const area& size) noexcept
    {
    }

    auto window::get_maximum_size() const noexcept -> area
    {
        return {9999, 9999};
    }

    void window::set_minimum_size(uint32_t width, uint32_t height) noexcept
    {
        set_minimum_size(area{width, height});
    }

    void window::set_minimum_size(const area& size) noexcept
    {
    }

    auto window::get_minimum_size() const noexcept -> area
    {
        return {1, 1};
    }

    void window::set_position(int32_t x, int32_t y) noexcept
    {
        set_position(point{x, y});
    }

    void window::set_position(const point& pos) noexcept
    {
        this_impl->p = pos;
    }

    auto window::get_position() const noexcept -> point
    {
        return this_impl->p;
    }

    auto window::get_title() const noexcept -> std::string
    {
        return this_impl->title;
    }

    void window::set_title(const std::string& str) noexcept
    {
        this_impl->title = str;
    }

    void window::show() noexcept
    {
    }

    void window::hide() noexcept
    {
    }

    void window::maximize() noexcept
    {
    }

    void window::minimize() noexcept
    {
    }

    void window::restore() noexcept
    {
    }

    void window::raise() noexcept
    {
    }

    void window::set_border(bool b) noexcept
    {
    }

    void window::set_fullscreen(bool b)
    {
    }

    void window::set_opacity(float opacity)
    {
    }

    auto window::get_opacity() const -> float
    {
        return {};
    }

    void window::grab_input(bool grab) noexcept
    {
    }

    bool window::is_input_grabbed() const noexcept
    {
        return {};
    }

    void window::request_focus()
    {
    }

    bool window::has_focus() const noexcept
    {
        return {};
    }

    void window::request_close() noexcept
    {
        event e{};
        e.type = events::window;
        e.window.type = window_event_id::close;
        e.window.window_id = get_id();
        push_event(std::move(e));
    }

    void window::set_cursor(const cursor& c) noexcept
    {
    }

    void window::set_cursor(cursor::type type) noexcept
    {
        set_cursor(get_system_cursor(type));
    }

    void window::show_cursor(bool show) noexcept
    {
    }

    void window::set_icon(const image& img)
    {
    }

    auto window::get_impl() const noexcept -> void*
    {
        return {};
    }
}
