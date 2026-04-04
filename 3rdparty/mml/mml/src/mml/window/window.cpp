////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <mml/window/window.hpp>
#include <mml/window/window_impl.hpp>
#include <mml/system/err.hpp>
#include <cstring>

namespace
{
    const mml::window* fullscreen_window = nullptr;
}


namespace mml
{
////////////////////////////////////////////////////////////
window::window() :
impl_          (nullptr),
size_          ({{0, 0}}),
visible_(false)
{

}


////////////////////////////////////////////////////////////
window::window(video_mode mode, const std::array<std::int32_t, 2>& position,
               const std::string& title, std::uint32_t style)
    : impl_(nullptr)
    , size_({{0, 0}})
    , visible_(true)
{
    create(mode, position, title, style);
}


////////////////////////////////////////////////////////////
window::window(window_handle handle)
    : impl_          (nullptr)
    , size_          ({{0, 0}})
    , visible_(true)
{
    create(handle);
}


////////////////////////////////////////////////////////////
window::~window()
{
    dispose();
}


////////////////////////////////////////////////////////////
void window::create(video_mode mode, const std::array<std::int32_t, 2>& position,
                    const std::string& title, std::uint32_t style)
{
    // Destroy the previous window implementation
    dispose();

    // fullscreen style requires some tests
    if (style & style::fullscreen)
    {
        // Make sure there's not already a fullscreen window (only one is allowed)
        if (fullscreen_window)
        {
            err() << "Creating two fullscreen windows is not allowed, switching to windowed mode" << std::endl;
            style &= ~style::fullscreen;
        }
        else
        {
            auto modes = video_mode::get_desktop_modes(0);
            bool valid = std::find(modes.begin(), modes.end(), mode) != modes.end();
            // Make sure that the chosen video mode is compatible
            if (!valid)
            {
                err() << "The requested video mode is not available, switching to a valid mode" << std::endl;
                mode = modes.at(0);
            }

            // Update the fullscreen window
            fullscreen_window = this;
        }
    }

    // Check validity of style according to the underlying platform
    #if defined(MML_SYSTEM_IOS) || defined(MML_SYSTEM_ANDROID)
        if (style & style::fullscreen)
            style &= ~style::titlebar;
        else
            style |= style::titlebar;
    #else
        if ((style & style::close) || (style & style::resize))
            style |= style::titlebar;
    #endif

    // Recreate the window implementation
    impl_ = priv::window_impl::create(mode, position, title, style);

    // Perform common initializations
    initialize();
}


////////////////////////////////////////////////////////////
void window::create(window_handle handle)
{
    // Destroy the previous window implementation
    dispose();

    // Recreate the window implementation
    impl_ = priv::window_impl::create(handle);

    // Perform common initializations
    initialize();
}


////////////////////////////////////////////////////////////
void window::dispose()
{
    // Delete the window implementation
    delete impl_;
    impl_ = nullptr;

    // Update the fullscreen window
    if (this == fullscreen_window)
        fullscreen_window = nullptr;
}

////////////////////////////////////////////////////////////
void window::request_close()
{
	if (impl_)
	{
		platform_event e;
		std::memset(&e, 0, sizeof(e));
		e.type = platform_event::closed;
		impl_->push_event(e);
	}
}
////////////////////////////////////////////////////////////
bool window::is_open() const
{
    return impl_ != nullptr;
}

////////////////////////////////////////////////////////////
bool window::poll_event(platform_event& event)
{
    if (impl_ && impl_->pop_event(event, false))
    {
        return filter_event(event);
    }
    else
    {
        return false;
    }
}


////////////////////////////////////////////////////////////
bool window::wait_event(platform_event& event)
{
    if (impl_ && impl_->pop_event(event, true))
    {
        return filter_event(event);
    }
    else
    {
        return false;
    }
}


////////////////////////////////////////////////////////////
std::array<std::int32_t, 2> window::get_position() const
{
    return impl_ ? impl_->get_position() : std::array<std::int32_t, 2>();
}


////////////////////////////////////////////////////////////
void window::set_position(const std::array<std::int32_t, 2>& position)
{
    if (impl_)
        impl_->set_position(position);
}


////////////////////////////////////////////////////////////
std::array<std::uint32_t, 2> window::get_size() const
{
    return size_;
}


////////////////////////////////////////////////////////////
void window::set_size(const std::array<std::uint32_t, 2>& size)
{
    if (impl_)
    {
        if(size_[0] != size[0] || size_[1] != size[1])
        {
            impl_->set_size(size);

            // Cache the new size
            size_ = size;

            // Notify the derived class
            on_resize();

        }
    }
}


////////////////////////////////////////////////////////////
std::array<std::int32_t, 4> window::get_frame_size() const
{
    if(!impl_)
    {
        return {{0, 0, 0, 0}};
    }
    return impl_->get_frame_size();
}


////////////////////////////////////////////////////////////
void window::set_title(const std::string& title)
{
    if (impl_)
        impl_->set_title(title);
}


////////////////////////////////////////////////////////////
void window::set_icon(unsigned int width, unsigned int height, const std::uint8_t* pixels)
{
    if (impl_)
        impl_->set_icon(width, height, pixels);
}


////////////////////////////////////////////////////////////
void window::set_visible(bool visible)
{
    if (impl_)
        impl_->set_visible(visible);
    
    visible_ = visible;
}

////////////////////////////////////////////////////////////
bool window::is_visible() const
{
    return visible_;
}

////////////////////////////////////////////////////////////
void window::maximize()
{
	if (impl_)
		impl_->maximize();
}

////////////////////////////////////////////////////////////
void window::minimize()
{
	if (impl_)
		impl_->minimize();
}

////////////////////////////////////////////////////////////
void window::restore()
{
	if (impl_)
		impl_->restore();
}

////////////////////////////////////////////////////////////
void window::set_opacity(float opacity)
{
	if (impl_)
		impl_->set_opacity(opacity);
}

////////////////////////////////////////////////////////////
void window::set_mouse_cursor_visible(bool visible)
{
    if (impl_)
        impl_->set_mouse_cursor_visible(visible);
}


////////////////////////////////////////////////////////////
void window::set_mouse_cursor_grabbed(bool grabbed)
{
    if (impl_)
        impl_->set_mouse_cursor_grabbed(grabbed);
}


////////////////////////////////////////////////////////////
void window::set_mouse_cursor(const cursor& cursor)
{
    if (impl_)
        impl_->set_mouse_cursor(cursor.get_impl());
}


////////////////////////////////////////////////////////////
void window::set_key_repeat_enabled(bool enabled)
{
    if (impl_)
        impl_->set_key_repeat_enabled(enabled);
}

////////////////////////////////////////////////////////////
void window::set_joystick_threshold(float threshold)
{
    if (impl_)
        impl_->set_joystick_threshold(threshold);
}


////////////////////////////////////////////////////////////
void window::request_focus()
{
    if (impl_)
        impl_->request_focus();
}


////////////////////////////////////////////////////////////
bool window::has_focus() const
{
    return impl_ && impl_->has_focus();
}


////////////////////////////////////////////////////////////
bool window::is_minimized() const
{
    return impl_ && impl_->is_minimized();
}


////////////////////////////////////////////////////////////
float window::get_display_scale() const
{
    if(!impl_)
    {
        return 1.f;
    }
    return impl_->get_display_scale();
}


////////////////////////////////////////////////////////////
float window::get_content_scale() const
{
    if(!impl_)
    {
        return 1.f;
    }
    return impl_->get_content_scale();
}


////////////////////////////////////////////////////////////
window_handle window::native_handle() const
{
    return impl_ ? impl_->native_handle() : window_handle{};
}
void* window::native_display_handle() const
{
    return impl_ ? impl_->native_display_handle() : nullptr;
}
////////////////////////////////////////////////////////////
void window::on_create()
{
    // Nothing by default
}


////////////////////////////////////////////////////////////
void window::on_resize()
{
    // Nothing by default
}


////////////////////////////////////////////////////////////
bool window::filter_event(const platform_event& event)
{
    // Notify resize events to the derived class
    if (event.type == platform_event::resized)
    {
        if(size_[0] != event.size.width || size_[1] != event.size.height)
        {
            // Cache the new size
            size_ = { {event.size.width, event.size.height} };
    
            // Notify the derived class
            on_resize();
        }      
    }

    return true;
}


////////////////////////////////////////////////////////////
void window::initialize()
{
    // Setup default behaviors (to get a consistent behavior across different implementations)
    set_mouse_cursor_visible(true);
    set_key_repeat_enabled(true);

    // Get and cache the initial size of the window
    size_ = impl_->get_size();

    // Notify the derived class
    on_create();
}

} // namespace mml
