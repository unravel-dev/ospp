////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#ifdef _WIN32_WINDOWS
    #undef _WIN32_WINDOWS
#endif
#ifdef _WIN32_WINNT
    #undef _WIN32_WINNT
#endif
#define _WIN32_WINDOWS 0x0501
#define _WIN32_WINNT   0x0501
#define WINVER         0x0501
#include <mml/window/win32/window_impl_win32.hpp>
#include <mml/window/window_style.hpp>
#include <mml/system/err.hpp>
#include <mml/system/utf.hpp>
#include <algorithm>
#include <vector>
#include <cstring>
#include <iterator>
#include <dbt.h>
// MinGW lacks the definition of some Win32 constants
#ifndef XBUTTON1
    #define XBUTTON1 0x0001
#endif
#ifndef XBUTTON2
    #define XBUTTON2 0x0002
#endif
#ifndef WM_MOUSEHWHEEL
    #define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef MAPVK_VK_TO_VSC
    #define MAPVK_VK_TO_VSC (0)
#endif

namespace
{
    unsigned int               window_count      = 0; // Windows owned by mml
    unsigned int               handle_count      = 0; // All window handles
    const wchar_t*             class_name        = L"MML_Window";
    mml::priv::window_impl_win32* fullscreenWindow = nullptr;

    void set_process_dpi_aware()
    {
        // Try SetProcessDpiAwareness first
        HINSTANCE shCoreDll = LoadLibraryW(L"Shcore.dll");

        if (shCoreDll)
        {
            enum ProcessDpiAwareness
            {
                ProcessDpiUnaware         = 0,
                ProcessSystemDpiAware     = 1,
                ProcessPerMonitorDpiAware = 2
            };

            typedef HRESULT (WINAPI* SetProcessDpiAwarenessFuncType)(ProcessDpiAwareness);
            SetProcessDpiAwarenessFuncType SetProcessDpiAwarenessFunc = reinterpret_cast<SetProcessDpiAwarenessFuncType>(GetProcAddress(shCoreDll, "SetProcessDpiAwareness"));

            if (SetProcessDpiAwarenessFunc)
            {
                // We only check for E_INVALIDARG because we would get
                // E_ACCESSDENIED if the DPI was already set previously
                // and S_OK means the call was successful
                if (SetProcessDpiAwarenessFunc(ProcessSystemDpiAware) == E_INVALIDARG)
                {
                    mml::err() << "Failed to set process DPI awareness" << std::endl;
                }
                else
                {
                    FreeLibrary(shCoreDll);
                    return;
                }
            }

            FreeLibrary(shCoreDll);
        }

        // Fall back to SetProcessDPIAware if SetProcessDpiAwareness
        // is not available on this system
        HINSTANCE user32Dll = LoadLibraryW(L"user32.dll");

        if (user32Dll)
        {
            typedef BOOL (WINAPI* SetProcessDPIAwareFuncType)(void);
            SetProcessDPIAwareFuncType SetProcessDPIAwareFunc = reinterpret_cast<SetProcessDPIAwareFuncType>(GetProcAddress(user32Dll, "SetProcessDPIAware"));

            if (SetProcessDPIAwareFunc)
            {
                if (!SetProcessDPIAwareFunc())
                    mml::err() << "Failed to set process DPI awareness" << std::endl;
            }

            FreeLibrary(user32Dll);
        }
    }
}

