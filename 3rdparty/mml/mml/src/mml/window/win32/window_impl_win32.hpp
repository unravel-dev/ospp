#ifndef MML_WINDOWIMPLWIN32_HPP
#define MML_WINDOWIMPLWIN32_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <mml/window/event.hpp>
#include <mml/window/window_impl.hpp>
#include <windows.h>


namespace mml
{
namespace priv
{
////////////////////////////////////////////////////////////
/// \brief Windows implementation of WindowImpl
///
////////////////////////////////////////////////////////////
class window_impl_win32 : public window_impl
{
public:

    ////////////////////////////////////////////////////////////
    /// \brief Construct the window implementation from an existing control
    ///
    /// \param handle Platform-specific handle of the control
    ///
    ////////////////////////////////////////////////////////////
    window_impl_win32(window_handle handle);

    ////////////////////////////////////////////////////////////
    /// \brief Create the window implementation
    ///
    /// \param mode  Video mode to use
    /// \param title Title of the window
    /// \param style window style
    /// \param settings Additional settings for the underlying OpenGL context
    ///
    ////////////////////////////////////////////////////////////
    window_impl_win32(video_mode mode, const std::array<std::int32_t, 2>& position,
                      const std::string& title, std::uint32_t style);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~window_impl_win32() override;

    ////////////////////////////////////////////////////////////
    /// \brief Get the OS-specific handle of the window
    ///
    /// \return Handle of the window
    ///
    ////////////////////////////////////////////////////////////
    window_handle native_handle() const final;
	void* native_display_handle() const final;
    ////////////////////////////////////////////////////////////
    /// \brief Get the position of the window
    ///
    /// \return Position of the window, in pixels
    ///
    ////////////////////////////////////////////////////////////
    std::array<std::int32_t, 2> get_position() const final;

    ////////////////////////////////////////////////////////////
    /// \brief Change the position of the window on screen
    ///
    /// \param position New position of the window, in pixels
    ///
    ////////////////////////////////////////////////////////////
    void set_position(const std::array<std::int32_t, 2>& position) final;

    ////////////////////////////////////////////////////////////
    /// \brief Get the client size of the window
    ///
    /// \return Size of the window, in pixels
    ///
    ////////////////////////////////////////////////////////////
    std::array<std::uint32_t, 2> get_size() const final;

    ////////////////////////////////////////////////////////////
    /// \brief Change the size of the rendering region of the window
    ///
    /// \param size New size, in pixels
    ///
    ////////////////////////////////////////////////////////////
    void set_size(const std::array<std::uint32_t, 2>& size) final;

    std::array<std::int32_t, 4> get_frame_size() const final;

    ////////////////////////////////////////////////////////////
    /// \brief Change the title of the window
    ///
    /// \param title New title
    ///
    ////////////////////////////////////////////////////////////
    void set_title(const std::string& title) final;

    ////////////////////////////////////////////////////////////
    /// \brief Change the window's icon
    ///
    /// \param width  Icon's width, in pixels
    /// \param height Icon's height, in pixels
    /// \param pixels Pointer to the pixels in memory, format must be RGBA 32 bits
    ///
    ////////////////////////////////////////////////////////////
    void set_icon(unsigned int width, unsigned int height, const std::uint8_t* pixels) final;

    ////////////////////////////////////////////////////////////
    /// \brief Show or hide the window
    ///
    /// \param visible True to show, false to hide
    ///
    ////////////////////////////////////////////////////////////
    void set_visible(bool visible) final;

	void maximize() final;
	void minimize() final;
	void restore() final;
	void set_opacity(float opacity) final;

    ////////////////////////////////////////////////////////////
    /// \brief Show or hide the mouse cursor
    ///
    /// \param visible True to show, false to hide
    ///
    ////////////////////////////////////////////////////////////
    void set_mouse_cursor_visible(bool visible) final;

    ////////////////////////////////////////////////////////////
    /// \brief Grab or release the mouse cursor
    ///
    /// \param grabbed True to enable, false to disable
    ///
    ////////////////////////////////////////////////////////////
    void set_mouse_cursor_grabbed(bool grabbed) final;

    ////////////////////////////////////////////////////////////
    /// \brief Set the displayed cursor to a native system cursor
    ///
    /// \param cursor Native system cursor type to display
    ///
    ////////////////////////////////////////////////////////////
    void set_mouse_cursor(const cursor_impl& cursor) final;

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable automatic key-repeat
    ///
    /// \param enabled True to enable, false to disable
    ///
    ////////////////////////////////////////////////////////////
    void set_key_repeat_enabled(bool enabled) final;

