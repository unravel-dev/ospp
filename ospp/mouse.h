#pragma once

#include "types.hpp"

namespace os
{
class window;

namespace mouse
{
enum class button : uint8_t
{
	none,
	left,
	right,
	middle,
	x1,
	x2,
	button_count ///< Keep last -- the total number of mouse buttons
};

auto is_button_pressed(button b) noexcept -> bool;

auto get_position() noexcept -> point;
auto get_position(const window& relative_to) noexcept -> point;

void set_position(const point& pos) noexcept;
void set_position(const point& pos, const window& relative_to) noexcept;

void capture(bool enabled);
void disable(bool val);

/// Relative mode for \a relative_to: the cursor is hidden and held, and only its motion counts. In the browser it is
/// the page's pointer lock, which the browser grants close to a player's click or key; after Esc releases it, the
/// next click on the page takes it again.
void set_relative_mode(const window& relative_to, bool enabled) noexcept;
/// The mouse's motion since the last call, in window coordinates; it counts while the cursor is held too.
auto get_relative_motion() noexcept -> point;
} // namespace mouse
} // namespace os