namespace mml
{
namespace priv
{
////////////////////////////////////////////////////////////
window_impl_win32::window_impl_win32(window_handle handle) :
handle_          (handle),
callback_        (0),
cursor_visible_   (true), // might need to call GetCursorInfo
last_cursor_      (LoadCursor(nullptr, IDC_ARROW)),
icon_            (nullptr),
key_repeat_enabled_(true),
last_size_        ({0, 0}),
resizing_        (false),
surrogate_       (0),
mouse_inside_     (false),
fullscreen_      (false),
cursor_grabbed_   (false)
{
    // Set that this process is DPI aware and can handle DPI scaling
    set_process_dpi_aware();

    if (handle_)
    {
        // If we're the first window handle, we only need to poll for joysticks when WM_DEVICECHANGE message is received
        if (handle_count == 0)
            joystick_impl::set_lazy_updates(true);

        ++handle_count;

        // We change the event procedure of the control (it is important to save the old one)
        SetWindowLongPtrW(handle_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        callback_ = SetWindowLongPtrW(handle_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&window_impl_win32::global_on_event));
    }
}


////////////////////////////////////////////////////////////
window_impl_win32::window_impl_win32(video_mode mode,
                                     const std::array<std::int32_t, 2>& position,
                                     const std::string& title,
                                     std::uint32_t style) :
handle_          (nullptr),
callback_        (0),
cursor_visible_   (true), // might need to call GetCursorInfo
last_cursor_      (LoadCursor(nullptr, IDC_ARROW)),
icon_            (nullptr),
key_repeat_enabled_(true),
last_size_        ({mode.width, mode.height}),
resizing_        (false),
surrogate_       (0),
mouse_inside_     (false),
fullscreen_      ((style & style::fullscreen) != 0),
cursor_grabbed_   (fullscreen_)
{
    // Set that this process is DPI aware and can handle DPI scaling
    set_process_dpi_aware();

    // Register the window class at first call
    if (window_count == 0)
        register_window_class();

    // Compute position and size
    HDC screenDC = GetDC(nullptr);

    int x = position[0];
    int y  = position[1];
    if(x == centered)
    {
        x = (GetDeviceCaps(screenDC, HORZRES) - static_cast<int>(mode.width))  / 2;
    }
    if(y == centered)
    {
        y = (GetDeviceCaps(screenDC, VERTRES) - static_cast<int>(mode.height)) / 2;
    }
    int width  = int(mode.width);
    int height = int(mode.height);
    ReleaseDC(nullptr, screenDC);

    // Choose the window style according to the style parameter
    bool hidden = (style & style::hidden) != 0;

    DWORD win32Style = 0;
    if(!hidden)
    {
        win32Style |= WS_VISIBLE;
    }
    if (style == style::none)
    {
        win32Style |= WS_POPUP;
    }
    else
    {
        if (style & style::titlebar) win32Style |= WS_CAPTION | WS_MINIMIZEBOX;
        if (style & style::resize)   win32Style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
        if (style & style::close)    win32Style |= WS_SYSMENU;
        
    }

    // In windowed mode, adjust width/height for the client area and convert (x, y) from client
    // top-left in screen space to the outer rect origin expected by CreateWindowW (SDL/GLFW semantics).
    if (!fullscreen_)
    {
		RECT rectangle = {0, 0, width, height};
        AdjustWindowRect(&rectangle, win32Style, false);
        x += static_cast<int>(rectangle.left);
        y += static_cast<int>(rectangle.top);
        width  = rectangle.right - rectangle.left;
        height = rectangle.bottom - rectangle.top;
    }
	std::wstring wtitle;
	wtitle.reserve(title.length() + 1);
	// Convert
	utf8::to_wide(title.begin(), title.end(), std::back_inserter(wtitle), 0);
    // Create the window
    handle_ = CreateWindowW(class_name, wtitle.c_str(), win32Style, x, y, width, height, nullptr, nullptr, GetModuleHandle(nullptr), this);
    // Register to receive device interface change notifications (used for joystick connection handling)
    DEV_BROADCAST_HDR deviceBroadcastHeader = {sizeof(DEV_BROADCAST_HDR), DBT_DEVTYP_DEVICEINTERFACE, 0};
    RegisterDeviceNotification(handle_, &deviceBroadcastHeader, DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);


    // If we're the first window handle, we only need to poll for joysticks when WM_DEVICECHANGE message is received
    if (handle_)
    {
        if (handle_count == 0)
            joystick_impl::set_lazy_updates(true);

        ++handle_count;
    }

    // By default, the OS limits the size of the window the the desktop size,
    // we have to resize it after creation to apply the real size
	set_size(std::array<std::uint32_t, 2>({ mode.width, mode.height }));

    // Switch to fullscreen if requested
    if (fullscreen_)
        switch_to_fullscreen(mode);

    // Increment window count
    window_count++;
}


////////////////////////////////////////////////////////////
window_impl_win32::~window_impl_win32()
{
    // TODO should we restore the cursor shape and visibility?

    // Destroy the custom icon, if any
    if (icon_)
        DestroyIcon(icon_);

    // If it's the last window handle we have to poll for joysticks again
    if (handle_)
    {
        --handle_count;

        if (handle_count == 0)
            joystick_impl::set_lazy_updates(false);
    }

    if (!callback_)
    {
        // Destroy the window
        if (handle_)
            DestroyWindow(handle_);

        // Decrement the window count
        window_count--;

        // Unregister window class if we were the last window
        if (window_count == 0)
            UnregisterClassW(class_name, GetModuleHandleW(nullptr));
    }
    else
    {
        // The window is external: remove the hook on its message callback
        SetWindowLongPtrW(handle_, GWLP_WNDPROC, callback_);
    }
}


////////////////////////////////////////////////////////////
window_handle window_impl_win32::native_handle() const
{
    return handle_;
}

void* window_impl_win32::native_display_handle() const
{
    return GetDC(handle_);
}


////////////////////////////////////////////////////////////
void window_impl_win32::process_events()
{
    // We process the window events only if we own it
    if (!callback_)
    {
        MSG message;
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
}


////////////////////////////////////////////////////////////
std::array<std::int32_t, 2> window_impl_win32::get_position() const
{
    if(!handle_)
    {
        return {{0, 0}};
    }
    POINT client_origin{0, 0};
    if(ClientToScreen(handle_, &client_origin) == 0)
    {
        return {{0, 0}};
    }
    return std::array<std::int32_t, 2>(
        {static_cast<std::int32_t>(client_origin.x), static_cast<std::int32_t>(client_origin.y)});
}


////////////////////////////////////////////////////////////
void window_impl_win32::set_position(const std::array<std::int32_t, 2>& position)
{
    if(!handle_)
    {
        return;
    }
    RECT window_rect{};
    if(GetWindowRect(handle_, &window_rect) == 0)
    {
        return;
    }
    POINT client_origin{0, 0};
    if(ClientToScreen(handle_, &client_origin) == 0)
    {
        return;
    }
    const int offset_x = static_cast<int>(client_origin.x - window_rect.left);
    const int offset_y = static_cast<int>(client_origin.y - window_rect.top);
    SetWindowPos(handle_, nullptr, static_cast<int>(position[0]) - offset_x,
                 static_cast<int>(position[1]) - offset_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    if(cursor_grabbed_)
        grab_cursor(true);
}


////////////////////////////////////////////////////////////
std::array<std::uint32_t, 2> window_impl_win32::get_size() const
{
    RECT rect;
    GetClientRect(handle_, &rect);

	return std::array<std::uint32_t, 2>
		({ static_cast<std::uint32_t>(rect.right - rect.left), static_cast<std::uint32_t>(rect.bottom - rect.top) });
}


////////////////////////////////////////////////////////////
void window_impl_win32::set_size(const std::array<std::uint32_t, 2>& size)
{
    // SetWindowPos wants the total size of the window (including title bar and borders),
    // so we have to compute it
    RECT rectangle = {0, 0, static_cast<long>(size[0]), static_cast<long>(size[1])};
    AdjustWindowRect(&rectangle, GetWindowLong(handle_, GWL_STYLE), false);
    int width  = rectangle.right - rectangle.left;
    int height = rectangle.bottom - rectangle.top;

    SetWindowPos(handle_, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
}


////////////////////////////////////////////////////////////
std::array<std::int32_t, 4> window_impl_win32::get_frame_size() const
{
    if(!handle_)
    {
        return {{0, 0, 0, 0}};
    }
    RECT window_rect{};
    RECT client_rect{};
    if(GetWindowRect(handle_, &window_rect) == 0 || GetClientRect(handle_, &client_rect) == 0)
    {
        return {{0, 0, 0, 0}};
    }
    POINT client_origin{0, 0};
    if(ClientToScreen(handle_, &client_origin) == 0)
    {
        return {{0, 0, 0, 0}};
    }
    const LONG client_w = client_rect.right - client_rect.left;
    const LONG client_h = client_rect.bottom - client_rect.top;
    const std::int32_t left = static_cast<std::int32_t>(client_origin.x - window_rect.left);
    const std::int32_t top = static_cast<std::int32_t>(client_origin.y - window_rect.top);
    const std::int32_t right = static_cast<std::int32_t>(window_rect.right - (client_origin.x + client_w));
    const std::int32_t bottom = static_cast<std::int32_t>(window_rect.bottom - (client_origin.y + client_h));
    return {{left, top, right, bottom}};
}


////////////////////////////////////////////////////////////
void window_impl_win32::set_title(const std::string& title)
{
	std::wstring wtitle;
	wtitle.reserve(title.length() + 1);

	// Convert
	utf8::to_wide(title.begin(), title.end(), std::back_inserter(wtitle), 0);
    SetWindowTextW(handle_, wtitle.c_str());
}


////////////////////////////////////////////////////////////
void window_impl_win32::set_icon(unsigned int width, unsigned int height, const std::uint8_t* pixels)
{
    // First destroy the previous one
    if (icon_)
        DestroyIcon(icon_);

    // Windows wants BGRA pixels: swap red and blue channels
    std::vector<std::uint8_t> iconPixels(width * height * 4);
    for (std::size_t i = 0; i < iconPixels.size() / 4; ++i)
    {
        iconPixels[i * 4 + 0] = pixels[i * 4 + 2];
        iconPixels[i * 4 + 1] = pixels[i * 4 + 1];
        iconPixels[i * 4 + 2] = pixels[i * 4 + 0];
        iconPixels[i * 4 + 3] = pixels[i * 4 + 3];
    }

    // Create the icon from the pixel array
    icon_ = CreateIcon(GetModuleHandleW(nullptr), width, height, 1, 32, nullptr, &iconPixels[0]);

    // Set it as both big and small icon of the window
    if (icon_)
    {
        SendMessageW(handle_, WM_SETICON, ICON_BIG,   (LPARAM)icon_);
        SendMessageW(handle_, WM_SETICON, ICON_SMALL, (LPARAM)icon_);
    }
    else
    {
        err() << "Failed to set the window's icon" << std::endl;
    }
}


////////////////////////////////////////////////////////////
void window_impl_win32::set_visible(bool visible)
{
    ShowWindow(handle_, visible ? SW_SHOW : SW_HIDE);
}

////////////////////////////////////////////////////////////
void window_impl_win32::maximize()
{
	ShowWindow(handle_, SW_MAXIMIZE);
}

////////////////////////////////////////////////////////////
void window_impl_win32::minimize()
{
	ShowWindow(handle_, SW_MINIMIZE);
}

////////////////////////////////////////////////////////////
void window_impl_win32::restore()
{
	ShowWindow(handle_, SW_RESTORE);
}

////////////////////////////////////////////////////////////
bool window_impl_win32::is_minimized() const
{
	return handle_ != nullptr && IsIconic(handle_) != FALSE;
}

////////////////////////////////////////////////////////////
float window_impl_win32::get_content_scale() const
{
	if(!handle_)
	{
		return 1.f;
	}
	using GetDpiForWindowFn = UINT(WINAPI*)(HWND);
	static auto get_dpi_for_window = reinterpret_cast<GetDpiForWindowFn>(
		GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForWindow"));
	if(get_dpi_for_window)
	{
		const UINT dpi = get_dpi_for_window(handle_);
		if(dpi > 0)
		{
			return static_cast<float>(dpi) / 96.f;
		}
	}
	HDC hdc = GetDC(handle_);
	if(!hdc)
	{
		return 1.f;
	}
	const int dpi_x = GetDeviceCaps(hdc, LOGPIXELSX);
	const int dpi_y = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(handle_, hdc);
	const int dpi = std::max(dpi_x > 0 ? dpi_x : 96, dpi_y > 0 ? dpi_y : 96);
	return static_cast<float>(dpi) / 96.f;
}

////////////////////////////////////////////////////////////
void make_transparent(HWND hwnd)
{
	SetWindowLongPtrW(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
}
void make_opaque(HWND hwnd)
{
	SetWindowLongPtrW(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
	RedrawWindow(hwnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
}
void window_impl_win32::set_opacity(float opacity)
{
	HWND hwnd = native_handle();
	if (opacity >= 1.0f)
	{
		make_opaque(hwnd);
	}
	else
	{
		make_transparent(hwnd);
        const BYTE alpha = (BYTE) ((int) (opacity * 255.0f));
        SetLayeredWindowAttributes(native_handle(), 0, alpha, LWA_ALPHA);
	}
}


////////////////////////////////////////////////////////////
void window_impl_win32::set_mouse_cursor_visible(bool visible)
{
    cursor_visible_ = visible;
    SetCursor(cursor_visible_ ? last_cursor_ : nullptr);
}


////////////////////////////////////////////////////////////
void window_impl_win32::set_mouse_cursor_grabbed(bool grabbed)
{
    cursor_grabbed_ = grabbed;
    grab_cursor(cursor_grabbed_);
}


////////////////////////////////////////////////////////////
void window_impl_win32::set_mouse_cursor(const cursor_impl& cursor)
{
    last_cursor_ = cursor.cursor_;
    SetCursor(cursor_visible_ ? last_cursor_ : nullptr);
}


////////////////////////////////////////////////////////////
void window_impl_win32::set_key_repeat_enabled(bool enabled)
{
    key_repeat_enabled_ = enabled;
}


////////////////////////////////////////////////////////////
void window_impl_win32::request_focus()
{
    // Allow focus stealing only within the same process; compare PIDs of current and foreground window
    DWORD thisPid       = GetWindowThreadProcessId(handle_, nullptr);
    DWORD foregroundPid = GetWindowThreadProcessId(GetForegroundWindow(), nullptr);

    if (thisPid == foregroundPid)
    {
        // The window requesting focus belongs to the same process as the current window: steal focus
        SetForegroundWindow(handle_);
    }
    else
    {
        // Different process: don't steal focus, but create a taskbar notification ("flash")
        FLASHWINFO info;
        info.cbSize    = sizeof(info);
        info.hwnd      = handle_;
        info.dwFlags   = FLASHW_TRAY;
        info.dwTimeout = 0;
        info.uCount    = 3;

        FlashWindowEx(&info);
    }
}


////////////////////////////////////////////////////////////
bool window_impl_win32::has_focus() const
{
    return handle_ == GetForegroundWindow();
}


////////////////////////////////////////////////////////////
void window_impl_win32::register_window_class()
{
    WNDCLASSW windowClass;
    windowClass.style         = 0;
    windowClass.lpfnWndProc   = &window_impl_win32::global_on_event;
    windowClass.cbClsExtra    = 0;
    windowClass.cbWndExtra    = 0;
    windowClass.hInstance     = GetModuleHandleW(nullptr);
    windowClass.hIcon         = nullptr;
    windowClass.hCursor       = nullptr;
    windowClass.hbrBackground = nullptr;
    windowClass.lpszMenuName  = nullptr;
    windowClass.lpszClassName = class_name;
    RegisterClassW(&windowClass);
}


////////////////////////////////////////////////////////////
void window_impl_win32::switch_to_fullscreen(const video_mode& mode)
{
    DEVMODEW devMode;
    devMode.dmSize       = sizeof(devMode);
    devMode.dmPelsWidth  = mode.width;
    devMode.dmPelsHeight = mode.height;
    devMode.dmBitsPerPel = mode.bits_per_pixel;
    devMode.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

    // Apply fullscreen mode
    if (ChangeDisplaySettingsW(&devMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
    {
        err() << "Failed to change display mode for fullscreen" << std::endl;
        return;
    }

    // Make the window flags compatible with fullscreen mode
    SetWindowLongW(handle_, GWL_STYLE, WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    SetWindowLongW(handle_, GWL_EXSTYLE, WS_EX_APPWINDOW);

    // Resize the window so that it fits the entire screen
    SetWindowPos(handle_, HWND_TOP, 0, 0, mode.width, mode.height, SWP_FRAMECHANGED);
    ShowWindow(handle_, SW_SHOW);

    // Set "this" as the current fullscreen window
    fullscreenWindow = this;
}


////////////////////////////////////////////////////////////
void window_impl_win32::cleanup()
{
    // Restore the previous video mode (in case we were running in fullscreen)
    if (fullscreenWindow == this)
    {
        ChangeDisplaySettingsW(nullptr, 0);
        fullscreenWindow = nullptr;
    }

    // Unhide the mouse cursor (in case it was hidden)
    set_mouse_cursor_visible(true);

    // No longer track the cursor
    set_tracking(false);

    // No longer capture the cursor
    ReleaseCapture();
}


////////////////////////////////////////////////////////////
void window_impl_win32::set_tracking(bool track)
{
    TRACKMOUSEEVENT mouseEvent;
    mouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
    mouseEvent.dwFlags = track ? TME_LEAVE : TME_CANCEL;
    mouseEvent.hwndTrack = handle_;
    mouseEvent.dwHoverTime = HOVER_DEFAULT;
    TrackMouseEvent(&mouseEvent);
}


////////////////////////////////////////////////////////////
void window_impl_win32::grab_cursor(bool grabbed)
{
    if (grabbed)
    {
        RECT rect;
        GetClientRect(handle_, &rect);
        MapWindowPoints(handle_, nullptr, reinterpret_cast<LPPOINT>(&rect), 2);
        ClipCursor(&rect);
    }
    else
    {
        ClipCursor(nullptr);
    }
}


////////////////////////////////////////////////////////////
void window_impl_win32::process_event(UINT message, WPARAM wParam, LPARAM lParam)
{
    // Don't process any message until window is created
    if (handle_ == nullptr)
        return;

    switch (message)
    {
        // Destroy event
        case WM_DESTROY:
        {
            // Here we must cleanup resources !
            cleanup();
            break;
        }

        // Set cursor event
        case WM_SETCURSOR:
        {
            // The mouse has moved, if the cursor is in our window we must refresh the cursor
            if (LOWORD(lParam) == HTCLIENT)
                SetCursor(cursor_visible_ ? last_cursor_ : nullptr);

            break;
        }

        // Close event
        case WM_CLOSE:
        {
            platform_event event;
            event.type = platform_event::closed;
            push_event(event);
            break;
        }

        // Resize event
        case WM_SIZE:
        {
            // Consider only events triggered by a maximize or a un-maximize
            if (wParam != SIZE_MINIMIZED && !resizing_ && last_size_ != get_size())
            {
                // Update the last handled size
                last_size_ = get_size();

                // Push a resize event
                platform_event event;
                event.type        = platform_event::resized;
                event.size.width  = last_size_[0];
                event.size.height = last_size_[1];
                push_event(event);

                // Restore/update cursor grabbing
                grab_cursor(cursor_grabbed_);
            }
            break;
        }

        // Start resizing
        case WM_ENTERSIZEMOVE:
        {
            resizing_ = true;
            grab_cursor(false);
            break;
        }

        // Stop resizing
        case WM_EXITSIZEMOVE:
        {
            resizing_ = false;

            // Ignore cases where the window has only been moved
            if(last_size_ != get_size())
            {
                // Update the last handled size
                last_size_ = get_size();

                // Push a resize event
                platform_event event;
                event.type        = platform_event::resized;
                event.size.width  = last_size_[0];
                event.size.height = last_size_[1];
                push_event(event);
            }

            // Restore/update cursor grabbing
            grab_cursor(cursor_grabbed_);
            break;
        }

        // The system request the min/max window size and position
        case WM_GETMINMAXINFO:
        {
            // We override the returned information to remove the default limit
            // (the OS doesn't allow windows bigger than the desktop by default)
            MINMAXINFO* info = reinterpret_cast<MINMAXINFO*>(lParam);
            info->ptMaxTrackSize.x = 50000;
            info->ptMaxTrackSize.y = 50000;
            break;
        }

        // Gain focus event
        case WM_SETFOCUS:
        {
            // Restore cursor grabbing
            grab_cursor(cursor_grabbed_);

            platform_event event;
            event.type = platform_event::gained_focus;
            push_event(event);
            break;
        }

        // Lost focus event
        case WM_KILLFOCUS:
        {
            // Ungrab the cursor
            grab_cursor(false);

            platform_event event;
            event.type = platform_event::lost_focus;
            push_event(event);
            break;
        }

        // Text event
        case WM_CHAR:
        {
            if (key_repeat_enabled_ || ((lParam & (1 << 30)) == 0))
            {
                // Get the code of the typed character
				std::uint32_t character = static_cast<std::uint32_t>(wParam);

                // Check if it is the first part of a surrogate pair, or a regular character
                if ((character >= 0xD800) && (character <= 0xDBFF))
                {
                    // First part of a surrogate pair: store it and wait for the second one
                    surrogate_ = static_cast<std::uint16_t>(character);
                }
                else
                {
                    // Check if it is the second part of a surrogate pair, or a regular character
                    if ((character >= 0xDC00) && (character <= 0xDFFF))
                    {
                        // Convert the UTF-16 surrogate pair to a single UTF-32 value
                        std::uint16_t utf16[] = {surrogate_, static_cast<std::uint16_t>(character)};
                        mml::utf16::to_utf32(utf16, utf16 + 2, &character);
                        surrogate_ = 0;
                    }

                    // Send a text_entered event
                    platform_event event;
                    event.type = platform_event::text_entered;
                    event.text.unicode = character;
                    push_event(event);
                }
            }
            break;
        }

        // Keydown event
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            if (key_repeat_enabled_ || ((HIWORD(lParam) & KF_REPEAT) == 0))
            {
                platform_event event;
                event.type         = platform_event::key_pressed;
                event.key.alt      = HIWORD(GetKeyState(VK_MENU))    != 0;
                event.key.control  = HIWORD(GetKeyState(VK_CONTROL)) != 0;
                event.key.shift    = HIWORD(GetKeyState(VK_SHIFT))   != 0;
                event.key.system   = HIWORD(GetKeyState(VK_LWIN)) || HIWORD(GetKeyState(VK_RWIN));
                event.key.code     = virtual_key_code_to_mml(wParam, lParam);
                event.key.scancode = to_scancode(wParam, lParam);
                push_event(event);
            }
            break;
        }

        // Keyup event
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            platform_event event;
            event.type         = platform_event::key_released;
            event.key.alt      = HIWORD(GetKeyState(VK_MENU))    != 0;
            event.key.control  = HIWORD(GetKeyState(VK_CONTROL)) != 0;
            event.key.shift    = HIWORD(GetKeyState(VK_SHIFT))   != 0;
            event.key.system   = HIWORD(GetKeyState(VK_LWIN)) || HIWORD(GetKeyState(VK_RWIN));
            event.key.code     = virtual_key_code_to_mml(wParam, lParam);
            event.key.scancode = to_scancode(wParam, lParam);
            push_event(event);
            break;
        }

        // Vertical mouse wheel event
        case WM_MOUSEWHEEL:
        {
            // mouse position is in screen coordinates, convert it to window coordinates
            POINT position;
            position.x = static_cast<std::int16_t>(LOWORD(lParam));
            position.y = static_cast<std::int16_t>(HIWORD(lParam));
            ScreenToClient(handle_, &position);

            std::int16_t delta = static_cast<std::int16_t>(HIWORD(wParam));

            platform_event event;

            event.type                   = platform_event::mouse_wheel_scrolled;
            event.mouse_wheel_scroll.wheel = mouse::vertical_wheel;
            event.mouse_wheel_scroll.delta = static_cast<float>(delta) / 120.f;
            event.mouse_wheel_scroll.x     = position.x;
            event.mouse_wheel_scroll.y     = position.y;
            push_event(event);
            break;
        }

        // Horizontal mouse wheel event
        case WM_MOUSEHWHEEL:
        {
            // mouse position is in screen coordinates, convert it to window coordinates
            POINT position;
            position.x = static_cast<std::int16_t>(LOWORD(lParam));
            position.y = static_cast<std::int16_t>(HIWORD(lParam));
            ScreenToClient(handle_, &position);

            std::int16_t delta = static_cast<std::int16_t>(HIWORD(wParam));

            platform_event event;
            event.type                   = platform_event::mouse_wheel_scrolled;
            event.mouse_wheel_scroll.wheel = mouse::horizontal_wheel;
            event.mouse_wheel_scroll.delta = -static_cast<float>(delta) / 120.f;
            event.mouse_wheel_scroll.x     = position.x;
            event.mouse_wheel_scroll.y     = position.y;
            push_event(event);
            break;
        }

        // mouse left button down event
        case WM_LBUTTONDOWN:
        {
            platform_event event;
            event.type               = platform_event::mouse_button_pressed;
            event.mouse_button.button = mouse::left;
            event.mouse_button.x      = static_cast<std::int16_t>(LOWORD(lParam));
            event.mouse_button.y      = static_cast<std::int16_t>(HIWORD(lParam));
            push_event(event);
            break;
        }

        // mouse left button up event
        case WM_LBUTTONUP:
        {
            platform_event event;
            event.type               = platform_event::mouse_button_released;
            event.mouse_button.button = mouse::left;
            event.mouse_button.x      = static_cast<std::int16_t>(LOWORD(lParam));
            event.mouse_button.y      = static_cast<std::int16_t>(HIWORD(lParam));
            push_event(event);
            break;
        }

        // mouse right button down event
        case WM_RBUTTONDOWN:
        {
            platform_event event;
            event.type               = platform_event::mouse_button_pressed;
            event.mouse_button.button = mouse::right;
            event.mouse_button.x      = static_cast<std::int16_t>(LOWORD(lParam));
            event.mouse_button.y      = static_cast<std::int16_t>(HIWORD(lParam));
            push_event(event);
            break;
        }

        // mouse right button up event
        case WM_RBUTTONUP:
        {
            platform_event event;
            event.type               = platform_event::mouse_button_released;
            event.mouse_button.button = mouse::right;
            event.mouse_button.x      = static_cast<std::int16_t>(LOWORD(lParam));
            event.mouse_button.y      = static_cast<std::int16_t>(HIWORD(lParam));
            push_event(event);
            break;
        }

        // mouse wheel button down event
        case WM_MBUTTONDOWN:
        {
            platform_event event;
            event.type               = platform_event::mouse_button_pressed;
            event.mouse_button.button = mouse::middle;
            event.mouse_button.x      = static_cast<std::int16_t>(LOWORD(lParam));
            event.mouse_button.y      = static_cast<std::int16_t>(HIWORD(lParam));
            push_event(event);
            break;
        }

        // mouse wheel button up event
        case WM_MBUTTONUP:
        {
            platform_event event;
            event.type               = platform_event::mouse_button_released;
            event.mouse_button.button = mouse::middle;
            event.mouse_button.x      = static_cast<std::int16_t>(LOWORD(lParam));
            event.mouse_button.y      = static_cast<std::int16_t>(HIWORD(lParam));
            push_event(event);
            break;
        }

        // mouse X button down event
        case WM_XBUTTONDOWN:
        {
            platform_event event;
            event.type               = platform_event::mouse_button_pressed;
            event.mouse_button.button = HIWORD(wParam) == XBUTTON1 ? mouse::x_button1 : mouse::x_button2;
            event.mouse_button.x      = static_cast<std::int16_t>(LOWORD(lParam));
            event.mouse_button.y      = static_cast<std::int16_t>(HIWORD(lParam));
            push_event(event);
            break;
        }

        // mouse X button up event
        case WM_XBUTTONUP:
        {
            platform_event event;
            event.type               = platform_event::mouse_button_released;
            event.mouse_button.button = HIWORD(wParam) == XBUTTON1 ? mouse::x_button1 : mouse::x_button2;
            event.mouse_button.x      = static_cast<std::int16_t>(LOWORD(lParam));
            event.mouse_button.y      = static_cast<std::int16_t>(HIWORD(lParam));
            push_event(event);
            break;
        }

        // mouse leave event
        case WM_MOUSELEAVE:
        {
            // Avoid this firing a second time in case the cursor is dragged outside
            if (mouse_inside_)
            {
                mouse_inside_ = false;

                // Generate a mouse_left event
                platform_event event;
                event.type = platform_event::mouse_left;
                push_event(event);
            }
            break;
        }

        // mouse move event
        case WM_MOUSEMOVE:
        {
            // Extract the mouse local coordinates
            int x = static_cast<std::int16_t>(LOWORD(lParam));
            int y = static_cast<std::int16_t>(HIWORD(lParam));

            // Get the client area of the window
            RECT area;
            GetClientRect(handle_, &area);

            // Capture the mouse in case the user wants to drag it outside
            if ((wParam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2)) == 0)
            {
                // Only release the capture if we really have it
                if (GetCapture() == handle_)
                    ReleaseCapture();
            }
            else if (GetCapture() != handle_)
            {
                // Set the capture to continue receiving mouse events
                SetCapture(handle_);
            }

            // If the cursor is outside the client area...
            if ((x < area.left) || (x > area.right) || (y < area.top) || (y > area.bottom))
            {
                // and it used to be inside, the mouse left it.
                if (mouse_inside_)
                {
                    mouse_inside_ = false;

                    // No longer care for the mouse leaving the window
                    set_tracking(false);

                    // Generate a mouse_left event
                    platform_event event;
                    event.type = platform_event::mouse_left;
                    push_event(event);
                }
            }
            else
            {
                // and vice-versa
                if (!mouse_inside_)
                {
                    mouse_inside_ = true;

                    // Look for the mouse leaving the window
                    set_tracking(true);

                    // Generate a mouse_entered event
                    platform_event event;
                    event.type = platform_event::mouse_entered;
                    push_event(event);
                }
            }

            // Generate a MouseMove event
            platform_event event;
            event.type        = platform_event::mouse_moved;
            event.mouse_move.x = x;
            event.mouse_move.y = y;
            push_event(event);
            break;
        }
        case WM_DEVICECHANGE:
        {
            // Some sort of device change has happened, update joystick connections
            if ((wParam == DBT_DEVICEARRIVAL) || (wParam == DBT_DEVICEREMOVECOMPLETE))
            {
                // Some sort of device change has happened, update joystick connections if it is a device interface
                DEV_BROADCAST_HDR* deviceBroadcastHeader = reinterpret_cast<DEV_BROADCAST_HDR*>(lParam);

                if (deviceBroadcastHeader && (deviceBroadcastHeader->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE))
                    joystick_impl::update_connections();
            }
            break;
        }
    }
}


////////////////////////////////////////////////////////////
keyboard::key window_impl_win32::virtual_key_code_to_mml(WPARAM key, LPARAM flags)
{
    // clang-format off
    switch (key)
    {
        // Check the scancode to distinguish between left and right shift
        case VK_SHIFT:
        {
            static const UINT lShift = MapVirtualKeyW(VK_LSHIFT, MAPVK_VK_TO_VSC);
            const UINT scancode = static_cast<UINT>((flags & (0xFF << 16)) >> 16);
            return scancode == lShift ? keyboard::LShift : keyboard::RShift;
        }

        // Check the "extended" flag to distinguish between left and right alt
        case VK_MENU : return (HIWORD(flags) & KF_EXTENDED) ? keyboard::RAlt : keyboard::LAlt;

        // Check the "extended" flag to distinguish between left and right control
        case VK_CONTROL : return (HIWORD(flags) & KF_EXTENDED) ? keyboard::RControl : keyboard::LControl;

        // Other keys are reported properly
        case VK_LWIN:       return keyboard::LSystem;
        case VK_RWIN:       return keyboard::RSystem;
        case VK_APPS:       return keyboard::Menu;
        case VK_OEM_1:      return keyboard::Semicolon;
        case VK_OEM_2:      return keyboard::Slash;
        case VK_OEM_PLUS:   return keyboard::Equal;
        case VK_OEM_MINUS:  return keyboard::Hyphen;
        case VK_OEM_4:      return keyboard::LBracket;
        case VK_OEM_6:      return keyboard::RBracket;
        case VK_OEM_COMMA:  return keyboard::Comma;
        case VK_OEM_PERIOD: return keyboard::Period;
        case VK_OEM_7:      return keyboard::Apostrophe;
        case VK_OEM_5:      return keyboard::Backslash;
        case VK_OEM_3:      return keyboard::Grave;
        case VK_ESCAPE:     return keyboard::Escape;
        case VK_SPACE:      return keyboard::Space;
        case VK_RETURN:     return keyboard::Enter;
        case VK_BACK:       return keyboard::Backspace;
        case VK_TAB:        return keyboard::Tab;
        case VK_PRIOR:      return keyboard::PageUp;
        case VK_NEXT:       return keyboard::PageDown;
        case VK_END:        return keyboard::End;
        case VK_HOME:       return keyboard::Home;
        case VK_INSERT:     return keyboard::Insert;
        case VK_DELETE:     return keyboard::Delete;
        case VK_ADD:        return keyboard::Add;
        case VK_SUBTRACT:   return keyboard::Subtract;
        case VK_MULTIPLY:   return keyboard::Multiply;
        case VK_DIVIDE:     return keyboard::Divide;
        case VK_PAUSE:      return keyboard::Pause;
        case VK_F1:         return keyboard::F1;
        case VK_F2:         return keyboard::F2;
        case VK_F3:         return keyboard::F3;
        case VK_F4:         return keyboard::F4;
        case VK_F5:         return keyboard::F5;
        case VK_F6:         return keyboard::F6;
        case VK_F7:         return keyboard::F7;
        case VK_F8:         return keyboard::F8;
        case VK_F9:         return keyboard::F9;
        case VK_F10:        return keyboard::F10;
        case VK_F11:        return keyboard::F11;
        case VK_F12:        return keyboard::F12;
        case VK_F13:        return keyboard::F13;
        case VK_F14:        return keyboard::F14;
        case VK_F15:        return keyboard::F15;
        case VK_LEFT:       return keyboard::Left;
        case VK_RIGHT:      return keyboard::Right;
        case VK_UP:         return keyboard::Up;
        case VK_DOWN:       return keyboard::Down;
        case VK_NUMPAD0:    return keyboard::Numpad0;
        case VK_NUMPAD1:    return keyboard::Numpad1;
        case VK_NUMPAD2:    return keyboard::Numpad2;
        case VK_NUMPAD3:    return keyboard::Numpad3;
        case VK_NUMPAD4:    return keyboard::Numpad4;
        case VK_NUMPAD5:    return keyboard::Numpad5;
        case VK_NUMPAD6:    return keyboard::Numpad6;
        case VK_NUMPAD7:    return keyboard::Numpad7;
        case VK_NUMPAD8:    return keyboard::Numpad8;
        case VK_NUMPAD9:    return keyboard::Numpad9;
        case 'A':           return keyboard::A;
        case 'Z':           return keyboard::Z;
        case 'E':           return keyboard::E;
        case 'R':           return keyboard::R;
        case 'T':           return keyboard::T;
        case 'Y':           return keyboard::Y;
        case 'U':           return keyboard::U;
        case 'I':           return keyboard::I;
        case 'O':           return keyboard::O;
        case 'P':           return keyboard::P;
        case 'Q':           return keyboard::Q;
        case 'S':           return keyboard::S;
        case 'D':           return keyboard::D;
        case 'F':           return keyboard::F;
        case 'G':           return keyboard::G;
        case 'H':           return keyboard::H;
        case 'J':           return keyboard::J;
        case 'K':           return keyboard::K;
        case 'L':           return keyboard::L;
        case 'M':           return keyboard::M;
        case 'W':           return keyboard::W;
        case 'X':           return keyboard::X;
        case 'C':           return keyboard::C;
        case 'V':           return keyboard::V;
        case 'B':           return keyboard::B;
        case 'N':           return keyboard::N;
        case '0':           return keyboard::Num0;
        case '1':           return keyboard::Num1;
        case '2':           return keyboard::Num2;
        case '3':           return keyboard::Num3;
        case '4':           return keyboard::Num4;
        case '5':           return keyboard::Num5;
        case '6':           return keyboard::Num6;
        case '7':           return keyboard::Num7;
        case '8':           return keyboard::Num8;
        case '9':           return keyboard::Num9;
    }
    // clang-format on

    return keyboard::Unknown;
}

////////////////////////////////////////////////////////////
keyboard::scancode window_impl_win32::to_scancode(WPARAM wParam, LPARAM lParam)
{
    int code = (lParam & (0xFF << 16)) >> 16;

    // Retrieve the scancode from the VirtualKey for synthetic key messages
    if (code == 0)
    {
        code = static_cast<int>(MapVirtualKey(static_cast<UINT>(wParam), MAPVK_VK_TO_VSC));
    }

    // Windows scancodes
    // Reference: https://msdn.microsoft.com/en-us/library/aa299374(v=vs.60).aspx
    // clang-format off
    switch (code)
    {
        case 1: return keyboard::scan::Escape;
        case 2: return keyboard::scan::Num1;
        case 3: return keyboard::scan::Num2;
        case 4: return keyboard::scan::Num3;
        case 5: return keyboard::scan::Num4;
        case 6: return keyboard::scan::Num5;
        case 7: return keyboard::scan::Num6;
        case 8: return keyboard::scan::Num7;
        case 9: return keyboard::scan::Num8;
        case 10: return keyboard::scan::Num9;
        case 11: return keyboard::scan::Num0;
        case 12: return keyboard::scan::Hyphen;
        case 13: return keyboard::scan::Equal;
        case 14: return keyboard::scan::Backspace;
        case 15: return keyboard::scan::Tab;
        case 16: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::MediaPreviousTrack : keyboard::scan::Q;
        case 17: return keyboard::scan::W;
        case 18: return keyboard::scan::E;
        case 19: return keyboard::scan::R;
        case 20: return keyboard::scan::T;
        case 21: return keyboard::scan::Y;
        case 22: return keyboard::scan::U;
        case 23: return keyboard::scan::I;
        case 24: return keyboard::scan::O;
        case 25: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::MediaNextTrack     : keyboard::scan::P;
        case 26: return keyboard::scan::LBracket;
        case 27: return keyboard::scan::RBracket;
        case 28: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::NumpadEnter        : keyboard::scan::Enter;
        case 29: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::RControl           : keyboard::scan::LControl;
        case 30: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::Select             : keyboard::scan::A;
        case 31: return keyboard::scan::S;
        case 32: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::VolumeMute         : keyboard::scan::D;
        case 33: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::LaunchApplication1 : keyboard::scan::F;
        case 34: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::MediaPlayPause     : keyboard::scan::G;
        case 35: return keyboard::scan::H;
        case 36: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::MediaStop          : keyboard::scan::J;
        case 37: return keyboard::scan::K;
        case 38: return keyboard::scan::L;
        case 39: return keyboard::scan::Semicolon;
        case 40: return keyboard::scan::Apostrophe;
        case 41: return keyboard::scan::Grave;
        case 42: return keyboard::scan::LShift;
        case 43: return keyboard::scan::Backslash;
        case 44: return keyboard::scan::Z;
        case 45: return keyboard::scan::X;
        case 46: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::VolumeDown   : keyboard::scan::C;
        case 47: return keyboard::scan::V;
        case 48: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::VolumeUp     : keyboard::scan::B;
        case 49: return keyboard::scan::N;
        case 50: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::HomePage     : keyboard::scan::M;
        case 51: return keyboard::scan::Comma;
        case 52: return keyboard::scan::Period;
        case 53: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::NumpadDivide : keyboard::scan::Slash;
        case 54: return keyboard::scan::RShift;
        case 55: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::PrintScreen  : keyboard::scan::NumpadMultiply;
        case 56: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::RAlt         : keyboard::scan::LAlt;
        case 57: return keyboard::scan::Space;
        case 58: return keyboard::scan::CapsLock;
        case 59: return keyboard::scan::F1;
        case 60: return keyboard::scan::F2;
        case 61: return keyboard::scan::F3;
        case 62: return keyboard::scan::F4;
        case 63: return keyboard::scan::F5;
        case 64: return keyboard::scan::F6;
        case 65: return keyboard::scan::F7;
        case 66: return keyboard::scan::F8;
        case 67: return keyboard::scan::F9;
        case 68: return keyboard::scan::F10;
        case 69: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::NumLock  : keyboard::scan::Pause;
        case 70: return keyboard::scan::ScrollLock;
        case 71: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::Home     : keyboard::scan::Numpad7;
        case 72: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::Up       : keyboard::scan::Numpad8;
        case 73: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::PageUp   : keyboard::scan::Numpad9;
        case 74: return keyboard::scan::NumpadMinus;
        case 75: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::Left     : keyboard::scan::Numpad4;
        case 76: return keyboard::scan::Numpad5;
        case 77: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::Right    : keyboard::scan::Numpad6;
        case 78: return keyboard::scan::NumpadPlus;
        case 79: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::End      : keyboard::scan::Numpad1;
        case 80: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::Down     : keyboard::scan::Numpad2;
        case 81: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::PageDown : keyboard::scan::Numpad3;
        case 82: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::Insert   : keyboard::scan::Numpad0;
        case 83: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::Delete   : keyboard::scan::NumpadDecimal;

        case 86: return keyboard::scan::NonUsBackslash;
        case 87: return keyboard::scan::F11;
        case 88: return keyboard::scan::F12;

        case 91: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::LSystem : keyboard::scan::Unknown;
        case 92: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::RSystem : keyboard::scan::Unknown;
        case 93: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::Menu    : keyboard::scan::Unknown;

        case 99: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::Help    : keyboard::scan::Unknown;
        case 100: return keyboard::scan::F13;
        case 101: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::Search             : keyboard::scan::F14;
        case 102: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::Favorites          : keyboard::scan::F15;
        case 103: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::Refresh            : keyboard::scan::F16;
        case 104: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::Stop               : keyboard::scan::F17;
        case 105: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::Forward            : keyboard::scan::F18;
        case 106: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::Back               : keyboard::scan::F19;
        case 107: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::LaunchApplication1 : keyboard::scan::F20;
        case 108: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::LaunchMail         : keyboard::scan::F21;
        case 109: return (HIWORD(lParam) & KF_EXTENDED) ? keyboard::scan::LaunchMediaSelect  : keyboard::scan::F22;
        case 110: return keyboard::scan::F23;

        case 118: return keyboard::scan::F24;

        default: return keyboard::scan::Unknown;
    }
    // clang-format on
}

////////////////////////////////////////////////////////////
LRESULT CALLBACK window_impl_win32::global_on_event(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Associate handle and window instance when the creation message is received
    if (message == WM_CREATE)
    {
        // Get window_impl_win32 instance (it was passed as the last argument of CreateWindow)
        LONG_PTR window = (LONG_PTR)reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams;

        // Set as the "user data" parameter of the window
        SetWindowLongPtrW(handle, GWLP_USERDATA, window);
    }

    // Get the WindowImpl instance corresponding to the window handle
    window_impl_win32* window = handle ? reinterpret_cast<window_impl_win32*>(GetWindowLongPtr(handle, GWLP_USERDATA)) : nullptr;

    // Forward the event to the appropriate function
    if (window)
    {
        window->process_event(message, wParam, lParam);

        if (window->callback_)
            return CallWindowProcW(reinterpret_cast<WNDPROC>(window->callback_), handle, message, wParam, lParam);
    }

    // We don't forward the WM_CLOSE message to prevent the OS from automatically destroying the window
    if (message == WM_CLOSE)
        return 0;

    // Don't forward the menu system command, so that pressing ALT or F10 doesn't steal the focus
    if ((message == WM_SYSCOMMAND) && (wParam == SC_KEYMENU))
        return 0;

    return DefWindowProcW(handle, message, wParam, lParam);
}

} // namespace priv

} // namespace mml

