#ifndef MML_WINDOWIMPL_HPP
#define MML_WINDOWIMPL_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <mml/config.hpp>
#include <mml/system/non_copyable.hpp>
#include <mml/window/cursor_impl.hpp>
#include <mml/window/event.hpp>
#include <mml/window/joystick.hpp>
#include <mml/window/joystick_impl.hpp>
#include <mml/window/sensor.hpp>
#include <mml/window/sensor_impl.hpp>
#include <mml/window/video_mode.hpp>
#include <mml/window/window.hpp>
#include <mml/window/window_handle.hpp>

#include <limits>
#include <queue>
#include <set>
#include <string>

namespace mml
{

namespace priv
{
////////////////////////////////////////////////////////////
/// \brief Abstract base class for OS-specific window implementation
///
////////////////////////////////////////////////////////////
class window_impl : non_copyable
{
public:
    constexpr static const auto centered = std::numeric_limits<int32_t>::max();

    ////////////////////////////////////////////////////////////
    /// \brief Create a new window depending on the current OS
    ///
    /// \param mode  Video mode to use
    /// \param title Title of the window
    /// \param style window style
    /// \param settings Additional settings for the underlying OpenGL context
    ///
    /// \return Pointer to the created window (don't forget to delete it)
    ///
    ////////////////////////////////////////////////////////////
    static window_impl* create(video_mode mode, const std::array<std::int32_t, 2>& position,
                               const std::string& title, std::uint32_t style);

    ////////////////////////////////////////////////////////////
    /// \brief Create a new window depending on to the current OS
    ///
    /// \param handle Platform-specific handle of the control
    ///
    /// \return Pointer to the created window (don't forget to delete it)
    ///
    ////////////////////////////////////////////////////////////
    static window_impl* create(window_handle handle);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~window_impl();

    ////////////////////////////////////////////////////////////
    /// \brief Change the joystick threshold, i.e. the value below which
    ///        no move event will be generated
    ///
    /// \param threshold New threshold, in range [0, 100]
    ///
    ////////////////////////////////////////////////////////////
    void set_joystick_threshold(float threshold);

    ////////////////////////////////////////////////////////////
    /// \brief Return the next window event available
    ///
    /// If there's no event available, this function calls the
    /// window's internal event processing function.
    /// The \a block parameter controls the behavior of the function
    /// if no event is available: if it is true then the function
    /// doesn't return until a new event is triggered; otherwise it
    /// returns false to indicate that no event is available.
    ///
    /// \param event platform_event to be returned
    /// \param block Use true to block the thread until an event arrives
    ///
    ////////////////////////////////////////////////////////////
    bool pop_event(platform_event& event, bool block);

    ////////////////////////////////////////////////////////////
    /// \brief Get the OS-specific handle of the window
    ///
    /// \return Handle of the window
    ///
    ////////////////////////////////////////////////////////////
    virtual window_handle native_handle() const = 0;
    virtual void* native_display_handle() const = 0;
    ////////////////////////////////////////////////////////////
    /// \brief Get the position of the window
    ///
    /// \return Position of the window, in pixels
    ///
    ////////////////////////////////////////////////////////////
    virtual std::array<std::int32_t, 2> get_position() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Change the position of the window on screen
    ///
    /// \param position New position of the window, in pixels
    ///
    ////////////////////////////////////////////////////////////
    virtual void set_position(const std::array<std::int32_t, 2>& position) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Get the client size of the window
    ///
    /// \return Size of the window, in pixels
    ///
    ////////////////////////////////////////////////////////////
    virtual std::array<std::uint32_t, 2> get_size() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Change the size of the rendering region of the window
    ///
    /// \param size New size, in pixels
    ///
    ////////////////////////////////////////////////////////////
    virtual void set_size(const std::array<std::uint32_t, 2>& size) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Get window decoration sizes around the client area
    ///
    /// Returns \a left, \a top, \a right, \a bottom in pixels. \a top includes
    /// the title bar. Defaults to zero when unsupported or undecorated.
    ///
    ////////////////////////////////////////////////////////////
    virtual std::array<std::int32_t, 4> get_frame_size() const;

