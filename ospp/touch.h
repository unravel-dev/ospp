#pragma once

namespace os
{
namespace touch
{
/// Whether the device has a touch screen. A desktop's SDL knows of one only once a finger touches it; a browser
/// tells from the start.
auto has_screen() noexcept -> bool;
} // namespace touch
} // namespace os
