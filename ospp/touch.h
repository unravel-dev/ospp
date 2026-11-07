#pragma once

namespace os
{
namespace touch
{
/// Whether the device has a touch screen; a touchpad is none. A browser tells from the start; a desktop's SDL may know
/// of a screen only once a finger touches it.
auto has_screen() noexcept -> bool;
} // namespace touch
} // namespace os
