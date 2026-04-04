////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
// clang-format off
#include <mml/system/err.hpp>
#include <mml/system/utf.hpp>
#include <mml/window/unix/clipboard_impl.hpp>
#include <mml/window/unix/display.hpp>
#include <mml/window/unix/input_impl.hpp>
#include <mml/window/unix/keyboard_impl.hpp>
#include <mml/window/unix/window_impl_x11.hpp>
#include <mml/window/window_style.hpp> // important to be included first (conflict with None)

#include <X11/Xatom.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
#include <X11/keysym.h>

#include <fcntl.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

#undef max
#undef min
#include <bitset>
#include <mutex>
#include <thread>

//clang-format on

#define _NET_WM_STATE_REMOVE 0l
#define _NET_WM_STATE_ADD 1l
#define _NET_WM_STATE_TOGGLE 2l
////////////////////////////////////////////////////////////
// Private data
////////////////////////////////////////////////////////////
namespace
{
// mml::priv::window_impl_x11*              fullscreenWindow = nullptr;
std::vector<mml::priv::window_impl_x11*> allWindows;
std::bitset<256> isKeyFiltered;
std::mutex allWindowsMutex;
std::string windowManagerName;
std::string wmAbsPosGood[] = {"Enlightenment", "FVWM", "i3", "KWin", "Xfwm4"};
static const unsigned long eventMask = FocusChangeMask | ButtonPressMask | ButtonReleaseMask |
									   ButtonMotionMask | PointerMotionMask | KeyPressMask | KeyReleaseMask |
									   StructureNotifyMask | EnterWindowMask | LeaveWindowMask |
									   VisibilityChangeMask | PropertyChangeMask;

static const unsigned int maxTrialsCount = 5;

// Predicate we use to find key repeat events in processEvent
struct KeyRepeatFinder
{
	KeyRepeatFinder(unsigned int keycode, Time time)
		: keycode(keycode)
		, time(time)
	{
	}

	// Predicate operator that checks event type, keycode and timestamp
	bool operator()(const XEvent& event)
	{
		return ((event.type == KeyPress) && (event.xkey.keycode == keycode) && (event.xkey.time - time < 2));
	}

	unsigned int keycode;
	Time time;
};
// Filter the events received by windows (only allow those matching a specific window)
Bool checkEvent(::Display*, XEvent* event, XPointer userData)
{
	// Just check if the event matches the window
	return event->xany.window == reinterpret_cast<::Window>(userData);
}

// Find the name of the current executable
std::string findExecutableName()
{
	// We use /proc/self/cmdline to get the command line
	// the user used to invoke this instance of the application
	int file = ::open("/proc/self/cmdline", O_RDONLY | O_NONBLOCK);

	if(file < 0)
		return "mml";

	std::vector<char> buffer(256, 0);
	std::size_t offset = 0;
	ssize_t result = 0;

	while((result = read(file, &buffer[offset], 256)) > 0)
	{
		buffer.resize(buffer.size() + result, 0);
		offset += result;
	}

	::close(file);

	if(offset)
	{
		buffer[offset] = 0;

		// Remove the path to keep the executable name only
		return basename(&buffer[0]);
	}

	// Default fallback name
	return "mml";
}

// Check if Extended window Manager Hints are supported
bool ewmhSupported()
{
	static bool checked = false;
	static bool ewmhSupported = false;

	if(checked)
		return ewmhSupported;

	checked = true;

	Atom netSupportingWmCheck = mml::priv::get_atom("_NET_SUPPORTING_WM_CHECK", true);
	Atom netSupported = mml::priv::get_atom("_NET_SUPPORTED", true);

	if(!netSupportingWmCheck || !netSupported)
		return false;

	::Display* display = mml::priv::open_display();

	Atom actualType;
	int actualFormat;
	unsigned long numItems;
	unsigned long numBytes;
	unsigned char* data;

	int result = XGetWindowProperty(display, DefaultRootWindow(display), netSupportingWmCheck, 0, 1, False,
									XA_WINDOW, &actualType, &actualFormat, &numItems, &numBytes, &data);

	if(result != Success || actualType != XA_WINDOW || numItems != 1)
	{
		if(result == Success)
			XFree(data);

		mml::priv::close_display(display);
		return false;
	}

	::Window rootWindow = *reinterpret_cast<::Window*>(data);

	XFree(data);

	if(!rootWindow)
	{
		mml::priv::close_display(display);
		return false;
	}

	result = XGetWindowProperty(display, rootWindow, netSupportingWmCheck, 0, 1, False, XA_WINDOW,
								&actualType, &actualFormat, &numItems, &numBytes, &data);

	if(result != Success || actualType != XA_WINDOW || numItems != 1)
	{
		if(result == Success)
			XFree(data);

		mml::priv::close_display(display);
		return false;
	}

	::Window childWindow = *reinterpret_cast<::Window*>(data);

	XFree(data);

	if(!childWindow)
	{
		mml::priv::close_display(display);
		return false;
	}

	// Conforming window managers should return the same window for both queries
	if(rootWindow != childWindow)
	{
		mml::priv::close_display(display);
		return false;
	}

	ewmhSupported = true;

	// We try to get the name of the window manager
	// for window manager specific workarounds
	Atom netWmName = mml::priv::get_atom("_NET_WM_NAME", true);

	if(!netWmName)
	{
		mml::priv::close_display(display);
		return true;
	}

	Atom utf8StringType = mml::priv::get_atom("UTF8_STRING");

	if(!utf8StringType)
		utf8StringType = XA_STRING;

	result = XGetWindowProperty(display, rootWindow, netWmName, 0, 0x7fffffff, False, utf8StringType,
								&actualType, &actualFormat, &numItems, &numBytes, &data);

	if(actualType && numItems)
	{
		// It seems the wm name string reply is not necessarily
		// nullptr-terminated. The work around is to get its actual
		// length to build a proper string
		const char* begin = reinterpret_cast<const char*>(data);
		// const char* end = begin + numItems;
		windowManagerName = std::string(begin, numItems);
	}

	if(result == Success)
		XFree(data);

	mml::priv::close_display(display);

	return true;
}

// Get the parent window.
::Window getParentWindow(::Display* disp, ::Window win)
{
	::Window root, parent;
	::Window* children = nullptr;
	unsigned int numChildren;

	XQueryTree(disp, win, &root, &parent, &children, &numChildren);

	// Children information is not used, so must be freed.
	if(children != nullptr)
		XFree(children);

	return parent;
}

// Get the Frame Extents from EWMH WMs that support it (left, right, top, bottom per EWMH).
bool getEWMHFrameExtents(::Display* disp, ::Window win, long& left, long& right, long& top, long& bottom)
{
	if(!ewmhSupported())
		return false;

	Atom frameExtents = mml::priv::get_atom("_NET_FRAME_EXTENTS", true);

	if(frameExtents == None)
		return false;

	bool gotFrameExtents = false;
	Atom actualType;
	int actualFormat;
	unsigned long numItems;
	unsigned long numBytesLeft;
	unsigned char* data = nullptr;

	int result = XGetWindowProperty(disp, win, frameExtents, 0, 4, False, XA_CARDINAL, &actualType,
									&actualFormat, &numItems, &numBytesLeft, &data);

	if((result == Success) && (actualType == XA_CARDINAL) && (actualFormat == 32) && (numItems == 4) &&
	   (numBytesLeft == 0) && (data != nullptr))
	{
		gotFrameExtents = true;

		long* extents = (long*)data;

		left = extents[0];
		right = extents[1];
		top = extents[2];
		bottom = extents[3];
	}

	// Always free data.
	if(data != nullptr)
		XFree(data);

	return gotFrameExtents;
}

// Check if the current WM is in the list of good WMs that provide
// a correct absolute position for the window when queried.
bool isWMAbsolutePositionGood()
{
	// This can only work with EWMH, to get the name.
	//        if (!ewmhSupported())
	//            return false;

	//        for (size_t i = 0; i < (sizeof(wmAbsPosGood) / sizeof(wmAbsPosGood[0])); i++)
	//        {
	//            if (wmAbsPosGood[i] == windowManagerName)
	//                return true;
	//        }

	return true;
}

mml::keyboard::key keysym_to_mml(KeySym symbol)
{
	switch(symbol)
	{
		case XK_Shift_L:
			return mml::keyboard::LShift;
		case XK_Shift_R:
			return mml::keyboard::RShift;
		case XK_Control_L:
			return mml::keyboard::LControl;
		case XK_Control_R:
			return mml::keyboard::RControl;
		case XK_Alt_L:
			return mml::keyboard::LAlt;
		case XK_Alt_R:
			return mml::keyboard::RAlt;
		case XK_Super_L:
			return mml::keyboard::LSystem;
		case XK_Super_R:
			return mml::keyboard::RSystem;
		case XK_Menu:
			return mml::keyboard::Menu;
		case XK_Escape:
			return mml::keyboard::Escape;
		case XK_semicolon:
			return mml::keyboard::Semicolon;
		case XK_slash:
			return mml::keyboard::Slash;
		case XK_equal:
			return mml::keyboard::Equal;
		case XK_minus:
			return mml::keyboard::Hyphen;
		case XK_bracketleft:
			return mml::keyboard::LBracket;
		case XK_bracketright:
			return mml::keyboard::RBracket;
		case XK_comma:
			return mml::keyboard::Comma;
		case XK_period:
			return mml::keyboard::Period;
		case XK_apostrophe:
			return mml::keyboard::Apostrophe;
		case XK_backslash:
			return mml::keyboard::Backslash;
		case XK_grave:
			return mml::keyboard::Grave;
		case XK_space:
			return mml::keyboard::Space;
		case XK_Return:
			return mml::keyboard::Enter;
		case XK_KP_Enter:
			return mml::keyboard::Enter;
		case XK_BackSpace:
			return mml::keyboard::Backspace;
		case XK_Tab:
			return mml::keyboard::Tab;
		case XK_Prior:
			return mml::keyboard::PageUp;
		case XK_Next:
			return mml::keyboard::PageDown;
		case XK_End:
			return mml::keyboard::End;
		case XK_Home:
			return mml::keyboard::Home;
		case XK_Insert:
			return mml::keyboard::Insert;
		case XK_Delete:
			return mml::keyboard::Delete;
		case XK_KP_Add:
			return mml::keyboard::Add;
		case XK_KP_Subtract:
			return mml::keyboard::Subtract;
		case XK_KP_Multiply:
			return mml::keyboard::Multiply;
		case XK_KP_Divide:
			return mml::keyboard::Divide;
		case XK_Pause:
			return mml::keyboard::Pause;
		case XK_F1:
			return mml::keyboard::F1;
		case XK_F2:
			return mml::keyboard::F2;
		case XK_F3:
			return mml::keyboard::F3;
		case XK_F4:
			return mml::keyboard::F4;
		case XK_F5:
			return mml::keyboard::F5;
		case XK_F6:
			return mml::keyboard::F6;
		case XK_F7:
			return mml::keyboard::F7;
		case XK_F8:
			return mml::keyboard::F8;
		case XK_F9:
			return mml::keyboard::F9;
		case XK_F10:
			return mml::keyboard::F10;
		case XK_F11:
			return mml::keyboard::F11;
		case XK_F12:
			return mml::keyboard::F12;
		case XK_F13:
			return mml::keyboard::F13;
		case XK_F14:
			return mml::keyboard::F14;
		case XK_F15:
			return mml::keyboard::F15;
		case XK_Left:
			return mml::keyboard::Left;
		case XK_Right:
			return mml::keyboard::Right;
		case XK_Up:
			return mml::keyboard::Up;
		case XK_Down:
			return mml::keyboard::Down;
		case XK_KP_Insert:
			return mml::keyboard::Numpad0;
		case XK_KP_End:
			return mml::keyboard::Numpad1;
		case XK_KP_Down:
			return mml::keyboard::Numpad2;
		case XK_KP_Page_Down:
			return mml::keyboard::Numpad3;
		case XK_KP_Left:
			return mml::keyboard::Numpad4;
		case XK_KP_Begin:
			return mml::keyboard::Numpad5;
		case XK_KP_Right:
			return mml::keyboard::Numpad6;
		case XK_KP_Home:
			return mml::keyboard::Numpad7;
		case XK_KP_Up:
			return mml::keyboard::Numpad8;
		case XK_KP_Page_Up:
			return mml::keyboard::Numpad9;
		case XK_a:
			return mml::keyboard::A;
		case XK_b:
			return mml::keyboard::B;
		case XK_c:
			return mml::keyboard::C;
		case XK_d:
			return mml::keyboard::D;
		case XK_e:
			return mml::keyboard::E;
		case XK_f:
			return mml::keyboard::F;
		case XK_g:
			return mml::keyboard::G;
		case XK_h:
			return mml::keyboard::H;
		case XK_i:
			return mml::keyboard::I;
		case XK_j:
			return mml::keyboard::J;
		case XK_k:
			return mml::keyboard::K;
		case XK_l:
			return mml::keyboard::L;
		case XK_m:
			return mml::keyboard::M;
		case XK_n:
			return mml::keyboard::N;
		case XK_o:
			return mml::keyboard::O;
		case XK_p:
			return mml::keyboard::P;
		case XK_q:
			return mml::keyboard::Q;
		case XK_r:
			return mml::keyboard::R;
		case XK_s:
			return mml::keyboard::S;
		case XK_t:
			return mml::keyboard::T;
		case XK_u:
			return mml::keyboard::U;
		case XK_v:
			return mml::keyboard::V;
		case XK_w:
			return mml::keyboard::W;
		case XK_x:
			return mml::keyboard::X;
		case XK_y:
			return mml::keyboard::Y;
		case XK_z:
			return mml::keyboard::Z;
		case XK_0:
			return mml::keyboard::Num0;
		case XK_1:
			return mml::keyboard::Num1;
		case XK_2:
			return mml::keyboard::Num2;
		case XK_3:
			return mml::keyboard::Num3;
		case XK_4:
			return mml::keyboard::Num4;
		case XK_5:
			return mml::keyboard::Num5;
		case XK_6:
			return mml::keyboard::Num6;
		case XK_7:
			return mml::keyboard::Num7;
		case XK_8:
			return mml::keyboard::Num8;
		case XK_9:
			return mml::keyboard::Num9;
	}

	return mml::keyboard::Unknown;
}
} // namespace