    ////////////////////////////////////////////////////////////
    /// \brief Request the current window to be made the active
    ///        foreground window
    ///
    ////////////////////////////////////////////////////////////
    void request_focus() final;

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the window has the input focus
    ///
    /// \return True if window has focus, false otherwise
    ///
    ////////////////////////////////////////////////////////////
    bool has_focus() const final;

    bool is_minimized() const final;
    float get_content_scale() const final;

protected:

    ////////////////////////////////////////////////////////////
    /// \brief Process incoming events from the operating system
    ///
    ////////////////////////////////////////////////////////////
    void process_events() final;

private:

    ////////////////////////////////////////////////////////////
    /// Register the window class
    ///
    ////////////////////////////////////////////////////////////
    void register_window_class();

    ////////////////////////////////////////////////////////////
    /// \brief Switch to fullscreen mode
    ///
    /// \param mode Video mode to switch to
    ///
    ////////////////////////////////////////////////////////////
    void switch_to_fullscreen(const video_mode& mode);

    ////////////////////////////////////////////////////////////
    /// \brief Free all the graphical resources attached to the window
    ///
    ////////////////////////////////////////////////////////////
    void cleanup();

    ////////////////////////////////////////////////////////////
    /// \brief Process a Win32 event
    ///
    /// \param message Message to process
    /// \param wParam  First parameter of the event
    /// \param lParam  Second parameter of the event
    ///
    ////////////////////////////////////////////////////////////
    void process_event(UINT message, WPARAM wParam, LPARAM lParam);

    ////////////////////////////////////////////////////////////
    /// \brief Enables or disables tracking for the mouse cursor leaving the window
    ///
    /// \param track True to enable, false to disable
    ///
    ////////////////////////////////////////////////////////////
    void set_tracking(bool track);

    ////////////////////////////////////////////////////////////
    /// \brief Grab or release the mouse cursor
    ///
    /// This is not to be confused with set_mouse_cursor_grabbed.
    /// Here _cursor_grabbed is not modified; it is used,
    /// for example, to release the cursor when switching to
    /// another application.
    ///
    /// \param grabbed True to enable, false to disable
    ///
    ////////////////////////////////////////////////////////////
    void grab_cursor(bool grabbed);

    ////////////////////////////////////////////////////////////
    /// \brief Convert a Win32 virtual key code to a mml key code
    ///
    /// \param key   Virtual key code to convert
    /// \param flags Additional flags
    ///
    /// \return mml key code corresponding to the key
    ///
    ////////////////////////////////////////////////////////////
    static keyboard::key virtual_key_code_to_mml(WPARAM key, LPARAM flags);

    ////////////////////////////////////////////////////////////
    /// \brief Convert a Win32 scancode to an mml scancode
    ///
    /// \param flags input flags
    ///
    /// \return MML scancode corresponding to the key
    ///
    ////////////////////////////////////////////////////////////
    static keyboard::scancode to_scancode(WPARAM wParam, LPARAM lParam);

    ////////////////////////////////////////////////////////////
    /// \brief Function called whenever one of our windows receives a message
    ///
    /// \param handle  Win32 handle of the window
    /// \param message Message received
    /// \param wParam  First parameter of the message
    /// \param lParam  Second parameter of the message
    ///
    /// \return True to discard the event after it has been processed
    ///
    ////////////////////////////////////////////////////////////
    static LRESULT CALLBACK global_on_event(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
	///< Win32 handle of the window
	HWND handle_;
	///< Stores the original event callback function of the control
	LONG_PTR callback_;
	///< Is the cursor visible or hidden?
	bool cursor_visible_;
	///< Last cursor used -- this data is not owned by the window and is required to be always valid
	HCURSOR last_cursor_;
	///< Custom icon assigned to the window
	HICON icon_;
	///< Automatic key-repeat state for keydown events
	bool key_repeat_enabled_;
	///< The last handled size of the window
	std::array<std::uint32_t, 2> last_size_;
	///< Is the window being resized?
	bool resizing_;
	///< First half of the surrogate pair, in case we're receiving a Unicode character in two events
	std::uint16_t surrogate_;
	///< mouse is inside the window?
	bool mouse_inside_;
	///< Is the window fullscreen?
	bool fullscreen_;
	///< Is the mouse cursor trapped?
	bool cursor_grabbed_;
};

} // namespace priv

} // namespace mml

#endif // MML_WINDOWIMPLWIN32_HPP
