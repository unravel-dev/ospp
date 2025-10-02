#pragma once
#include "../../keyboard.h"

#include "window.hpp"

#include <map>

namespace os
{
namespace detail
{
namespace glfw
{

inline auto from_layout_independent_impl(int32_t code) -> key::code
{
	switch(code)
	{
		case GLFW_KEY_SPACE:
			return key::space;
		case GLFW_KEY_APOSTROPHE:
			return key::apostrophe;
		case GLFW_KEY_COMMA:
			return key::comma;
		case GLFW_KEY_MINUS:
			return key::minus;
		case GLFW_KEY_PERIOD:
			return key::period;
		case GLFW_KEY_SLASH:
			return key::slash;
		case GLFW_KEY_0:
			return key::digit0;
		case GLFW_KEY_1:
			return key::digit1;
		case GLFW_KEY_2:
			return key::digit2;
		case GLFW_KEY_3:
			return key::digit3;
		case GLFW_KEY_4:
			return key::digit4;
		case GLFW_KEY_5:
			return key::digit5;
		case GLFW_KEY_6:
			return key::digit6;
		case GLFW_KEY_7:
			return key::digit7;
		case GLFW_KEY_8:
			return key::digit8;
		case GLFW_KEY_9:
			return key::digit9;
		case GLFW_KEY_SEMICOLON:
			return key::semicolon;
		case GLFW_KEY_EQUAL:
			return key::equals;
		case GLFW_KEY_A:
			return key::a;
		case GLFW_KEY_B:
			return key::b;
		case GLFW_KEY_C:
			return key::c;
		case GLFW_KEY_D:
			return key::d;
		case GLFW_KEY_E:
			return key::e;
		case GLFW_KEY_F:
			return key::f;
		case GLFW_KEY_G:
			return key::g;
		case GLFW_KEY_H:
			return key::h;
		case GLFW_KEY_I:
			return key::i;
		case GLFW_KEY_J:
			return key::j;
		case GLFW_KEY_K:
			return key::k;
		case GLFW_KEY_L:
			return key::l;
		case GLFW_KEY_M:
			return key::m;
		case GLFW_KEY_N:
			return key::n;
		case GLFW_KEY_O:
			return key::o;
		case GLFW_KEY_P:
			return key::p;
		case GLFW_KEY_Q:
			return key::q;
		case GLFW_KEY_R:
			return key::r;
		case GLFW_KEY_S:
			return key::s;
		case GLFW_KEY_T:
			return key::t;
		case GLFW_KEY_U:
			return key::u;
		case GLFW_KEY_V:
			return key::v;
		case GLFW_KEY_W:
			return key::w;
		case GLFW_KEY_X:
			return key::x;
		case GLFW_KEY_Y:
			return key::y;
		case GLFW_KEY_Z:
			return key::z;
		case GLFW_KEY_LEFT_BRACKET:
			return key::leftbracket;
		case GLFW_KEY_BACKSLASH:
			return key::backslash;
		case GLFW_KEY_RIGHT_BRACKET:
			return key::rightbracket;
		case GLFW_KEY_GRAVE_ACCENT:
			return key::grave;
		case GLFW_KEY_ESCAPE:
			return key::escape;
		case GLFW_KEY_ENTER:
			return key::enter;
		case GLFW_KEY_TAB:
			return key::tab;
		case GLFW_KEY_BACKSPACE:
			return key::backspace;
		case GLFW_KEY_INSERT:
			return key::insert;
		case GLFW_KEY_DELETE:
			return key::del;
		case GLFW_KEY_RIGHT:
			return key::right;
		case GLFW_KEY_LEFT:
			return key::left;
		case GLFW_KEY_DOWN:
			return key::down;
		case GLFW_KEY_UP:
			return key::up;
		case GLFW_KEY_PAGE_UP:
			return key::pageup;
		case GLFW_KEY_PAGE_DOWN:
			return key::pagedown;
		case GLFW_KEY_HOME:
			return key::home;
		case GLFW_KEY_END:
			return key::end;
		case GLFW_KEY_CAPS_LOCK:
			return key::capslock;
		case GLFW_KEY_SCROLL_LOCK:
			return key::scrolllock;
		case GLFW_KEY_NUM_LOCK:
			return key::numlockclear;
		case GLFW_KEY_PRINT_SCREEN:
			return key::printscreen;
		case GLFW_KEY_PAUSE:
			return key::pause;
		case GLFW_KEY_F1:
			return key::f1;
		case GLFW_KEY_F2:
			return key::f2;
		case GLFW_KEY_F3:
			return key::f3;
		case GLFW_KEY_F4:
			return key::f4;
		case GLFW_KEY_F5:
			return key::f5;
		case GLFW_KEY_F6:
			return key::f6;
		case GLFW_KEY_F7:
			return key::f7;
		case GLFW_KEY_F8:
			return key::f8;
		case GLFW_KEY_F9:
			return key::f9;
		case GLFW_KEY_F10:
			return key::f10;
		case GLFW_KEY_F11:
			return key::f11;
		case GLFW_KEY_F12:
			return key::f12;
		case GLFW_KEY_F13:
			return key::f13;
		case GLFW_KEY_F14:
			return key::f14;
		case GLFW_KEY_F15:
			return key::f15;
		case GLFW_KEY_F16:
			return key::f16;
		case GLFW_KEY_F17:
			return key::f17;
		case GLFW_KEY_F18:
			return key::f18;
		case GLFW_KEY_F19:
			return key::f19;
		case GLFW_KEY_F20:
			return key::f20;
		case GLFW_KEY_F21:
			return key::f21;
		case GLFW_KEY_F22:
			return key::f22;
		case GLFW_KEY_F23:
			return key::f23;
		case GLFW_KEY_F24:
			return key::f24;
		case GLFW_KEY_KP_0:
			return key::kp_digit0;
		case GLFW_KEY_KP_1:
			return key::kp_digit1;
		case GLFW_KEY_KP_2:
			return key::kp_digit2;
		case GLFW_KEY_KP_3:
			return key::kp_digit3;
		case GLFW_KEY_KP_4:
			return key::kp_digit4;
		case GLFW_KEY_KP_5:
			return key::kp_digit5;
		case GLFW_KEY_KP_6:
			return key::kp_digit6;
		case GLFW_KEY_KP_7:
			return key::kp_digit7;
		case GLFW_KEY_KP_8:
			return key::kp_digit8;
		case GLFW_KEY_KP_9:
			return key::kp_digit9;
		case GLFW_KEY_KP_DECIMAL:
			return key::kp_decimal;
		case GLFW_KEY_KP_DIVIDE:
			return key::kp_divide;
		case GLFW_KEY_KP_MULTIPLY:
			return key::kp_multiply;
		case GLFW_KEY_KP_SUBTRACT:
			return key::kp_minus;
		case GLFW_KEY_KP_ADD:
			return key::kp_plus;
		case GLFW_KEY_KP_ENTER:
			return key::kp_enter;
		case GLFW_KEY_KP_EQUAL:
			return key::kp_equals;
		case GLFW_KEY_LEFT_SHIFT:
			return key::lshift;
		case GLFW_KEY_LEFT_CONTROL:
			return key::lctrl;
		case GLFW_KEY_LEFT_ALT:
			return key::lalt;
		case GLFW_KEY_LEFT_SUPER:
			return key::lgui;
		case GLFW_KEY_RIGHT_SHIFT:
			return key::rshift;
		case GLFW_KEY_RIGHT_CONTROL:
			return key::rctrl;
		case GLFW_KEY_RIGHT_ALT:
			return key::ralt;
		case GLFW_KEY_RIGHT_SUPER:
			return key::rgui;
		case GLFW_KEY_MENU:
			return key::menu;

		default:
			return key::unknown;
	}
}

inline auto to_layout_independent_impl(key::code code) -> int32_t
{
	switch(code)
	{
		case key::space:
			return GLFW_KEY_SPACE;
		case key::apostrophe:
			return GLFW_KEY_APOSTROPHE;
		case key::comma:
			return GLFW_KEY_COMMA;
		case key::minus:
			return GLFW_KEY_MINUS;
		case key::period:
			return GLFW_KEY_PERIOD;
		case key::slash:
			return GLFW_KEY_SLASH;
		case key::digit0:
			return GLFW_KEY_0;
		case key::digit1:
			return GLFW_KEY_1;
		case key::digit2:
			return GLFW_KEY_2;
		case key::digit3:
			return GLFW_KEY_3;
		case key::digit4:
			return GLFW_KEY_4;
		case key::digit5:
			return GLFW_KEY_5;
		case key::digit6:
			return GLFW_KEY_6;
		case key::digit7:
			return GLFW_KEY_7;
		case key::digit8:
			return GLFW_KEY_8;
		case key::digit9:
			return GLFW_KEY_9;
		case key::semicolon:
			return GLFW_KEY_SEMICOLON;
		case key::equals:
			return GLFW_KEY_EQUAL;
		case key::a:
			return GLFW_KEY_A;
		case key::b:
			return GLFW_KEY_B;
		case key::c:
			return GLFW_KEY_C;
		case key::d:
			return GLFW_KEY_D;
		case key::e:
			return GLFW_KEY_E;
		case key::f:
			return GLFW_KEY_F;
		case key::g:
			return GLFW_KEY_G;
		case key::h:
			return GLFW_KEY_H;
		case key::i:
			return GLFW_KEY_I;
		case key::j:
			return GLFW_KEY_J;
		case key::k:
			return GLFW_KEY_K;
		case key::l:
			return GLFW_KEY_L;
		case key::m:
			return GLFW_KEY_M;
		case key::n:
			return GLFW_KEY_N;
		case key::o:
			return GLFW_KEY_O;
		case key::p:
			return GLFW_KEY_P;
		case key::q:
			return GLFW_KEY_Q;
		case key::r:
			return GLFW_KEY_R;
		case key::s:
			return GLFW_KEY_S;
		case key::t:
			return GLFW_KEY_T;
		case key::u:
			return GLFW_KEY_U;
		case key::v:
			return GLFW_KEY_V;
		case key::w:
			return GLFW_KEY_W;
		case key::x:
			return GLFW_KEY_X;
		case key::y:
			return GLFW_KEY_Y;
		case key::z:
			return GLFW_KEY_Z;
		case key::leftbracket:
			return GLFW_KEY_LEFT_BRACKET;
		case key::backslash:
			return GLFW_KEY_BACKSLASH;
		case key::rightbracket:
			return GLFW_KEY_RIGHT_BRACKET;
		case key::grave:
			return GLFW_KEY_GRAVE_ACCENT;
		case key::escape:
			return GLFW_KEY_ESCAPE;
		case key::enter:
			return GLFW_KEY_ENTER;
		case key::tab:
			return GLFW_KEY_TAB;
		case key::backspace:
			return GLFW_KEY_BACKSPACE;
		case key::insert:
			return GLFW_KEY_INSERT;
		case key::del:
			return GLFW_KEY_DELETE;
		case key::right:
			return GLFW_KEY_RIGHT;
		case key::left:
			return GLFW_KEY_LEFT;
		case key::down:
			return GLFW_KEY_DOWN;
		case key::up:
			return GLFW_KEY_UP;
		case key::pageup:
			return GLFW_KEY_PAGE_UP;
		case key::pagedown:
			return GLFW_KEY_PAGE_DOWN;
		case key::home:
			return GLFW_KEY_HOME;
		case key::end:
			return GLFW_KEY_END;
		case key::capslock:
			return GLFW_KEY_CAPS_LOCK;
		case key::scrolllock:
			return GLFW_KEY_SCROLL_LOCK;
		case key::numlockclear:
			return GLFW_KEY_NUM_LOCK;
		case key::printscreen:
			return GLFW_KEY_PRINT_SCREEN;
		case key::pause:
			return GLFW_KEY_PAUSE;
		case key::f1:
			return GLFW_KEY_F1;
		case key::f2:
			return GLFW_KEY_F2;
		case key::f3:
			return GLFW_KEY_F3;
		case key::f4:
			return GLFW_KEY_F4;
		case key::f5:
			return GLFW_KEY_F5;
		case key::f6:
			return GLFW_KEY_F6;
		case key::f7:
			return GLFW_KEY_F7;
		case key::f8:
			return GLFW_KEY_F8;
		case key::f9:
			return GLFW_KEY_F9;
		case key::f10:
			return GLFW_KEY_F10;
		case key::f11:
			return GLFW_KEY_F11;
		case key::f12:
			return GLFW_KEY_F12;
		case key::f13:
			return GLFW_KEY_F13;
		case key::f14:
			return GLFW_KEY_F14;
		case key::f15:
			return GLFW_KEY_F15;
		case key::f16:
			return GLFW_KEY_F16;
		case key::f17:
			return GLFW_KEY_F17;
		case key::f18:
			return GLFW_KEY_F18;
		case key::f19:
			return GLFW_KEY_F19;
		case key::f20:
			return GLFW_KEY_F20;
		case key::f21:
			return GLFW_KEY_F21;
		case key::f22:
			return GLFW_KEY_F22;
		case key::f23:
			return GLFW_KEY_F23;
		case key::f24:
			return GLFW_KEY_F24;
		case key::kp_digit0:
			return GLFW_KEY_KP_0;
		case key::kp_digit1:
			return GLFW_KEY_KP_1;
		case key::kp_digit2:
			return GLFW_KEY_KP_2;
		case key::kp_digit3:
			return GLFW_KEY_KP_3;
		case key::kp_digit4:
			return GLFW_KEY_KP_4;
		case key::kp_digit5:
			return GLFW_KEY_KP_5;
		case key::kp_digit6:
			return GLFW_KEY_KP_6;
		case key::kp_digit7:
			return GLFW_KEY_KP_7;
		case key::kp_digit8:
			return GLFW_KEY_KP_8;
		case key::kp_digit9:
			return GLFW_KEY_KP_9;
		case key::kp_decimal:
			return GLFW_KEY_KP_DECIMAL;
		case key::kp_divide:
			return GLFW_KEY_KP_DIVIDE;
		case key::kp_multiply:
			return GLFW_KEY_KP_MULTIPLY;
		case key::kp_minus:
			return GLFW_KEY_KP_SUBTRACT;
		case key::kp_plus:
			return GLFW_KEY_KP_ADD;
		case key::kp_enter:
			return GLFW_KEY_KP_ENTER;
		case key::kp_equals:
			return GLFW_KEY_KP_EQUAL;
		case key::lshift:
			return GLFW_KEY_LEFT_SHIFT;
		case key::lctrl:
			return GLFW_KEY_LEFT_CONTROL;
		case key::lalt:
			return GLFW_KEY_LEFT_ALT;
		case key::lgui:
			return GLFW_KEY_LEFT_SUPER;
		case key::rshift:
			return GLFW_KEY_RIGHT_SHIFT;
		case key::rctrl:
			return GLFW_KEY_RIGHT_CONTROL;
		case key::ralt:
			return GLFW_KEY_RIGHT_ALT;
		case key::rgui:
			return GLFW_KEY_RIGHT_SUPER;
		case key::menu:
			return GLFW_KEY_MENU;

		default:
			return GLFW_KEY_UNKNOWN;
	}
}

namespace key
{

inline auto from_string(const std::string& str) noexcept -> os::key::code
{
	const static std::map<std::string, os::key::code> key_map = []() {
		std::map<std::string, os::key::code> result;

		using type = std::underlying_type_t<os::key::code>;
		for(type i = 0; i < os::key::code::count; ++i)
		{
			auto key_code = static_cast<os::key::code>(i);
			result.emplace(to_string(key_code), key_code);
		}

		return result;
	}();

	auto it = key_map.find(str);
	if(it != std::end(key_map))
	{
		return it->second;
	}

	return os::key::code::unknown;
}

inline auto to_string(os::key::code key_code) noexcept -> std::string
{
	auto key_impl = to_layout_independent_impl(key_code);
	auto scan_impl = glfwGetKeyScancode(key_impl);
	auto name = glfwGetKeyName(key_impl, scan_impl);
	if(name != nullptr)
	{
		return name;
	}

	return {};
}

inline auto is_pressed(os::key::code key_code) noexcept -> bool
{
	auto impl_key = to_layout_independent_impl(key_code);

	auto focused_win = os::detail::glfw::get_focused_win();
	if(focused_win)
	{
		auto state = glfwGetKey(focused_win->get_impl(), impl_key);
		return state != GLFW_RELEASE;
	}
	return false;
}
}

inline auto has_screen_keyboard() noexcept -> bool
{
	return false;
}

inline void start_text_input(const window& win) noexcept
{
}

inline void stop_text_input(const window& win) noexcept
{
}

inline auto is_text_input_active(const window& win) noexcept -> bool
{
	return false;
}

inline void set_text_input_area(const window& win, const point& pos, const area& area, int cursor) noexcept
{
}
}
}
}
