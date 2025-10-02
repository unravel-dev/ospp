#include "keyboard.h"
#include "window.hpp"

namespace os
{
    namespace key
    {
        os::key::code from_string(const std::string& str) noexcept
        {
            return {};
        }

        std::string to_string(os::key::code key_code) noexcept
        {
            return {};
        }

        bool is_pressed(os::key::code key_code) noexcept
        {
            return {};
        }
    }

    bool has_screen_keyboard() noexcept
    {
        return {};
    }

    void start_text_input(const window& win) noexcept
    {
    }

    void stop_text_input(const window& win) noexcept
    {
    }

    bool is_text_input_active(const window& win) noexcept
    {
        return {};
    }

    void set_text_input_area(const window& win, const point& pos, const area& area, int cursor) noexcept
    {
    }
}