    ////////////////////////////////////////////////////////////
    /// \brief Change the title of the window
    ///
    /// \param title New title
    ///
    ////////////////////////////////////////////////////////////
    virtual void set_title(const std::string& title) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Change the window's icon
    ///
    /// \param width  Icon's width, in pixels
    /// \param height Icon's height, in pixels
    /// \param pixels Pointer to the pixels in memory, format must be RGBA 32 bits
    ///
    ////////////////////////////////////////////////////////////
    virtual void set_icon(unsigned int width, unsigned int height, const std::uint8_t* pixels) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Show or hide the window
    ///
    /// \param visible True to show, false to hide
    ///
    ////////////////////////////////////////////////////////////
    virtual void set_visible(bool visible) = 0;

	virtual void maximize() = 0;
	virtual void minimize() = 0;
	virtual void restore() = 0;
	virtual void set_opacity(float opacity) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Show or hide the mouse cursor
    ///
    /// \param visible True to show, false to hide
    ///
    ////////////////////////////////////////////////////////////
    virtual void set_mouse_cursor_visible(bool visible) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Grab or release the mouse cursor and keeps it from leaving
    ///
    /// \param grabbed True to enable, false to disable
    ///
    ////////////////////////////////////////////////////////////
    virtual void set_mouse_cursor_grabbed(bool grabbed) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Set the displayed cursor to a native system cursor
    ///
    /// \param cursor Native system cursor type to display
    ///
    ////////////////////////////////////////////////////////////
    virtual void set_mouse_cursor(const cursor_impl& cursor) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable automatic key-repeat
    ///
    /// \param enabled True to enable, false to disable
    ///
    ////////////////////////////////////////////////////////////
    virtual void set_key_repeat_enabled(bool enabled) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Request the current window to be made the active
    ///        foreground window
    ///
    ////////////////////////////////////////////////////////////
    virtual void request_focus() = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the window has the input focus
    ///
    /// \return True if window has focus, false otherwise
    ///
    ////////////////////////////////////////////////////////////
    virtual bool has_focus() const = 0;

    virtual bool is_minimized() const;
    virtual float get_display_scale() const;
    /// GLFW-style scale from window DPI (max of axes / 96 on Win32); 1 if unknown.
    virtual float get_content_scale() const;

	////////////////////////////////////////////////////////////
	/// \brief Push a new event into the event queue
	///
	/// This function is to be used by derived classes, to
	/// notify the mml window that a new event was triggered
	/// by the system.
	///
	/// \param event platform_event to push
	///
	////////////////////////////////////////////////////////////
	void push_event(const platform_event& event);

protected:

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    window_impl();

    ////////////////////////////////////////////////////////////
    /// \brief Process incoming events from the operating system
    ///
    ////////////////////////////////////////////////////////////
    virtual void process_events() = 0;

private:

    ////////////////////////////////////////////////////////////
    /// \brief Read the joysticks state and generate the appropriate events
    ///
    ////////////////////////////////////////////////////////////
    void process_joystick_events();

    ////////////////////////////////////////////////////////////
    /// \brief Read the sensors state and generate the appropriate events
    ///
    ////////////////////////////////////////////////////////////
    void process_sensor_events();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    ///< Queue of available events
	std::queue<platform_event> events_;
    ///< Previous state of the joysticks
	joystick_state joystick_states_[joystick::count];
    ///< Previous value of the sensors
	std::array<float, 3> sensor_value_[sensor::count];
	///< joystick threshold (minimum motion for "move" event to be generated)
	float joystick_threshold_;
};

} // namespace priv

} // namespace mml


#endif // MML_WINDOWIMPL_HPP
