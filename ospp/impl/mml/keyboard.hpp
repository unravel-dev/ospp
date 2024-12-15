#pragma once
#include "../../keyboard.h"
#include <mml/window/window.hpp>

#include <algorithm>
#include <cassert>
#include <map>
namespace os
{
namespace detail
{
namespace mml
{
namespace
{
struct mappings
{
	void to_lower(std::string& data)
	{
		std::transform(data.begin(), data.end(), data.begin(),
					   [](unsigned char c) { return std::tolower(c); });
	}

	void init_from(const std::map<key::code, ::mml::keyboard::scancode>& map)
	{
		os_to_impl_scancode = map;

		for(const auto& kvp : map)
		{
			auto it = impl_scancode_to_os.find(kvp.second);
			if(it != std::end(impl_scancode_to_os))
			{
				assert(false && "scancode already added");
			}
			impl_scancode_to_os[kvp.second] = kvp.first;

			auto name = ::mml::keyboard::get_description(kvp.second);
			to_lower(name);
			string_to_key_code[name] = kvp.first;
		}
	}

	std::map<std::string, key::code> string_to_key_code;
	std::map<::mml::keyboard::scancode, key::code> impl_scancode_to_os;
	std::map<key::code, ::mml::keyboard::scancode> os_to_impl_scancode;
};

mappings key_map = []()
{
	mappings m;
	m.init_from({
		{key::a, ::mml::keyboard::scan::A},
		{key::b, ::mml::keyboard::scan::B},
		{key::c, ::mml::keyboard::scan::C},
		{key::d, ::mml::keyboard::scan::D},
		{key::e, ::mml::keyboard::scan::E},
		{key::f, ::mml::keyboard::scan::F},
		{key::g, ::mml::keyboard::scan::G},
		{key::h, ::mml::keyboard::scan::H},
		{key::i, ::mml::keyboard::scan::I},
		{key::j, ::mml::keyboard::scan::J},
		{key::k, ::mml::keyboard::scan::K},
		{key::l, ::mml::keyboard::scan::L},
		{key::m, ::mml::keyboard::scan::M},
		{key::n, ::mml::keyboard::scan::N},
		{key::o, ::mml::keyboard::scan::O},
		{key::p, ::mml::keyboard::scan::P},
		{key::q, ::mml::keyboard::scan::Q},
		{key::r, ::mml::keyboard::scan::R},
		{key::s, ::mml::keyboard::scan::S},
		{key::t, ::mml::keyboard::scan::T},
		{key::u, ::mml::keyboard::scan::U},
		{key::v, ::mml::keyboard::scan::V},
		{key::w, ::mml::keyboard::scan::W},
		{key::x, ::mml::keyboard::scan::X},
		{key::y, ::mml::keyboard::scan::Y},
		{key::z, ::mml::keyboard::scan::Z},
		{key::digit1, ::mml::keyboard::scan::Num1},
		{key::digit2, ::mml::keyboard::scan::Num2},
		{key::digit3, ::mml::keyboard::scan::Num3},
		{key::digit4, ::mml::keyboard::scan::Num4},
		{key::digit5, ::mml::keyboard::scan::Num5},
		{key::digit6, ::mml::keyboard::scan::Num6},
		{key::digit7, ::mml::keyboard::scan::Num7},
		{key::digit8, ::mml::keyboard::scan::Num8},
		{key::digit9, ::mml::keyboard::scan::Num9},
		{key::digit0, ::mml::keyboard::scan::Num0},
		{key::enter, ::mml::keyboard::scan::Enter},
		{key::escape, ::mml::keyboard::scan::Escape},
		{key::backspace, ::mml::keyboard::scan::Backspace},
		{key::tab, ::mml::keyboard::scan::Tab},
		{key::space, ::mml::keyboard::scan::Space},
		{key::minus, ::mml::keyboard::scan::Hyphen},
		{key::equals, ::mml::keyboard::scan::Equal},
		{key::leftbracket, ::mml::keyboard::scan::LBracket},
		{key::rightbracket, ::mml::keyboard::scan::RBracket},
		{key::backslash, ::mml::keyboard::scan::Backslash},
		//{key::nonushash, ::mml::keyboard::scan::},
		{key::semicolon, ::mml::keyboard::scan::Semicolon},
		{key::apostrophe, ::mml::keyboard::scan::Apostrophe},
		{key::grave, ::mml::keyboard::scan::Grave},
		{key::comma, ::mml::keyboard::scan::Comma},
		{key::period, ::mml::keyboard::scan::Period},
		{key::slash, ::mml::keyboard::scan::Slash},
		{key::capslock, ::mml::keyboard::scan::CapsLock},
		{key::f1, ::mml::keyboard::scan::F1},
		{key::f2, ::mml::keyboard::scan::F2},
		{key::f3, ::mml::keyboard::scan::F3},
		{key::f4, ::mml::keyboard::scan::F4},
		{key::f5, ::mml::keyboard::scan::F5},
		{key::f6, ::mml::keyboard::scan::F6},
		{key::f7, ::mml::keyboard::scan::F7},
		{key::f8, ::mml::keyboard::scan::F8},
		{key::f9, ::mml::keyboard::scan::F9},
		{key::f10, ::mml::keyboard::scan::F10},
		{key::f11, ::mml::keyboard::scan::F11},
		{key::f12, ::mml::keyboard::scan::F12},
		{key::printscreen, ::mml::keyboard::scan::PrintScreen},
		{key::scrolllock, ::mml::keyboard::scan::ScrollLock},
		{key::pause, ::mml::keyboard::scan::Pause},
		{key::insert, ::mml::keyboard::scan::Insert},
		{key::home, ::mml::keyboard::scan::Home},
		{key::pageup, ::mml::keyboard::scan::PageUp},
		{key::del, ::mml::keyboard::scan::Delete},
		{key::end, ::mml::keyboard::scan::End},
		{key::pagedown, ::mml::keyboard::scan::PageDown},
		{key::right, ::mml::keyboard::scan::Right},
		{key::left, ::mml::keyboard::scan::Left},
		{key::down, ::mml::keyboard::scan::Down},
		{key::up, ::mml::keyboard::scan::Up},
		{key::numlockclear, ::mml::keyboard::scan::NumLock},
		{key::kp_divide, ::mml::keyboard::scan::NumpadDivide},
		{key::kp_multiply, ::mml::keyboard::scan::NumpadMultiply},
		{key::kp_minus, ::mml::keyboard::scan::NumpadMinus},
		{key::kp_plus, ::mml::keyboard::scan::NumpadPlus},
		{key::kp_enter, ::mml::keyboard::scan::NumpadEnter},
		{key::kp_digit1, ::mml::keyboard::scan::Numpad0},
		{key::kp_digit2, ::mml::keyboard::scan::Numpad1},
		{key::kp_digit3, ::mml::keyboard::scan::Numpad2},
		{key::kp_digit4, ::mml::keyboard::scan::Numpad3},
		{key::kp_digit5, ::mml::keyboard::scan::Numpad4},
		{key::kp_digit6, ::mml::keyboard::scan::Numpad5},
		{key::kp_digit7, ::mml::keyboard::scan::Numpad6},
		{key::kp_digit8, ::mml::keyboard::scan::Numpad7},
		{key::kp_digit9, ::mml::keyboard::scan::Numpad8},
		{key::kp_digit0, ::mml::keyboard::scan::Numpad9},
		{key::kp_period, ::mml::keyboard::scan::NumpadDecimal},
		{key::nonusbackslash, ::mml::keyboard::scan::NonUsBackslash},
		{key::application, ::mml::keyboard::scan::Application},
		//{key::power, ::mml::keyboard::scan::},
		{key::kp_equals, ::mml::keyboard::scan::NumpadEqual},
		{key::f13, ::mml::keyboard::scan::F13},
		{key::f14, ::mml::keyboard::scan::F14},
		{key::f15, ::mml::keyboard::scan::F15},
		{key::f16, ::mml::keyboard::scan::F16},
		{key::f17, ::mml::keyboard::scan::F17},
		{key::f18, ::mml::keyboard::scan::F18},
		{key::f19, ::mml::keyboard::scan::F19},
		{key::f20, ::mml::keyboard::scan::F20},
		{key::f21, ::mml::keyboard::scan::F21},
		{key::f22, ::mml::keyboard::scan::F22},
		{key::f23, ::mml::keyboard::scan::F23},
		{key::f24, ::mml::keyboard::scan::F24},
		{key::execute, ::mml::keyboard::scan::Execute},
		{key::help, ::mml::keyboard::scan::Help},
		{key::menu, ::mml::keyboard::scan::Menu},
		{key::select, ::mml::keyboard::scan::Select},
		{key::stop, ::mml::keyboard::scan::Stop},
		//{key::again, ::mml::keyboard::scan::},
		{key::undo, ::mml::keyboard::scan::Undo},
		{key::cut, ::mml::keyboard::scan::Cut},
		{key::copy, ::mml::keyboard::scan::Copy},
		{key::paste, ::mml::keyboard::scan::Paste},
		{key::find, ::mml::keyboard::scan::Search},
		{key::mute, ::mml::keyboard::scan::VolumeMute},
		{key::volumeup, ::mml::keyboard::scan::VolumeUp},
		{key::volumedown, ::mml::keyboard::scan::VolumeDown},
		//{key::kp_comma, ::mml::keyboard::scan::},
		//{key::kp_equalsas400, ::mml::keyboard::scan::},
		//{key::international1, ::mml::keyboard::scan::},
		//{key::international2, ::mml::keyboard::scan::},
		//{key::international3, ::mml::keyboard::scan::},
		//{key::international4, ::mml::keyboard::scan::},
		//{key::international5, ::mml::keyboard::scan::},
		//{key::international6, ::mml::keyboard::scan::},
		//{key::international7, ::mml::keyboard::scan::},
		//{key::international8, ::mml::keyboard::scan::},
		//{key::international9, ::mml::keyboard::scan::},
		//{key::lang1, ::mml::keyboard::scan::},
		//{key::lang2, ::mml::keyboard::scan::},
		//{key::lang3, ::mml::keyboard::scan::},
		//{key::lang4, ::mml::keyboard::scan::},
		//{key::lang5, ::mml::keyboard::scan::},
		//{key::lang6, ::mml::keyboard::scan::},
		//{key::lang7, ::mml::keyboard::scan::},
		//{key::lang8, ::mml::keyboard::scan::},
		//{key::lang9, ::mml::keyboard::scan::},
		//{key::alterase, ::mml::keyboard::scan::},
		//{key::sysreq, ::mml::keyboard::scan::},
		//{key::cancel, ::mml::keyboard::scan::},
		//{key::clear, ::mml::keyboard::scan::},
		//{key::prior, ::mml::keyboard::scan::},
		//{key::return2, ::mml::keyboard::scan::},
		//{key::separator, ::mml::keyboard::scan::},
		//{key::out, ::mml::keyboard::scan::},
		//{key::oper, ::mml::keyboard::scan::},
		//{key::clearagain, ::mml::keyboard::scan::},
		//{key::crsel, ::mml::keyboard::scan::},
		//{key::exsel, ::mml::keyboard::scan::},
		//{key::kp_digit00, ::mml::keyboard::scan::},
		//{key::kp_digit000, ::mml::keyboard::scan::},
		//{key::thousandsseparator, ::mml::keyboard::scan::},
		//{key::decimalseparator, ::mml::keyboard::scan::},
		//{key::currencyunit, ::mml::keyboard::scan::},
		//{key::currencysubunit, ::mml::keyboard::scan::},
		//{key::kp_leftparen, ::mml::keyboard::scan::},
		//{key::kp_rightparen, ::mml::keyboard::scan::},
		//{key::kp_leftbrace, ::mml::keyboard::scan::},
		//{key::kp_rightbrace, ::mml::keyboard::scan::},
		//{key::kp_tab, ::mml::keyboard::scan::},
		//{key::kp_backspace, ::mml::keyboard::scan::},
		//{key::kp_a, ::mml::keyboard::scan::},
		//{key::kp_b, ::mml::keyboard::scan::},
		//{key::kp_c, ::mml::keyboard::scan::},
		//{key::kp_d, ::mml::keyboard::scan::},
		//{key::kp_e, ::mml::keyboard::scan::},
		//{key::kp_f, ::mml::keyboard::scan::},
		//{key::kp_xor, ::mml::keyboard::scan::},
		//{key::kp_power, ::mml::keyboard::scan::},
		//{key::kp_percent, ::mml::keyboard::scan::},
		//{key::kp_less, ::mml::keyboard::scan::},
		//{key::kp_greater, ::mml::keyboard::scan::},
		//{key::kp_ampersand, ::mml::keyboard::scan::},
		//{key::kp_dblampersand, ::mml::keyboard::scan::},
		//{key::kp_verticalbar, ::mml::keyboard::scan::},
		//{key::kp_dblverticalbar, ::mml::keyboard::scan::},
		//{key::kp_colon, ::mml::keyboard::scan::},
		//{key::kp_hash, ::mml::keyboard::scan::},
		//{key::kp_space, ::mml::keyboard::scan::},
		//{key::kp_at, ::mml::keyboard::scan::},
		//{key::kp_exclam, ::mml::keyboard::scan::},
		//{key::kp_memstore, ::mml::keyboard::scan::},
		//{key::kp_memrecall, ::mml::keyboard::scan::},
		//{key::kp_memclear, ::mml::keyboard::scan::},
		//{key::kp_memadd, ::mml::keyboard::scan::},
		//{key::kp_memsubtract, ::mml::keyboard::scan::},
		//{key::kp_memmultiply, ::mml::keyboard::scan::},
		//{key::kp_memdivide, ::mml::keyboard::scan::},
		//{key::kp_plusminus, ::mml::keyboard::scan::},
		//{key::kp_clear, ::mml::keyboard::scan::},
		//{key::kp_clearentry, ::mml::keyboard::scan::},
		//{key::kp_binary, ::mml::keyboard::scan::},
		//{key::kp_octal, ::mml::keyboard::scan::},
		//{key::kp_decimal, ::mml::keyboard::scan::},
		//{key::kp_hexadecimal, ::mml::keyboard::scan::},
		{key::lctrl, ::mml::keyboard::scan::LControl},
		{key::lshift, ::mml::keyboard::scan::LShift},
		{key::lalt, ::mml::keyboard::scan::LAlt},
		{key::lgui, ::mml::keyboard::scan::LSystem},
		{key::rctrl, ::mml::keyboard::scan::RControl},
		{key::rshift, ::mml::keyboard::scan::RShift},
		{key::ralt, ::mml::keyboard::scan::RAlt},
		{key::rgui, ::mml::keyboard::scan::RSystem},
		{key::mode, ::mml::keyboard::scan::ModeChange},
		{key::media_next, ::mml::keyboard::scan::MediaNextTrack},
		{key::media_prev, ::mml::keyboard::scan::MediaPreviousTrack},
		{key::media_stop, ::mml::keyboard::scan::MediaStop},
		{key::media_play, ::mml::keyboard::scan::MediaPlayPause},
		{key::media_select, ::mml::keyboard::scan::LaunchMediaSelect},
		// {key::ac_search, ::mml::keyboard::scan::Search},
		{key::ac_home, ::mml::keyboard::scan::HomePage},
		{key::ac_back, ::mml::keyboard::scan::Back},
		{key::ac_forward, ::mml::keyboard::scan::Forward},
		//{key::ac_stop, ::mml::keyboard::scan::Stop},
		{key::ac_refresh, ::mml::keyboard::scan::Refresh},
		{key::ac_bookmarks, ::mml::keyboard::scan::Favorites},
		//{key::brightnessdown, ::mml::keyboard::scan::},
		//{key::brightnessup, ::mml::keyboard::scan::},
		//{key::displayswitch, ::mml::keyboard::scan::},
		//{key::kbdillumtoggle, ::mml::keyboard::scan::},
		//{key::kbdillumdown, ::mml::keyboard::scan::},
		//{key::kbdillumup, ::mml::keyboard::scan::},
		//{key::eject, ::mml::keyboard::scan::},
		//{key::sleep, ::mml::keyboard::scan::},
		//{key::media_rewind, ::mml::keyboard::scan::},
		//{key::media_fast_forward, ::mml::keyboard::scan::},
	});
	return m;
}();

inline auto from_layout_independent_impl(::mml::keyboard::scancode code) -> key::code
{
	auto it = key_map.impl_scancode_to_os.find(code);
	if(it != std::end(key_map.impl_scancode_to_os))
	{
		return it->second;
	}

	return key::code::unknown;
}

inline auto to_layout_independent_impl(key::code code) -> ::mml::keyboard::scan
{
	auto it = key_map.os_to_impl_scancode.find(code);
	if(it != std::end(key_map.os_to_impl_scancode))
	{
		return it->second;
	}

	return ::mml::keyboard::scan::Unknown;
}

} // namespace

namespace key
{
inline auto from_string(const std::string& key) noexcept -> os::key::code
{
	auto lower_key = key;
	key_map.to_lower(lower_key);
	auto it = key_map.string_to_key_code.find(lower_key);
	if(it != std::end(key_map.string_to_key_code))
	{
		return it->second;
	}
	return os::key::code::unknown;
}

inline auto to_string(os::key::code c) noexcept -> std::string
{
	return ::mml::keyboard::get_description(to_layout_independent_impl(c));
}

inline auto is_pressed(os::key::code key_code) noexcept -> bool
{
	return ::mml::keyboard::is_key_pressed(to_layout_independent_impl(key_code));
}
} // namespace key

inline auto has_screen_keyboard() noexcept -> bool
{
	return false;
}

inline void start_text_input() noexcept
{
	::mml::keyboard::set_virtual_keyboard_visible(true);
}

inline void stop_text_input() noexcept
{
	::mml::keyboard::set_virtual_keyboard_visible(false);
}

inline auto is_text_input_active() noexcept -> bool
{
	return false;
}
} // namespace mml
} // namespace detail
} // namespace os