namespace mml
{
namespace priv
{
void set_net_wm_state(Display* display, ::Window xwindow, uint32_t style)
{
	auto _NET_WM_STATE = get_atom("_NET_WM_STATE", False);
	//    auto _NET_WM_STATE_FOCUSED = get_atom("_NET_WM_STATE_FOCUSED", False);
	auto _NET_WM_STATE_ABOVE = get_atom("_NET_WM_STATE_ABOVE", False);
	//    auto _NET_WM_WINDOW_TYPE = get_atom("_NET_WM_WINDOW_TYPE", False);
	auto _NET_WM_STATE_SKIP_TASKBAR = get_atom("_NET_WM_STATE_SKIP_TASKBAR", False);
	auto _NET_WM_STATE_SKIP_PAGER = get_atom("_NET_WM_STATE_SKIP_PAGER", False);

	Atom atoms[16];
	int count = 0;

	if(style & style::always_on_top)
	{
		atoms[count++] = _NET_WM_STATE_ABOVE;
	}
	if(style & style::no_taskbar)
	{
		atoms[count++] = _NET_WM_STATE_SKIP_TASKBAR;
		atoms[count++] = _NET_WM_STATE_SKIP_PAGER;
	}

	if(count > 0)
	{
		XChangeProperty(display, xwindow, _NET_WM_STATE, XA_ATOM, 32, PropModeReplace, (unsigned char*)atoms,
						count);
	} /*else {
		XDeleteProperty(display, xwindow, _NET_WM_STATE);
	}*/
}

////////////////////////////////////////////////////////////
window_impl_x11::window_impl_x11(window_handle handle)
	: window_(0)
	, screen_(0)
	, input_method_(nullptr)
	, input_context_(nullptr)
	, is_external_(true)
	,
	// old_video_mode_   (0),
	// old_rrc_rtc_      (0),
	hidden_cursor_(0)
	, last_cursor_(None)
	, key_repeat_(true)
	, previous_pos_{{-1, -1}}
	, previous_size_{{-1, -1}}
	, use_size_hints_(false)
	, fullscreen_(false)
	, cursor_grabbed_(false)
	, cursor_visible_(true)
	, window_mapped_(false)
	, icon_pixmap_(0)
	, icon_mask_pixmap_(0)
	, last_input_time_(0)
{
	// Open a connection with the X server
	display_ = open_display();

	// Make sure to check for EWMH support before we do anything
	ewmhSupported();

	screen_ = DefaultScreen(display_);

	// Save the window handle
	window_ = handle;

	if(window_)
	{
		// Make sure the window is listening to all the required events
		XSetWindowAttributes attributes;
		attributes.event_mask = eventMask;

		XChangeWindowAttributes(display_, window_, CWEventMask, &attributes);

		// Set the WM protocols
		set_protocols();

		// Do some common initializations
		initialize(true);
	}
}

////////////////////////////////////////////////////////////
window_impl_x11::window_impl_x11(video_mode mode, const std::array<std::int32_t, 2>& position,
								 const std::string& title, unsigned long style)
	: window_(0)
	, screen_(0)
	, input_method_(nullptr)
	, input_context_(nullptr)
	, is_external_(false)
	,
	// old_video_mode_   (0),
	// old_rrc_rtc_      (0),
	hidden_cursor_(0)
	, last_cursor_(None)
	, key_repeat_(true)
	, previous_pos_{{-1, -1}}
	, previous_size_{{-1, -1}}
	, use_size_hints_(false)
	, fullscreen_(false)
	, cursor_grabbed_(fullscreen_)
	, window_mapped_(false)
	, icon_pixmap_(0)
	, icon_mask_pixmap_(0)
	, last_input_time_(0)
{
	// Open a connection with the X server
	display_ = open_display();

	// Make sure to check for EWMH support before we do anything
	ewmhSupported();

	screen_ = DefaultScreen(display_);

	// Compute position and size
	int32_t x{};
	int32_t y{};
	if(fullscreen_)
	{
		auto bounds = video_bounds::get_display_bounds(0);
		x = bounds.x;
		y = bounds.y;
	}
	else
	{
		x = position[0];
		y = position[1];

		if(x == centered)
		{
			x = (DisplayWidth(display_, screen_) - mode.width) / 2;
		}
		if(y == centered)
		{
			y = (DisplayHeight(display_, screen_) - mode.height) / 2;
		}
	}

	int width = int(mode.width);
	int height = int(mode.height);

	// Choose the visual according to the context setting
	Visual* visual = DefaultVisual(display_, screen_);

	int32_t depth = DefaultDepth(display_, screen_);
	// Define the window attributes
	XSetWindowAttributes attributes;
	attributes.colormap = XCreateColormap(display_, DefaultRootWindow(display_), visual, AllocNone);
	attributes.event_mask = eventMask;
	bool override = style & style::tooltip || style & style::popup_menu;
	attributes.override_redirect = ((fullscreen_ && !ewmhSupported()) || override) ? True : False;

	window_ = XCreateWindow(display_, DefaultRootWindow(display_), x, y, width, height, 0, depth, InputOutput,
							visual, CWEventMask | CWOverrideRedirect | CWColormap, &attributes);

	if(!window_)
	{
		err() << "Failed to create window" << std::endl;
		return;
	}

	XSizeHints my_hints{};
	std::memset(&my_hints, 0, sizeof(my_hints));
	my_hints.flags = PPosition | PSize; /* I want to specify position and size */
	my_hints.x = x;						/* The origin and size coords I want */
	my_hints.y = y;
	my_hints.width = width;
	my_hints.height = height;

	XSetNormalHints(display_, window_, &my_hints); /* Where new_window is the new window */

	// Set the WM protocols
	set_protocols();

	// Set the WM initial state to the normal state
	XWMHints* hints = XAllocWMHints();
	hints->flags = StateHint;
	hints->initial_state = NormalState;
	XSetWMHints(display_, window_, hints);
	XFree(hints);

	long compositor = 2; /* don't disable compositing except for "normal" windows */

	const char* wintype_name = nullptr;
	if(style & style::utility)
	{
		wintype_name = "_NET_WM_WINDOW_TYPE_UTILITY";
	}
	else if(style & style::tooltip)
	{
		wintype_name = "_NET_WM_WINDOW_TYPE_TOOLTIP";
	}
	else if(style & style::popup_menu)
	{
		wintype_name = "_NET_WM_WINDOW_TYPE_POPUP_MENU";
	}
	else
	{
		wintype_name = "_NET_WM_WINDOW_TYPE_NORMAL";
		compositor = 1; /* disable compositing for "normal" windows */
	}
	{
		auto _NET_WM_WINDOW_TYPE = get_atom("_NET_WM_WINDOW_TYPE", False);
		auto wintype = get_atom(wintype_name, False);
		XChangeProperty(display_, window_, _NET_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace,
						(unsigned char*)&wintype, 1);
	}

	{
		auto _NET_WM_BYPASS_COMPOSITOR = get_atom("_NET_WM_BYPASS_COMPOSITOR", False);
		XChangeProperty(display_, window_, _NET_WM_BYPASS_COMPOSITOR, XA_CARDINAL, 32, PropModeReplace,
						(unsigned char*)&compositor, 1);
	}

	set_net_wm_state(display_, window_, style);

	// If not in fullscreen, set the window's style (tell the window manager to
	// change our window's decorations and functions according to the requested style)
	if(!fullscreen_)
	{
		Atom WMHintsAtom = get_atom("_MOTIF_WM_HINTS", false);
		if(WMHintsAtom)
		{
			static const unsigned long MWM_HINTS_FUNCTIONS = 1 << 0;
			static const unsigned long MWM_HINTS_DECORATIONS = 1 << 1;

			// static const unsigned long MWM_DECOR_ALL         = 1 << 0;
			static const unsigned long MWM_DECOR_BORDER = 1 << 1;
			static const unsigned long MWM_DECOR_RESIZEH = 1 << 2;
			static const unsigned long MWM_DECOR_TITLE = 1 << 3;
			static const unsigned long MWM_DECOR_MENU = 1 << 4;
			static const unsigned long MWM_DECOR_MINIMIZE = 1 << 5;
			static const unsigned long MWM_DECOR_MAXIMIZE = 1 << 6;

			// static const unsigned long MWM_FUNC_ALL          = 1 << 0;
			static const unsigned long MWM_FUNC_RESIZE = 1 << 1;
			static const unsigned long MWM_FUNC_MOVE = 1 << 2;
			static const unsigned long MWM_FUNC_MINIMIZE = 1 << 3;
			static const unsigned long MWM_FUNC_MAXIMIZE = 1 << 4;
			static const unsigned long MWM_FUNC_CLOSE = 1 << 5;

			struct WMHints
			{
				unsigned long flags;
				unsigned long functions;
				unsigned long decorations;
				long inputMode;
				unsigned long state;
			};

			WMHints hints;
			std::memset(&hints, 0, sizeof(hints));
			hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
			hints.decorations = 0;
			hints.functions = 0;

			if(style & style::titlebar)
			{
				hints.decorations |= MWM_DECOR_BORDER | MWM_DECOR_TITLE | MWM_DECOR_MINIMIZE | MWM_DECOR_MENU;
				hints.functions |= MWM_FUNC_MOVE | MWM_FUNC_MINIMIZE;
			}
			if(style & style::resize)
			{
				hints.decorations |= MWM_DECOR_MAXIMIZE | MWM_DECOR_RESIZEH;
				hints.functions |= MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE;
			}
			if(style & style::close)
			{
				hints.decorations |= 0;
				hints.functions |= MWM_FUNC_CLOSE;
			}

			XChangeProperty(display_, window_, WMHintsAtom, WMHintsAtom, 32, PropModeReplace,
							reinterpret_cast<const unsigned char*>(&hints), 5);
		}
	}

	// This is a hack to force some windows managers to disable resizing
	if(!(style & style::resize))
	{
		use_size_hints_ = true;
		XSizeHints* sizeHints = XAllocSizeHints();

		sizeHints->flags = PMinSize | PMaxSize | USPosition;
		sizeHints->min_width = sizeHints->max_width = width;
		sizeHints->min_height = sizeHints->max_height = height;
		sizeHints->x = x;
		sizeHints->y = y;
		XSetWMNormalHints(display_, window_, sizeHints);
		XFree(sizeHints);
	}

	// Set the window's WM class (this can be used by window managers)
	XClassHint* hint = XAllocClassHint();

	// The instance name should be something unique to this invocation
	// of the application but is rarely if ever used these days.
	// For simplicity, we retrieve it via the base executable name.
	std::string executableName = findExecutableName();
	std::vector<char> windowInstance(executableName.size() + 1, 0);
	std::copy(executableName.begin(), executableName.end(), windowInstance.begin());
	hint->res_name = &windowInstance[0];

	// The class name identifies a class of windows that
	// "are of the same type". We simply use the initial window name as
	// the class name.
	std::string ansiTitle = title;
	std::vector<char> windowClass(ansiTitle.size() + 1, 0);
	std::copy(ansiTitle.begin(), ansiTitle.end(), windowClass.begin());
	hint->res_class = &windowClass[0];

	XSetClassHint(display_, window_, hint);

	XFree(hint);

	// Set the window's name
	set_title(title);

	// Do some common initializations
	bool hidden = (style & style::hidden) != 0;
	initialize(!hidden);

	// Set fullscreen video mode and switch to fullscreen if necessary
	if(fullscreen_)
	{
		// Disable hint for min and max size,
		// otherwise some windows managers will not remove window decorations
		XSizeHints* sizeHints = XAllocSizeHints();
		long flags = 0;
		XGetWMNormalHints(display_, window_, sizeHints, &flags);
		sizeHints->flags &= ~(PMinSize | PMaxSize);
		XSetWMNormalHints(display_, window_, sizeHints);
		XFree(sizeHints);

		//        set_video_mode(mode);
		//		switch_to_fullscreen();
	}
}

////////////////////////////////////////////////////////////
window_impl_x11::~window_impl_x11()
{
	// Cleanup graphical resources
	cleanup();

	// Destroy icon pixmap
	if(icon_pixmap_)
		XFreePixmap(display_, icon_pixmap_);

	// Destroy icon mask pixmap
	if(icon_mask_pixmap_)
		XFreePixmap(display_, icon_mask_pixmap_);

	// Destroy the cursor
	if(hidden_cursor_)
		XFreeCursor(display_, hidden_cursor_);

	// Destroy the input context
	if(input_context_)
		XDestroyIC(input_context_);

	// Destroy the window
	if(window_ && !is_external_)
	{
		XFlush(display_);
		XUnmapWindow(display_, window_);
		XFlush(display_);
		XDestroyWindow(display_, window_);
		XFlush(display_);
	}

	// Close the input method
	if(input_method_)
		XCloseIM(input_method_);

	// Close the connection with the X server
	close_display(display_);

	// Remove this window from the global list of windows (required for focus request)
	std::lock_guard<std::mutex> lock(allWindowsMutex);
	allWindows.erase(std::find(allWindows.begin(), allWindows.end(), this));
}

////////////////////////////////////////////////////////////
window_handle window_impl_x11::native_handle() const
{
	return window_;
}

////////////////////////////////////////////////////////////
void window_impl_x11::process_events()
{
	XEvent event;

	// Pick out the events that are interesting for this window
	while(XCheckIfEvent(display_, &event, &checkEvent, reinterpret_cast<XPointer>(window_)))
	{
		// This function implements a workaround to properly discard
		// repeated key events when necessary. The problem is that the
		// system's key events policy doesn't match mml's one: X server will generate
		// both repeated KeyPress and KeyRelease events when maintaining a key down, while
		// mml only wants repeated KeyPress events. Thus, we have to:
		// - Discard duplicated KeyRelease events when m_keyRepeat is true
		// - Discard both duplicated KeyPress and KeyRelease events when m_keyRepeat is false

		bool processThisEvent = true;

		// Detect repeated key events
		while(event.type == KeyRelease)
		{
			XEvent nextEvent;
			if(XCheckIfEvent(display_, &nextEvent, checkEvent, reinterpret_cast<XPointer>(window_)))
			{
				if((nextEvent.type == KeyPress) && (nextEvent.xkey.keycode == event.xkey.keycode) &&
				   (event.xkey.time <= nextEvent.xkey.time) && (nextEvent.xkey.time <= event.xkey.time + 1))
				{
					// This sequence of events comes from maintaining a key down
					if(key_repeat_)
					{
						// Ignore the KeyRelease event and process the KeyPress event
						event = nextEvent;
						break;
					}
					else
					{
						// Ignore both events
						processThisEvent = false;
						break;
					}
				}
				else
				{
					// This sequence of events does not come from maintaining a key down,
					// so process the KeyRelease event normally,
					process_event(event);
					// but loop because the next event can be the first half
					// of a sequence coming from maintaining a key down.
					event = nextEvent;
				}
			}
			else
			{
				// No event after this KeyRelease event so assume it can be processed.
				break;
			}
		}

		if(processThisEvent)
		{
			process_event(event);
		}
	}

	// Process clipboard window events
	priv::clipboard_impl::process_events();
}

////////////////////////////////////////////////////////////
std::array<std::int32_t, 2> window_impl_x11::get_position() const
{
	// Get absolute position of our window relative to root window. This
	// takes into account all information that X11 has, including X11
	// border widths and any decorations. It corresponds to where the
	// window actually is, but not necessarily to where we told it to
	// go using setPosition() and XMoveWindow(). To have the two match
	// as expected, we may have to subtract decorations and borders.
	::Window child;
	int xAbsRelToRoot, yAbsRelToRoot;

	XTranslateCoordinates(display_, window_, DefaultRootWindow(display_), 0, 0, &xAbsRelToRoot,
						  &yAbsRelToRoot, &child);

	// CASE 1: some rare WMs actually put the window exactly where we tell
	// it to, even with decorations and such, which get shifted back.
	// In these rare cases, we can use the absolute value directly.
	if(isWMAbsolutePositionGood())
		return std::array<std::int32_t, 2>{{std::int32_t(xAbsRelToRoot), std::int32_t(yAbsRelToRoot)}};

	// CASE 2: most modern WMs support EWMH and can define _NET_FRAME_EXTENTS
	// with the exact frame size to subtract, so if present, we prefer it and
	// query it first. According to spec, this already includes any borders.
	long frame_left = 0;
	long frame_right = 0;
	long frame_top = 0;
	long frame_bottom = 0;

	if(getEWMHFrameExtents(display_, window_, frame_left, frame_right, frame_top, frame_bottom))
	{
		// Get final X/Y coordinates: subtract EWMH frame extents from
		// absolute window position.
		return std::array<std::int32_t, 2>{
			{std::int32_t(xAbsRelToRoot - frame_left), std::int32_t(yAbsRelToRoot - frame_top)}};
	}

	// CASE 3: EWMH frame extents were not available, use geometry.
	// We climb back up to the window before the root and use its
	// geometry information to extract X/Y position. This because
	// re-parenting WMs may re-parent the window multiple times, so
	// we'd have to climb up to the furthest ancestor and sum the
	// relative differences and borders anyway; and doing that to
	// subtract those values from the absolute coordinates of the
	// window is equivalent to going up the tree and asking the
	// furthest ancestor what it's relative distance to the root is.
	// So we use that approach because it's simpler.
	// This approach assumes that any window between the root and
	// our window is part of decorations/borders in some way. This
	// seems to hold true for most reasonable WM implementations.
	::Window ancestor = window_;
	::Window root = DefaultRootWindow(display_);

	while(getParentWindow(display_, ancestor) != root)
	{
		// Next window up (parent window).
		ancestor = getParentWindow(display_, ancestor);
	}

	// Get final X/Y coordinates: take the relative position to
	// the root of the furthest ancestor window.
	int xRelToRoot, yRelToRoot;
	unsigned int width, height, borderWidth, depth;

	XGetGeometry(display_, ancestor, &root, &xRelToRoot, &yRelToRoot, &width, &height, &borderWidth, &depth);

	return std::array<std::int32_t, 2>{{std::int32_t(xRelToRoot), std::int32_t(yRelToRoot)}};
}

////////////////////////////////////////////////////////////
void window_impl_x11::set_position(const std::array<std::int32_t, 2>& position)
{
	XMoveWindow(display_, window_, position[0], position[1]);
	XFlush(display_);
}

////////////////////////////////////////////////////////////
std::array<std::uint32_t, 2> window_impl_x11::get_size() const
{
	XWindowAttributes attributes;
	XGetWindowAttributes(display_, window_, &attributes);
	return std::array<std::uint32_t, 2>{
		{static_cast<std::uint32_t>(attributes.width), static_cast<std::uint32_t>(attributes.height)}};
}

////////////////////////////////////////////////////////////
std::array<std::int32_t, 4> window_impl_x11::get_frame_size() const
{
	long left = 0;
	long right = 0;
	long top = 0;
	long bottom = 0;
	if(getEWMHFrameExtents(display_, window_, left, right, top, bottom))
	{
		return {{static_cast<std::int32_t>(left), static_cast<std::int32_t>(top),
				 static_cast<std::int32_t>(right), static_cast<std::int32_t>(bottom)}};
	}
	return {{0, 0, 0, 0}};
}

////////////////////////////////////////////////////////////
void window_impl_x11::set_size(const std::array<std::uint32_t, 2>& size)
{
	// If resizing is disable for the window we have to update the size hints (required by some window
	// managers).
	if(use_size_hints_)
	{
		XSizeHints* sizeHints = XAllocSizeHints();

		long userhints;
		XGetWMNormalHints(display_, window_, sizeHints, &userhints);

		sizeHints->flags |= PMinSize | PMaxSize;
		sizeHints->min_width = sizeHints->max_width = size[0];
		sizeHints->min_height = sizeHints->max_height = size[1];
		XSetWMNormalHints(display_, window_, sizeHints);
		XFree(sizeHints);
	}

	XResizeWindow(display_, window_, size[0], size[1]);
	XFlush(display_);
}

////////////////////////////////////////////////////////////
void window_impl_x11::set_title(const std::string& title)
{
	// Bare X11 has no Unicode window title support.
	// There is however an option to tell the window manager your Unicode title via hints.

	// Convert to UTF-8 encoding.
	std::basic_string<std::uint8_t> utf8Title;
	utf32::to_utf8(title.begin(), title.end(), std::back_inserter(utf8Title));

	Atom useUtf8 = get_atom("UTF8_STRING", false);

	// Set the _NET_WM_NAME atom, which specifies a UTF-8 encoded window title.
	Atom wmName = get_atom("_NET_WM_NAME", false);
	XChangeProperty(display_, window_, wmName, useUtf8, 8, PropModeReplace, utf8Title.c_str(),
					utf8Title.size());

	// Set the _NET_WM_ICON_NAME atom, which specifies a UTF-8 encoded window title.
	Atom wmIconName = get_atom("_NET_WM_ICON_NAME", false);
	XChangeProperty(display_, window_, wmIconName, useUtf8, 8, PropModeReplace, utf8Title.c_str(),
					utf8Title.size());

// Set the non-Unicode title as a fallback for window managers who don't support _NET_WM_NAME.
#ifdef X_HAVE_UTF8_STRING
	Xutf8SetWMProperties(display_, window_, title.c_str(), title.c_str(), nullptr, 0, nullptr, nullptr,
						 nullptr);
#else
	XmbSetWMProperties(_display, _window, title.c_str(), title.c_str(), nullptr, 0, nullptr, nullptr,
					   nullptr);
#endif
}

////////////////////////////////////////////////////////////
void window_impl_x11::set_icon(unsigned int width, unsigned int height, const std::uint8_t* pixels)
{
	// X11 wants BGRA pixels: swap red and blue channels
	// Note: this memory will be freed by XDestroyImage
	std::uint8_t* iconPixels = static_cast<std::uint8_t*>(std::malloc(width * height * 4));
	for(std::size_t i = 0; i < width * height; ++i)
	{
		iconPixels[i * 4 + 0] = pixels[i * 4 + 2];
		iconPixels[i * 4 + 1] = pixels[i * 4 + 1];
		iconPixels[i * 4 + 2] = pixels[i * 4 + 0];
		iconPixels[i * 4 + 3] = pixels[i * 4 + 3];
	}

	// Create the icon pixmap
	Visual* defVisual = DefaultVisual(display_, screen_);
	unsigned int defDepth = DefaultDepth(display_, screen_);
	XImage* iconImage =
		XCreateImage(display_, defVisual, defDepth, ZPixmap, 0, (char*)iconPixels, width, height, 32, 0);
	if(!iconImage)
	{
		err() << "Failed to set the window's icon" << std::endl;
		return;
	}

	if(icon_pixmap_)
		XFreePixmap(display_, icon_pixmap_);

	if(icon_mask_pixmap_)
		XFreePixmap(display_, icon_mask_pixmap_);

	icon_pixmap_ = XCreatePixmap(display_, RootWindow(display_, screen_), width, height, defDepth);
	XGCValues values;
	GC iconGC = XCreateGC(display_, icon_pixmap_, 0, &values);
	XPutImage(display_, icon_pixmap_, iconGC, iconImage, 0, 0, 0, 0, width, height);
	XFreeGC(display_, iconGC);
	XDestroyImage(iconImage);

	// Create the mask pixmap (must have 1 bit depth)
	std::size_t pitch = (width + 7) / 8;
	std::vector<std::uint8_t> maskPixels(pitch * height, 0);
	for(std::size_t j = 0; j < height; ++j)
	{
		for(std::size_t i = 0; i < pitch; ++i)
		{
			for(std::size_t k = 0; k < 8; ++k)
			{
				if(i * 8 + k < width)
				{
					std::uint8_t opacity = (pixels[(i * 8 + k + j * width) * 4 + 3] > 0) ? 1 : 0;
					maskPixels[i + j * pitch] |= (opacity << k);
				}
			}
		}
	}
	icon_mask_pixmap_ =
		XCreatePixmapFromBitmapData(display_, window_, (char*)&maskPixels[0], width, height, 1, 0, 1);

	// Send our new icon to the window through the WMHints
	XWMHints* hints = XAllocWMHints();
	hints->flags = IconPixmapHint | IconMaskHint;
	hints->icon_pixmap = icon_pixmap_;
	hints->icon_mask = icon_mask_pixmap_;
	XSetWMHints(display_, window_, hints);
	XFree(hints);

	// ICCCM wants BGRA pixels: swap red and blue channels
	// ICCCM also wants the first 2 unsigned 32-bit values to be width and height
	std::vector<unsigned long> icccmIconPixels(2 + width * height, 0);
	unsigned long* ptr = &icccmIconPixels[0];

	*ptr++ = width;
	*ptr++ = height;

	for(std::size_t i = 0; i < width * height; ++i)
	{
		*ptr++ = (pixels[i * 4 + 2] << 0) | (pixels[i * 4 + 1] << 8) | (pixels[i * 4 + 0] << 16) |
				 (pixels[i * 4 + 3] << 24);
	}

	Atom netWmIcon = get_atom("_NET_WM_ICON");

	XChangeProperty(display_, window_, netWmIcon, XA_CARDINAL, 32, PropModeReplace,
					reinterpret_cast<const unsigned char*>(&icccmIconPixels[0]), 2 + width * height);

	XFlush(display_);
}

////////////////////////////////////////////////////////////
void window_impl_x11::set_visible(bool visible)
{
	if(visible)
	{
		XMapWindow(display_, window_);

		XFlush(display_);

		// Before continuing, make sure the WM has
		// internally marked the window as viewable
		while(!window_mapped_ && !is_external_)
			process_events();
	}
	else
	{
		XUnmapWindow(display_, window_);

		XFlush(display_);

		// Before continuing, make sure the WM has
		// internally marked the window as unviewable
		while(window_mapped_ && !is_external_)
			process_events();
	}
}

////////////////////////////////////////////////////////////
void window_impl_x11::maximize()
{

	Atom _NET_WM_STATE = get_atom("_NET_WM_STATE", false);

	Atom _NET_WM_STATE_MAXIMIZED_VERT = get_atom("_NET_WM_STATE_MAXIMIZED_VERT", false);

	Atom _NET_WM_STATE_MAXIMIZED_HORZ = get_atom("_NET_WM_STATE_MAXIMIZED_HORZ", false);

	if(window_mapped_)
	{

		XEvent e;
		std::memset(&e, 0, sizeof(e));

		e.xany.type = ClientMessage;
		e.xclient.message_type = _NET_WM_STATE;
		e.xclient.format = 32;
		e.xclient.window = window_;
		e.xclient.data.l[0] = _NET_WM_STATE_ADD;
		e.xclient.data.l[1] = _NET_WM_STATE_MAXIMIZED_VERT;
		e.xclient.data.l[2] = _NET_WM_STATE_MAXIMIZED_HORZ;
		e.xclient.data.l[3] = 0l;

		XSendEvent(display_, RootWindow(display_, screen_), 0,
				   SubstructureNotifyMask | SubstructureRedirectMask, &e);
	}
	else
	{
		// X11_SetNetWMState(_this, data->xwindow, window->flags);
	}

	XFlush(display_);
}

////////////////////////////////////////////////////////////
void window_impl_x11::minimize()
{
	XIconifyWindow(display_, window_, screen_);

	XFlush(display_);
}
static Bool is_map_notify(Display* /*dpy*/, XEvent* ev, XPointer win)
{
	return ev->type == MapNotify && ev->xmap.window == *((Window*)win);
}
////////////////////////////////////////////////////////////
void window_impl_x11::restore()
{
	{
		Atom _NET_WM_STATE = get_atom("_NET_WM_STATE", false);

		Atom _NET_WM_STATE_MAXIMIZED_VERT = get_atom("_NET_WM_STATE_MAXIMIZED_VERT", false);

		Atom _NET_WM_STATE_MAXIMIZED_HORZ = get_atom("_NET_WM_STATE_MAXIMIZED_HORZ", false);

		if(window_mapped_)
		{

			XEvent e;
			std::memset(&e, 0, sizeof(e));

			e.xany.type = ClientMessage;
			e.xclient.message_type = _NET_WM_STATE;
			e.xclient.format = 32;
			e.xclient.window = window_;
			e.xclient.data.l[0] = _NET_WM_STATE_REMOVE;
			e.xclient.data.l[1] = _NET_WM_STATE_MAXIMIZED_VERT;
			e.xclient.data.l[2] = _NET_WM_STATE_MAXIMIZED_HORZ;
			e.xclient.data.l[3] = 0l;

			XSendEvent(display_, RootWindow(display_, screen_), 0,
					   SubstructureNotifyMask | SubstructureRedirectMask, &e);
		}
		else
		{
			// X11_SetNetWMState(_this, data->xwindow, window->flags);
		}

		XFlush(display_);
	}

	{
		XEvent event;
		if(!window_mapped_)
		{
			XMapRaised(display_, window_);
			/* Blocking wait for "MapNotify" event.
			 * We use X11_XIfEvent because pXWindowEvent takes a mask rather than a type,
			 * and XCheckTypedWindowEvent doesn't block */

			XIfEvent(display_, &event, &is_map_notify, (XPointer)&window_);

			XFlush(display_);
		}
	}

	{
		Atom _NET_ACTIVE_WINDOW = get_atom("_NET_ACTIVE_WINDOW", false);

		if(window_mapped_)
		{
			XEvent e;
			std::memset(&e, 0, sizeof(e));

			e.xany.type = ClientMessage;
			e.xclient.message_type = _NET_ACTIVE_WINDOW;
			e.xclient.format = 32;
			e.xclient.window = window_;
			e.xclient.data.l[0] = 1; /* source indication. 1 = application */
			e.xclient.data.l[1] = last_input_time_;
			e.xclient.data.l[2] = 0;

			XSendEvent(display_, RootWindow(display_, screen_), 0,
					   SubstructureNotifyMask | SubstructureRedirectMask, &e);

			XFlush(display_);
		}
	}
}

////////////////////////////////////////////////////////////
void window_impl_x11::set_opacity(float opacity)
{
	const std::uint32_t fully_opaque = 0xFFFFFFFF;
	const long alpha = (long)((double)opacity * (double)fully_opaque);

	Atom property = get_atom("_NET_WM_WINDOW_OPACITY", false);
	if(property != None)
	{
		XChangeProperty(display_, window_, property, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&alpha,
						1);
		XFlush(display_);
	}
}

////////////////////////////////////////////////////////////
void window_impl_x11::set_mouse_cursor_visible(bool visible)
{
	cursor_visible_ = visible;
	XDefineCursor(display_, window_, cursor_visible_ ? last_cursor_ : hidden_cursor_);
	XFlush(display_);
}

////////////////////////////////////////////////////////////
void window_impl_x11::set_mouse_cursor(const cursor_impl& cursor)
{
	last_cursor_ = cursor.cursor_;
	if(cursor_visible_)
	{
		XDefineCursor(display_, window_, last_cursor_);
		XFlush(display_);
	}
}

////////////////////////////////////////////////////////////
void window_impl_x11::set_mouse_cursor_grabbed(bool grabbed)
{
	// This has no effect in fullscreen mode
	if(fullscreen_ || (cursor_grabbed_ == grabbed))
		return;

	if(grabbed)
	{
		// Try multiple times to grab the cursor
		for(unsigned int trial = 0; trial < maxTrialsCount; ++trial)
		{
			int result = XGrabPointer(display_, window_, True, None, GrabModeAsync, GrabModeAsync, window_,
									  None, CurrentTime);

			if(result == GrabSuccess)
			{
				cursor_grabbed_ = true;
				break;
			}

			// The cursor grab failed, trying again after a small sleep
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}

		if(!cursor_grabbed_)
			err() << "Failed to grab mouse cursor" << std::endl;
	}
	else
	{
		XUngrabPointer(display_, CurrentTime);
	}
}

////////////////////////////////////////////////////////////
void window_impl_x11::set_key_repeat_enabled(bool enabled)
{
	key_repeat_ = enabled;
}

////////////////////////////////////////////////////////////
void window_impl_x11::request_focus()
{
	// Focus is only stolen among mml windows, not between applications
	// Check the global list of windows to find out whether an mml window has the focus
	// Note: can't handle console and other non-mml windows belonging to the application.
	bool windowFocused = false;

	{
		std::lock_guard<std::mutex> lock(allWindowsMutex);
		for(std::vector<window_impl_x11*>::iterator itr = allWindows.begin(); itr != allWindows.end(); ++itr)
		{
			if((*itr)->has_focus())
			{
				windowFocused = true;
				break;
			}
		}
	}

	// Check if window is viewable (not on other desktop, ...)
	// TODO: Check also if minimized
	XWindowAttributes attributes;
	if(XGetWindowAttributes(display_, window_, &attributes) == 0)
	{
		mml::err() << "Failed to check if window is viewable while requesting focus" << std::endl;
		return; // error getting attribute
	}

	bool windowViewable = (attributes.map_state == IsViewable);

	if(windowFocused && windowViewable)
	{
		// Another mml window of this application has the focus and the current window is viewable:
		// steal focus (i.e. bring window to the front and give it input focus)
		grab_focus();
	}
	else
	{
		// Otherwise: display urgency hint (flashing application logo)
		// Ensure WM hints exist, allocate if necessary
		XWMHints* hints = XGetWMHints(display_, window_);
		if(hints == nullptr)
			hints = XAllocWMHints();

		// Add urgency (notification) flag to hints
		hints->flags |= XUrgencyHint;
		XSetWMHints(display_, window_, hints);
		XFree(hints);
	}
}

////////////////////////////////////////////////////////////
bool window_impl_x11::is_minimized() const
{
	Atom wm_state_atom = get_atom("WM_STATE", false);
	if(wm_state_atom == None)
	{
		return false;
	}
	Atom actual_type = None;
	int actual_format = 0;
	unsigned long nitems = 0;
	unsigned long bytes_after = 0;
	unsigned char* data = nullptr;
	const int status =
		XGetWindowProperty(display_, window_, wm_state_atom, 0, 2, False, AnyPropertyType, &actual_type,
						   &actual_format, &nitems, &bytes_after, &data);
	if(status != Success || data == nullptr || nitems == 0)
	{
		if(data != nullptr)
		{
			XFree(data);
		}
		return false;
	}
	const auto* states = reinterpret_cast<long*>(data);
	const long state = states[0];
	XFree(data);
	// ICCCM: IconicState == 3 (see X11/Xutil.h)
	return state == IconicState;
}

////////////////////////////////////////////////////////////
bool window_impl_x11::has_focus() const
{
	::Window focusedWindow = 0;
	int revertToReturn = 0;
	XGetInputFocus(display_, &focusedWindow, &revertToReturn);

	return (window_ == focusedWindow);
}

////////////////////////////////////////////////////////////
void window_impl_x11::grab_focus()
{
	Atom netActiveWindow = None;

	if(ewmhSupported())
		netActiveWindow = get_atom("_NET_ACTIVE_WINDOW");

	// Only try to grab focus if the window is mapped
	XWindowAttributes attr;

	XGetWindowAttributes(display_, window_, &attr);

	if(attr.map_state == IsUnmapped)
		return;

	if(netActiveWindow)
	{
		XEvent event;
		std::memset(&event, 0, sizeof(event));

		event.type = ClientMessage;
		event.xclient.window = window_;
		event.xclient.format = 32;
		event.xclient.message_type = netActiveWindow;
		event.xclient.data.l[0] = 1; // Normal application
		event.xclient.data.l[1] = last_input_time_;
		event.xclient.data.l[2] = 0; // We don't know the currently active window

		int result = XSendEvent(display_, DefaultRootWindow(display_), False,
								SubstructureNotifyMask | SubstructureRedirectMask, &event);

		XFlush(display_);

		if(!result)
			err() << "Setting fullscreen failed, could not send \"_NET_ACTIVE_WINDOW\" event" << std::endl;
	}
	else
	{
		XRaiseWindow(display_, window_);
		XSetInputFocus(display_, window_, RevertToPointerRoot, CurrentTime);
		XFlush(display_);
	}
}

////////////////////////////////////////////////////////////
void window_impl_x11::set_protocols()
{
	Atom wmProtocols = get_atom("WM_PROTOCOLS");
	Atom wmDeleteWindow = get_atom("WM_DELETE_WINDOW");

	if(!wmProtocols)
	{
		err() << "Failed to request WM_PROTOCOLS atom." << std::endl;
		return;
	}

	std::vector<Atom> atoms;

	if(wmDeleteWindow)
	{
		atoms.push_back(wmDeleteWindow);
	}
	else
	{
		err() << "Failed to request WM_DELETE_WINDOW atom." << std::endl;
	}

	Atom netWmPing = None;
	Atom netWmPid = None;

	if(ewmhSupported())
	{
		netWmPing = get_atom("_NET_WM_PING", true);
		netWmPid = get_atom("_NET_WM_PID", true);
	}

	if(netWmPing && netWmPid)
	{
		const long pid = getpid();

		XChangeProperty(display_, window_, netWmPid, XA_CARDINAL, 32, PropModeReplace,
						reinterpret_cast<const unsigned char*>(&pid), 1);

		atoms.push_back(netWmPing);
	}

	if(!atoms.empty())
	{
		XChangeProperty(display_, window_, wmProtocols, XA_ATOM, 32, PropModeReplace,
						reinterpret_cast<const unsigned char*>(&atoms[0]), atoms.size());
	}
	else
	{
		err() << "Didn't set any window protocols" << std::endl;
	}
}

////////////////////////////////////////////////////////////
void window_impl_x11::initialize(bool visible)
{
	// Create the input context
	input_method_ = XOpenIM(display_, nullptr, nullptr, nullptr);

	if(input_method_)
	{
		input_context_ = XCreateIC(input_method_, XNClientWindow, window_, XNFocusWindow, window_,
								   XNInputStyle, XIMPreeditNothing | XIMStatusNothing, nullptr);
	}
	else
	{
		input_context_ = nullptr;
	}

	if(!input_context_)
		err() << "Failed to create input context for window -- text_entered event won't be able to return "
				 "unicode"
			  << std::endl;

	Atom wmWindowType = get_atom("_NET_WM_WINDOW_TYPE", false);
	Atom wmWindowTypeNormal = get_atom("_NET_WM_WINDOW_TYPE_NORMAL", false);

	if(wmWindowType && wmWindowTypeNormal)
	{
		XChangeProperty(display_, window_, wmWindowType, XA_ATOM, 32, PropModeReplace,
						reinterpret_cast<const unsigned char*>(&wmWindowTypeNormal), 1);
	}

	// Show the window
	set_visible(visible);

	// Raise the window and grab input focus
	grab_focus();

	// Create the hidden cursor
	create_hidden_cursor();

	// Flush the commands queue
	XFlush(display_);

	// Add this window to the global list of windows (required for focus request)
	std::lock_guard<std::mutex> lock(allWindowsMutex);
	allWindows.push_back(this);
}

////////////////////////////////////////////////////////////
void window_impl_x11::update_last_input_time(::Time time)
{
	if(time && (time != last_input_time_))
	{
		Atom netWmUserTime = get_atom("_NET_WM_USER_TIME", true);

		if(netWmUserTime)
		{
			XChangeProperty(display_, window_, netWmUserTime, XA_CARDINAL, 32, PropModeReplace,
							reinterpret_cast<const unsigned char*>(&time), 1);
		}

		last_input_time_ = time;
	}
}

////////////////////////////////////////////////////////////
void window_impl_x11::create_hidden_cursor()
{
	// Create the cursor's pixmap (1x1 pixels)
	Pixmap cursorPixmap = XCreatePixmap(display_, window_, 1, 1, 1);
	GC graphicsContext = XCreateGC(display_, cursorPixmap, 0, nullptr);
	XDrawPoint(display_, cursorPixmap, graphicsContext, 0, 0);
	XFreeGC(display_, graphicsContext);

	// Create the cursor, using the pixmap as both the shape and the mask of the cursor
	XColor color;
	color.flags = DoRed | DoGreen | DoBlue;
	color.red = color.blue = color.green = 0;
	hidden_cursor_ = XCreatePixmapCursor(display_, cursorPixmap, cursorPixmap, &color, &color, 0, 0);

	// We don't need the pixmap any longer, free it
	XFreePixmap(display_, cursorPixmap);
}

////////////////////////////////////////////////////////////
void window_impl_x11::cleanup()
{
	// Restore the previous video mode (in case we were running in fullscreen)
	// reset_video_mode();

	// Unhide the mouse cursor (in case it was hidden)
	set_mouse_cursor_visible(true);
}

////////////////////////////////////////////////////////////
bool window_impl_x11::process_event(XEvent& windowEvent)
{
	// Convert the X11 event to a Event
	switch(windowEvent.type)
	{
		// Destroy event
		case DestroyNotify:
		{
			// The window is about to be destroyed: we must cleanup resources
			cleanup();
			break;
		}

			// Gain focus event
		case FocusIn:
		{
			// Update the input context
			if(input_context_)
				XSetICFocus(input_context_);

			// Grab cursor
			if(cursor_grabbed_)
			{
				// Try multiple times to grab the cursor
				for(unsigned int trial = 0; trial < maxTrialsCount; ++trial)
				{
					int result = XGrabPointer(display_, window_, True, None, GrabModeAsync, GrabModeAsync,
											  window_, None, CurrentTime);

					if(result == GrabSuccess)
					{
						cursor_grabbed_ = true;
						break;
					}

					// The cursor grab failed, trying again after a small sleep
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
				}

				if(!cursor_grabbed_)
					err() << "Failed to grab mouse cursor" << std::endl;
			}

			platform_event event;
			event.type = platform_event::gained_focus;
			push_event(event);

			// If the window has been previously marked urgent (notification) as a result of a focus request,
			// undo that
			XWMHints* hints = XGetWMHints(display_, window_);
			if(hints != nullptr)
			{
				// Remove urgency (notification) flag from hints
				hints->flags &= ~XUrgencyHint;
				XSetWMHints(display_, window_, hints);
				XFree(hints);
			}

			break;
		}

			// Lost focus event
		case FocusOut:
		{
			// Update the input context
			if(input_context_)
				XUnsetICFocus(input_context_);

			// Release cursor
			if(cursor_grabbed_)
				XUngrabPointer(display_, CurrentTime);

			platform_event event;
			event.type = platform_event::lost_focus;
			push_event(event);
			break;
		}

		// Resize event
		case ConfigureNotify:
		{
			/* Real configure notify events are relative to the parent, synthetic events are absolute. */
			if(!windowEvent.xconfigure.send_event)
			{
				unsigned int NumChildren;
				Window ChildReturn, Root, Parent;
				Window* Children;
				/* Translate these coodinates back to relative to root */
				XQueryTree(display_, windowEvent.xconfigure.window, &Root, &Parent, &Children, &NumChildren);
				XTranslateCoordinates(windowEvent.xconfigure.display, Parent,
									  DefaultRootWindow(windowEvent.xconfigure.display),
									  windowEvent.xconfigure.x, windowEvent.xconfigure.y,
									  &windowEvent.xconfigure.x, &windowEvent.xconfigure.y, &ChildReturn);
			}
			if((windowEvent.xconfigure.x != previous_pos_[0]) ||
			   (windowEvent.xconfigure.y != previous_pos_[1]))
			{
				platform_event event;
				event.type = platform_event::moved;
				event.move.x = windowEvent.xconfigure.x;
				event.move.y = windowEvent.xconfigure.y;
				push_event(event);

				previous_pos_[0] = windowEvent.xconfigure.x;
				previous_pos_[1] = windowEvent.xconfigure.y;
			}
			// ConfigureNotify can be triggered for other reasons, check if the size has actually changed
			if((windowEvent.xconfigure.width != previous_size_[0]) ||
			   (windowEvent.xconfigure.height != previous_size_[1]))
			{
				platform_event event;
				event.type = platform_event::resized;
				event.size.width = windowEvent.xconfigure.width;
				event.size.height = windowEvent.xconfigure.height;
				push_event(event);

				previous_size_[0] = windowEvent.xconfigure.width;
				previous_size_[1] = windowEvent.xconfigure.height;
			}
			break;
		}

			// Close event
		case ClientMessage:
		{
			// Input methods might want random ClientMessage events
			if(!XFilterEvent(&windowEvent, None))
			{
				static Atom wmProtocols = get_atom("WM_PROTOCOLS");

				// Handle window manager protocol messages we support
				if(windowEvent.xclient.message_type == wmProtocols)
				{
					static Atom wmDeleteWindow = get_atom("WM_DELETE_WINDOW");
					static Atom netWmPing = ewmhSupported() ? get_atom("_NET_WM_PING", true) : None;

					if((windowEvent.xclient.format == 32) &&
					   (windowEvent.xclient.data.l[0]) == static_cast<long>(wmDeleteWindow))
					{
						// Handle the WM_DELETE_WINDOW message
						platform_event event;
						event.type = platform_event::closed;
						push_event(event);
					}
					else if(netWmPing && (windowEvent.xclient.format == 32) &&
							(windowEvent.xclient.data.l[0]) == static_cast<long>(netWmPing))
					{
						// Handle the _NET_WM_PING message, send pong back to WM to show that we are
						// responsive
						windowEvent.xclient.window = DefaultRootWindow(display_);

						XSendEvent(display_, DefaultRootWindow(display_), False,
								   SubstructureNotifyMask | SubstructureRedirectMask, &windowEvent);
					}
				}
			}
			break;
		}

			// Key down event
		case KeyPress:
		{
			// Fill the event parameters
			// TODO: if modifiers are wrong, use XGetModifierMapping to retrieve the actual modifiers mapping
			platform_event event;
			event.type = platform_event::key_pressed;
			event.key.code = keyboard_impl::get_key_from_event(windowEvent.xkey);
			event.key.scancode = keyboard_impl::get_scancode_from_event(windowEvent.xkey);
			event.key.alt = windowEvent.xkey.state & Mod1Mask;
			event.key.control = windowEvent.xkey.state & ControlMask;
			event.key.shift = windowEvent.xkey.state & ShiftMask;
			event.key.system = windowEvent.xkey.state & Mod4Mask;

			const bool filtered = XFilterEvent(&windowEvent, None);

			// Generate a KeyPressed event if needed
			if(filtered)
			{
				push_event(event);
				isKeyFiltered.set(windowEvent.xkey.keycode);
			}
			else
			{
				// Push a KeyPressed event if the key has never been filtered before
				// (a KeyPressed event would have already been pushed if it had been filtered).
				//
				// Some dummy IMs (like the built-in one you get by setting XMODIFIERS=@im=none)
				// never filter events away, and we have to take care of that.
				//
				// In addition, ignore text-only KeyPress events generated by IMs (with keycode set to 0).
				if(!isKeyFiltered.test(windowEvent.xkey.keycode) && windowEvent.xkey.keycode != 0)
					push_event(event);
			}

			// Generate TextEntered events if needed
			if(!filtered)
			{
#ifdef X_HAVE_UTF8_STRING
				if(input_context_)
				{
					Status status;
					std::uint8_t keyBuffer[64];

					int length = Xutf8LookupString(input_context_, &windowEvent.xkey,
												   reinterpret_cast<char*>(keyBuffer), sizeof(keyBuffer),
												   nullptr, &status);

					if(status == XBufferOverflow)
						err() << "A TextEntered event has more than 64 bytes of UTF-8 input, and "
								 "has been discarded\nThis means either you have typed a very long string "
								 "(more than 20 chars), or your input method is broken in obscure ways."
							  << std::endl;
					else if(status == XLookupChars)
					{
						// There might be more than 1 characters in this event,
						// so we must iterate it
						std::uint32_t unicode = 0;
						std::uint8_t* iter = keyBuffer;
						while(iter < keyBuffer + length)
						{
							iter = utf8::decode(iter, keyBuffer + length, unicode, 0);
							if(unicode != 0)
							{
								platform_event textEvent;
								textEvent.type = platform_event::text_entered;
								textEvent.text.unicode = unicode;
								push_event(textEvent);
							}
						}
					}
				}
				else
#endif
				{
					static XComposeStatus status;
					char keyBuffer[16];
					if(XLookupString(&windowEvent.xkey, keyBuffer, sizeof(keyBuffer), nullptr, &status))
					{
						platform_event textEvent;
						textEvent.type = platform_event::text_entered;
						textEvent.text.unicode = static_cast<std::uint32_t>(keyBuffer[0]);
						push_event(textEvent);
					}
				}
			}

			update_last_input_time(windowEvent.xkey.time);

			break;
		}

			// Key up event
		case KeyRelease:
		{
			// Fill the event parameters
			platform_event event;
			event.type = platform_event::key_released;
			event.key.code = keyboard_impl::get_key_from_event(windowEvent.xkey);
			event.key.scancode = keyboard_impl::get_scancode_from_event(windowEvent.xkey);
			event.key.alt = windowEvent.xkey.state & Mod1Mask;
			event.key.control = windowEvent.xkey.state & ControlMask;
			event.key.shift = windowEvent.xkey.state & ShiftMask;
			event.key.system = windowEvent.xkey.state & Mod4Mask;
			push_event(event);

			break;
		}

			// Mouse button pressed
		case ButtonPress:
		{
			// Buttons 4 and 5 are the vertical wheel and 6 and 7 the horizontal wheel.
			unsigned int button = windowEvent.xbutton.button;
			if((button == Button1) || (button == Button2) || (button == Button3) || (button == 8) ||
			   (button == 9))
			{
				platform_event event;
				event.type = platform_event::mouse_button_pressed;
				event.mouse_button.x = windowEvent.xbutton.x;
				event.mouse_button.y = windowEvent.xbutton.y;

				// clang-format off
				switch(button)
				{
					case Button1: event.mouse_button.button = mouse::left;     break;
					case Button2: event.mouse_button.button = mouse::middle;   break;
					case Button3: event.mouse_button.button = mouse::right;    break;
					case 8:       event.mouse_button.button = mouse::x_button1; break;
					case 9:       event.mouse_button.button = mouse::x_button2; break;
				}
				// clang-format on

				push_event(event);
			}

			update_last_input_time(windowEvent.xbutton.time);

			break;
		}

			// Mouse button released
		case ButtonRelease:
		{
			unsigned int button = windowEvent.xbutton.button;
			if((button == Button1) || (button == Button2) || (button == Button3) || (button == 8) ||
			   (button == 9))
			{
				platform_event event;
				event.type = platform_event::mouse_button_released;
				event.mouse_button.x = windowEvent.xbutton.x;
				event.mouse_button.y = windowEvent.xbutton.y;
				switch(button)
				{
					case Button1:
						event.mouse_button.button = mouse::left;
						break;
					case Button2:
						event.mouse_button.button = mouse::middle;
						break;
					case Button3:
						event.mouse_button.button = mouse::right;
						break;
					case 8:
						event.mouse_button.button = mouse::x_button1;
						break;
					case 9:
						event.mouse_button.button = mouse::x_button2;
						break;
				}
				push_event(event);
			}
			else if((button == Button4) || (button == Button5))
			{
				platform_event event;

				event.type = platform_event::mouse_wheel_scrolled;
				event.mouse_wheel_scroll.wheel = mouse::vertical_wheel;
				event.mouse_wheel_scroll.delta = (button == Button4) ? 1 : -1;
				event.mouse_wheel_scroll.x = windowEvent.xbutton.x;
				event.mouse_wheel_scroll.y = windowEvent.xbutton.y;
				push_event(event);
			}
			else if((button == 6) || (button == 7))
			{
				platform_event event;
				event.type = platform_event::mouse_wheel_scrolled;
				event.mouse_wheel_scroll.wheel = mouse::horizontal_wheel;
				event.mouse_wheel_scroll.delta = (button == 6) ? 1 : -1;
				event.mouse_wheel_scroll.x = windowEvent.xbutton.x;
				event.mouse_wheel_scroll.y = windowEvent.xbutton.y;
				push_event(event);
			}
			break;
		}

			// Mouse moved
		case MotionNotify:
		{
			platform_event event;
			event.type = platform_event::mouse_moved;
			event.mouse_move.x = windowEvent.xmotion.x;
			event.mouse_move.y = windowEvent.xmotion.y;
			push_event(event);
			break;
		}

			// Mouse entered
		case EnterNotify:
		{
			if(windowEvent.xcrossing.mode == NotifyNormal)
			{
				platform_event event;
				event.type = platform_event::mouse_entered;
				push_event(event);
			}
			break;
		}

			// Mouse left
		case LeaveNotify:
		{
			if(windowEvent.xcrossing.mode == NotifyNormal)
			{
				platform_event event;
				event.type = platform_event::mouse_left;
				push_event(event);
			}
			break;
		}

			// Keyboard mapping changed
		case MappingNotify:
		{
			if(windowEvent.xmapping.request == MappingKeyboard)
				XRefreshKeyboardMapping(&windowEvent.xmapping);

			break;
		}

			// Window unmapped
		case UnmapNotify:
		{
			if(windowEvent.xunmap.window == window_)
				window_mapped_ = false;

			break;
		}

			// Window visibility change
		case VisibilityNotify:
		{
			// We prefer using VisibilityNotify over MapNotify because
			// some window managers like awesome don't internally flag a
			// window as viewable even after it is mapped but before it
			// is visible leading to certain function calls failing with
			// an unviewable error if called before VisibilityNotify arrives

			// Empirical testing on most widely used window managers shows
			// that mapping a window will always lead to a VisibilityNotify
			// event that is not VisibilityFullyObscured
			if(windowEvent.xvisibility.window == window_)
			{
				if(windowEvent.xvisibility.state != VisibilityFullyObscured)
					window_mapped_ = true;
			}

			break;
		}

			// Window property change
		case PropertyNotify:
		{
			if(!last_input_time_)
				last_input_time_ = windowEvent.xproperty.time;

			break;
		}
	}

	return true;
}

////////////////////////////////////////////////////////////
bool window_impl_x11::checkXRandR(int& xRandRMajor, int& xRandRMinor)
{
	// Check if the XRandR extension is present
	int version;
	if(!XQueryExtension(display_, "RANDR", &version, &version, &version))
	{
		err() << "XRandR extension is not supported" << std::endl;
		return false;
	}

	// Check XRandR version, 1.2 required
	if(!XRRQueryVersion(display_, &xRandRMajor, &xRandRMinor) || xRandRMajor < 1 ||
	   (xRandRMajor == 1 && xRandRMinor < 2))
	{
		err() << "XRandR is too old" << std::endl;
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////
RROutput window_impl_x11::getOutputPrimary(::Window& rootWindow, XRRScreenResources* res, int xRandRMajor,
										   int xRandRMinor)
{
	// if xRandR version >= 1.3 get the primary screen else take the first screen
	if((xRandRMajor == 1 && xRandRMinor >= 3) || xRandRMajor > 1)
	{
		RROutput output = XRRGetOutputPrimary(display_, rootWindow);

		// Check if returned output is valid, otherwise use the first screen
		if(output == None)
			return res->outputs[0];
		else
			return output;
	}

	// xRandr version can't get the primary screen, use the first screen
	return res->outputs[0];
}

} // namespace priv

} // namespace mml
