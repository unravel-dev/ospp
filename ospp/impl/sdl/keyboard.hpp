#pragma once
#include "../../keyboard.h"

#include "window.hpp"
#include "config.hpp"
#include <cassert>
#include <map>
namespace os
{
namespace detail
{
namespace sdl
{

namespace
{
struct mappings
{
	void init_from(const std::map<key::code, SDL_Scancode>& map)
	{
		os_to_impl_scancode = map;

		for(const auto& kvp : map)
		{
			auto scancode = kvp.second;
			auto it = impl_scancode_to_os.find(scancode);
			if(it != std::end(impl_scancode_to_os))
			{
				auto name = SDL_GetScancodeName(kvp.second);
				(void)name;
				assert(false && "scancode already added");
			}
			impl_scancode_to_os[scancode] = kvp.first;

			//            auto name = SDL_GetScancodeName(kvp.second);
			//            string_to_key_code[name] = kvp.first;
			//            std::cout << name << std::endl;
		}
	}

	//    std::map<std::string, key::code> string_to_key_code;
	std::map<SDL_Scancode, key::code> impl_scancode_to_os;
	std::map<key::code, SDL_Scancode> os_to_impl_scancode;
};

mappings key_map = []()
{
	mappings m;
	m.init_from({
		{key::a, SDL_SCANCODE_A},
		{key::b, SDL_SCANCODE_B},
		{key::c, SDL_SCANCODE_C},
		{key::d, SDL_SCANCODE_D},
		{key::e, SDL_SCANCODE_E},
		{key::f, SDL_SCANCODE_F},
		{key::g, SDL_SCANCODE_G},
		{key::h, SDL_SCANCODE_H},
		{key::i, SDL_SCANCODE_I},
		{key::j, SDL_SCANCODE_J},
		{key::k, SDL_SCANCODE_K},
		{key::l, SDL_SCANCODE_L},
		{key::m, SDL_SCANCODE_M},
		{key::n, SDL_SCANCODE_N},
		{key::o, SDL_SCANCODE_O},
		{key::p, SDL_SCANCODE_P},
		{key::q, SDL_SCANCODE_Q},
		{key::r, SDL_SCANCODE_R},
		{key::s, SDL_SCANCODE_S},
		{key::t, SDL_SCANCODE_T},
		{key::u, SDL_SCANCODE_U},
		{key::v, SDL_SCANCODE_V},
		{key::w, SDL_SCANCODE_W},
		{key::x, SDL_SCANCODE_X},
		{key::y, SDL_SCANCODE_Y},
		{key::z, SDL_SCANCODE_Z},
		{key::digit1, SDL_SCANCODE_1},
		{key::digit2, SDL_SCANCODE_2},
		{key::digit3, SDL_SCANCODE_3},
		{key::digit4, SDL_SCANCODE_4},
		{key::digit5, SDL_SCANCODE_5},
		{key::digit6, SDL_SCANCODE_6},
		{key::digit7, SDL_SCANCODE_7},
		{key::digit8, SDL_SCANCODE_8},
		{key::digit9, SDL_SCANCODE_9},
		{key::digit0, SDL_SCANCODE_0},

		{key::enter, SDL_SCANCODE_RETURN},
		{key::escape, SDL_SCANCODE_ESCAPE},
		{key::backspace, SDL_SCANCODE_BACKSPACE},
		{key::tab, SDL_SCANCODE_TAB},
		{key::space, SDL_SCANCODE_SPACE},

		{key::minus, SDL_SCANCODE_MINUS},
		{key::equals, SDL_SCANCODE_EQUALS},
		{key::leftbracket, SDL_SCANCODE_LEFTBRACKET},
		{key::rightbracket, SDL_SCANCODE_RIGHTBRACKET},
		{key::backslash, SDL_SCANCODE_BACKSLASH},
		{key::nonushash, SDL_SCANCODE_NONUSHASH},
		{key::semicolon, SDL_SCANCODE_SEMICOLON},
		{key::apostrophe, SDL_SCANCODE_APOSTROPHE},
		{key::grave, SDL_SCANCODE_GRAVE},
		{key::comma, SDL_SCANCODE_COMMA},
		{key::period, SDL_SCANCODE_PERIOD},
		{key::slash, SDL_SCANCODE_SLASH},

		{key::capslock, SDL_SCANCODE_CAPSLOCK},

		{key::f1, SDL_SCANCODE_F1},
		{key::f2, SDL_SCANCODE_F2},
		{key::f3, SDL_SCANCODE_F3},
		{key::f4, SDL_SCANCODE_F4},
		{key::f5, SDL_SCANCODE_F5},
		{key::f6, SDL_SCANCODE_F6},
		{key::f7, SDL_SCANCODE_F7},
		{key::f8, SDL_SCANCODE_F8},
		{key::f9, SDL_SCANCODE_F9},
		{key::f10, SDL_SCANCODE_F10},
		{key::f11, SDL_SCANCODE_F11},
		{key::f12, SDL_SCANCODE_F12},

		{key::printscreen, SDL_SCANCODE_PRINTSCREEN},
		{key::scrolllock, SDL_SCANCODE_SCROLLLOCK},
		{key::pause, SDL_SCANCODE_PAUSE},
		{key::insert, SDL_SCANCODE_INSERT},
		{key::home, SDL_SCANCODE_HOME},
		{key::pageup, SDL_SCANCODE_PAGEUP},
		{key::del, SDL_SCANCODE_DELETE},
		{key::end, SDL_SCANCODE_END},
		{key::pagedown, SDL_SCANCODE_PAGEDOWN},
		{key::right, SDL_SCANCODE_RIGHT},
		{key::left, SDL_SCANCODE_LEFT},
		{key::down, SDL_SCANCODE_DOWN},
		{key::up, SDL_SCANCODE_UP},

		{key::numlockclear, SDL_SCANCODE_NUMLOCKCLEAR},

		{key::kp_divide, SDL_SCANCODE_KP_DIVIDE},
		{key::kp_multiply, SDL_SCANCODE_KP_MULTIPLY},
		{key::kp_minus, SDL_SCANCODE_KP_MINUS},
		{key::kp_plus, SDL_SCANCODE_KP_PLUS},
		{key::kp_enter, SDL_SCANCODE_KP_ENTER},
		{key::kp_digit1, SDL_SCANCODE_KP_0},
		{key::kp_digit2, SDL_SCANCODE_KP_1},
		{key::kp_digit3, SDL_SCANCODE_KP_2},
		{key::kp_digit4, SDL_SCANCODE_KP_3},
		{key::kp_digit5, SDL_SCANCODE_KP_4},
		{key::kp_digit6, SDL_SCANCODE_KP_5},
		{key::kp_digit7, SDL_SCANCODE_KP_6},
		{key::kp_digit8, SDL_SCANCODE_KP_7},
		{key::kp_digit9, SDL_SCANCODE_KP_8},
		{key::kp_digit0, SDL_SCANCODE_KP_9},
		{key::kp_period, SDL_SCANCODE_KP_PERIOD},

		{key::nonusbackslash, SDL_SCANCODE_NONUSBACKSLASH},
		{key::application, SDL_SCANCODE_APPLICATION},
		{key::power, SDL_SCANCODE_POWER},
		{key::kp_equals, SDL_SCANCODE_KP_EQUALS},
		{key::f13, SDL_SCANCODE_F13},
		{key::f14, SDL_SCANCODE_F14},
		{key::f15, SDL_SCANCODE_F15},
		{key::f16, SDL_SCANCODE_F16},
		{key::f17, SDL_SCANCODE_F17},
		{key::f18, SDL_SCANCODE_F18},
		{key::f19, SDL_SCANCODE_F19},
		{key::f20, SDL_SCANCODE_F20},
		{key::f21, SDL_SCANCODE_F21},
		{key::f22, SDL_SCANCODE_F22},
		{key::f23, SDL_SCANCODE_F23},
		{key::f24, SDL_SCANCODE_F24},
		{key::execute, SDL_SCANCODE_EXECUTE},
		{key::help, SDL_SCANCODE_HELP},
		{key::menu, SDL_SCANCODE_MENU},
		{key::select, SDL_SCANCODE_SELECT},
		{key::stop, SDL_SCANCODE_STOP},
		{key::again, SDL_SCANCODE_AGAIN},
		{key::undo, SDL_SCANCODE_UNDO},
		{key::cut, SDL_SCANCODE_CUT},
		{key::copy, SDL_SCANCODE_COPY},
		{key::paste, SDL_SCANCODE_PASTE},
		{key::find, SDL_SCANCODE_FIND},
		{key::mute, SDL_SCANCODE_MUTE},
		{key::volumeup, SDL_SCANCODE_VOLUMEUP},
		{key::volumedown, SDL_SCANCODE_VOLUMEDOWN},
		{key::kp_comma, SDL_SCANCODE_KP_COMMA},
		{key::kp_equalsas400, SDL_SCANCODE_KP_EQUALSAS400},
		{key::international1, SDL_SCANCODE_INTERNATIONAL1},
		{key::international2, SDL_SCANCODE_INTERNATIONAL2},
		{key::international3, SDL_SCANCODE_INTERNATIONAL3},
		{key::international4, SDL_SCANCODE_INTERNATIONAL4},
		{key::international5, SDL_SCANCODE_INTERNATIONAL5},
		{key::international6, SDL_SCANCODE_INTERNATIONAL6},
		{key::international7, SDL_SCANCODE_INTERNATIONAL7},
		{key::international8, SDL_SCANCODE_INTERNATIONAL8},
		{key::international9, SDL_SCANCODE_INTERNATIONAL9},
		{key::lang1, SDL_SCANCODE_LANG1},
		{key::lang2, SDL_SCANCODE_LANG2},
		{key::lang3, SDL_SCANCODE_LANG3},
		{key::lang4, SDL_SCANCODE_LANG4},
		{key::lang5, SDL_SCANCODE_LANG5},
		{key::lang6, SDL_SCANCODE_LANG6},
		{key::lang7, SDL_SCANCODE_LANG7},
		{key::lang8, SDL_SCANCODE_LANG8},
		{key::lang9, SDL_SCANCODE_LANG9},

		{key::alterase, SDL_SCANCODE_ALTERASE},
		{key::sysreq, SDL_SCANCODE_SYSREQ},
		{key::cancel, SDL_SCANCODE_CANCEL},
		{key::clear, SDL_SCANCODE_CLEAR},
		{key::prior, SDL_SCANCODE_PRIOR},
		{key::return2, SDL_SCANCODE_RETURN2},
		{key::separator, SDL_SCANCODE_SEPARATOR},
		{key::out, SDL_SCANCODE_OUT},
		{key::oper, SDL_SCANCODE_OPER},
		{key::clearagain, SDL_SCANCODE_CLEARAGAIN},
		{key::crsel, SDL_SCANCODE_CRSEL},
		{key::exsel, SDL_SCANCODE_EXSEL},
		{key::kp_digit00, SDL_SCANCODE_KP_00},
		{key::kp_digit000, SDL_SCANCODE_KP_000},
		{key::thousandsseparator, SDL_SCANCODE_THOUSANDSSEPARATOR},
		{key::decimalseparator, SDL_SCANCODE_DECIMALSEPARATOR},
		{key::currencyunit, SDL_SCANCODE_CURRENCYUNIT},
		{key::currencysubunit, SDL_SCANCODE_CURRENCYSUBUNIT},
		{key::kp_leftparen, SDL_SCANCODE_KP_LEFTPAREN},
		{key::kp_rightparen, SDL_SCANCODE_KP_RIGHTPAREN},
		{key::kp_leftbrace, SDL_SCANCODE_KP_LEFTBRACE},
		{key::kp_rightbrace, SDL_SCANCODE_KP_RIGHTBRACE},
		{key::kp_tab, SDL_SCANCODE_KP_TAB},
		{key::kp_backspace, SDL_SCANCODE_KP_BACKSPACE},
		{key::kp_a, SDL_SCANCODE_KP_A},
		{key::kp_b, SDL_SCANCODE_KP_B},
		{key::kp_c, SDL_SCANCODE_KP_C},
		{key::kp_d, SDL_SCANCODE_KP_D},
		{key::kp_e, SDL_SCANCODE_KP_E},
		{key::kp_f, SDL_SCANCODE_KP_F},
		{key::kp_xor, SDL_SCANCODE_KP_XOR},
		{key::kp_power, SDL_SCANCODE_KP_POWER},
		{key::kp_percent, SDL_SCANCODE_KP_PERCENT},
		{key::kp_less, SDL_SCANCODE_KP_LESS},
		{key::kp_greater, SDL_SCANCODE_KP_GREATER},
		{key::kp_ampersand, SDL_SCANCODE_KP_AMPERSAND},
		{key::kp_dblampersand, SDL_SCANCODE_KP_DBLAMPERSAND},
		{key::kp_verticalbar, SDL_SCANCODE_KP_VERTICALBAR},
		{key::kp_dblverticalbar, SDL_SCANCODE_KP_DBLVERTICALBAR},
		{key::kp_colon, SDL_SCANCODE_KP_COLON},
		{key::kp_hash, SDL_SCANCODE_KP_HASH},
		{key::kp_space, SDL_SCANCODE_KP_SPACE},
		{key::kp_at, SDL_SCANCODE_KP_AT},
		{key::kp_exclam, SDL_SCANCODE_KP_EXCLAM},
		{key::kp_memstore, SDL_SCANCODE_KP_MEMSTORE},
		{key::kp_memrecall, SDL_SCANCODE_KP_MEMRECALL},
		{key::kp_memclear, SDL_SCANCODE_KP_MEMCLEAR},
		{key::kp_memadd, SDL_SCANCODE_KP_MEMADD},
		{key::kp_memsubtract, SDL_SCANCODE_KP_MEMSUBTRACT},
		{key::kp_memmultiply, SDL_SCANCODE_KP_MEMMULTIPLY},
		{key::kp_memdivide, SDL_SCANCODE_KP_MEMDIVIDE},
		{key::kp_plusminus, SDL_SCANCODE_KP_PLUSMINUS},
		{key::kp_clear, SDL_SCANCODE_KP_CLEAR},
		{key::kp_clearentry, SDL_SCANCODE_KP_CLEARENTRY},
		{key::kp_binary, SDL_SCANCODE_KP_BINARY},
		{key::kp_octal, SDL_SCANCODE_KP_OCTAL},
		{key::kp_decimal, SDL_SCANCODE_KP_DECIMAL},
		{key::kp_hexadecimal, SDL_SCANCODE_KP_HEXADECIMAL},
		{key::lctrl, SDL_SCANCODE_LCTRL},
		{key::lshift, SDL_SCANCODE_LSHIFT},
		{key::lalt, SDL_SCANCODE_LALT},
		{key::lgui, SDL_SCANCODE_LGUI},
		{key::rctrl, SDL_SCANCODE_RCTRL},
		{key::rshift, SDL_SCANCODE_RSHIFT},
		{key::ralt, SDL_SCANCODE_RALT},
		{key::rgui, SDL_SCANCODE_RGUI},
		{key::mode, SDL_SCANCODE_MODE},
		{key::media_play, SDL_SCANCODE_MEDIA_PLAY},
		{key::media_pause, SDL_SCANCODE_MEDIA_PAUSE},
		{key::media_record, SDL_SCANCODE_MEDIA_RECORD},
		{key::media_fast_forward, SDL_SCANCODE_MEDIA_FAST_FORWARD},
		{key::media_rewind, SDL_SCANCODE_MEDIA_REWIND},
		{key::media_next, SDL_SCANCODE_MEDIA_NEXT_TRACK},
		{key::media_prev, SDL_SCANCODE_MEDIA_PREVIOUS_TRACK},
		{key::media_stop, SDL_SCANCODE_MEDIA_STOP},
		{key::media_eject, SDL_SCANCODE_MEDIA_EJECT},
		{key::media_play_pause, SDL_SCANCODE_MEDIA_PLAY_PAUSE},
		{key::media_select, SDL_SCANCODE_MEDIA_SELECT},
		{key::ac_new, SDL_SCANCODE_AC_NEW},
		{key::ac_open, SDL_SCANCODE_AC_OPEN},
		{key::ac_close, SDL_SCANCODE_AC_CLOSE},
		{key::ac_exit, SDL_SCANCODE_AC_EXIT},
		{key::ac_save, SDL_SCANCODE_AC_SAVE},
		{key::ac_print, SDL_SCANCODE_AC_PRINT},
		{key::ac_properties, SDL_SCANCODE_AC_PROPERTIES},
		{key::ac_search, SDL_SCANCODE_AC_SEARCH},
		{key::ac_home, SDL_SCANCODE_AC_HOME},
		{key::ac_back, SDL_SCANCODE_AC_BACK},
		{key::ac_forward, SDL_SCANCODE_AC_FORWARD},
		{key::ac_stop, SDL_SCANCODE_AC_STOP},
		{key::ac_refresh, SDL_SCANCODE_AC_REFRESH},
		{key::ac_bookmarks, SDL_SCANCODE_AC_BOOKMARKS},
		{key::sleep, SDL_SCANCODE_SLEEP},
	});
	return m;
}();
} // namespace

inline auto from_layout_independent_impl(SDL_Scancode code) -> key::code
{
	auto it = key_map.impl_scancode_to_os.find(code);
	if(it != std::end(key_map.impl_scancode_to_os))
	{
		return it->second;
	}

	return key::code::unknown;
}

inline auto to_layout_independent_impl(key::code code) -> SDL_Scancode
{
	auto it = key_map.os_to_impl_scancode.find(code);
	if(it != std::end(key_map.os_to_impl_scancode))
	{
		return it->second;
	}

	return SDL_SCANCODE_UNKNOWN;
}

namespace key
{
inline auto from_string(const std::string& str) noexcept -> os::key::code
{
	return from_layout_independent_impl(SDL_GetScancodeFromName(str.c_str()));
}

inline auto to_string(os::key::code key_code) noexcept -> std::string
{
	return SDL_GetScancodeName(to_layout_independent_impl(key_code));
}

inline auto is_pressed(os::key::code key_code) noexcept -> bool
{
	int num_keys{0};
	const auto* data = SDL_GetKeyboardState(&num_keys);
	auto impl_key = to_layout_independent_impl(key_code);
	if(impl_key < num_keys)
	{
		return data[impl_key] != 0;
	}
	return false;
}
} // namespace key

inline auto has_screen_keyboard() noexcept -> bool
{
	return SDL_HasScreenKeyboardSupport();
}

inline void start_text_input(const window& win) noexcept
{
	auto window = reinterpret_cast<window_impl*>(win.get_impl());
	SDL_StartTextInput(window->get_impl());
}

inline void stop_text_input(const window& win) noexcept
{
	auto window = reinterpret_cast<window_impl*>(win.get_impl());
	SDL_StopTextInput(window->get_impl());
}

inline auto is_text_input_active(const window& win) noexcept -> bool
{
	auto window = reinterpret_cast<window_impl*>(win.get_impl());
	return SDL_TextInputActive(window->get_impl());
}

inline void set_text_input_area(const window& win, const point& pos, const area& area, int cursor) noexcept
{
	auto window = reinterpret_cast<window_impl*>(win.get_impl());
	SDL_Rect rect;
	rect.x = pos.x;
	rect.y = pos.y;
	rect.w = area.w;
	rect.h = area.h;
	SDL_SetTextInputArea(window->get_impl(), &rect, cursor);
}


} // namespace sdl
} // namespace detail
} // namespace os
