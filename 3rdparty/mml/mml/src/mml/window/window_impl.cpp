////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <mml/window/window_impl.hpp>
#include <mml/window/event.hpp>
#include <mml/window/joystick_manager.hpp>
#include <mml/window/sensor_manager.hpp>
#include <algorithm>
#include <cmath>
#include <thread>
#include <chrono>

#if defined(MML_SYSTEM_WINDOWS)

    #include <mml/window/win32/window_impl_win32.hpp>
    using window_impl_type = mml::priv::window_impl_win32;

#elif defined(MML_SYSTEM_LINUX) || defined(MML_SYSTEM_FREEBSD)

    #include <mml/window/unix/window_impl_x11.hpp>
    using window_impl_type = mml::priv::window_impl_x11;

#endif


namespace mml
{
namespace priv
{
////////////////////////////////////////////////////////////
window_impl* window_impl::create(video_mode mode, const std::array<std::int32_t, 2>& position,
                                 const std::string& title, std::uint32_t style)
{
    return new window_impl_type(mode, position, title, style);
}


////////////////////////////////////////////////////////////
window_impl* window_impl::create(window_handle handle)
{
    return new window_impl_type(handle);
}


////////////////////////////////////////////////////////////
window_impl::window_impl() :
joystick_threshold_(0.1f)
{
    // Get the initial joystick states
    joystick_manager::get_instance().update();
    for (unsigned int i = 0; i < joystick::count; ++i)
        joystick_states_[i] = joystick_manager::get_instance().get_state(i);

    // Get the initial sensor states
    for (unsigned int i = 0; i < sensor::count; ++i)
        sensor_value_[i] = std::array<float, 3>({{ 0, 0, 0 }});
}


////////////////////////////////////////////////////////////
window_impl::~window_impl() = default;


////////////////////////////////////////////////////////////
void window_impl::set_joystick_threshold(float threshold)
{
    joystick_threshold_ = threshold;
}


////////////////////////////////////////////////////////////
bool window_impl::pop_event(platform_event& event, bool block)
{
    // If the event queue is empty, let's first check if new events are available from the OS
    if (events_.empty())
    {
        // Get events from the system
        process_joystick_events();
        process_sensor_events();
        process_events();

        // In blocking mode, we must process events until one is triggered
        if (block)
        {
            // Here we use a manual wait loop instead of the optimized
            // wait-event provided by the OS, so that we don't skip joystick
            // events (which require polling)
            while (events_.empty())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                process_joystick_events();
                process_sensor_events();
                process_events();
            }
        }
    }

    // Pop the first event of the queue, if it is not empty
    if (!events_.empty())
    {
        event = events_.front();
        events_.pop();

        return true;
    }

    return false;
}


////////////////////////////////////////////////////////////
void window_impl::push_event(const platform_event& event)
{
    events_.push(event);
}


////////////////////////////////////////////////////////////
void window_impl::process_joystick_events()
{
    // First update the global joystick states
    joystick_manager::get_instance().update();

    for (unsigned int i = 0; i < joystick::count; ++i)
    {
        // Copy the previous state of the joystick and get the new one
        joystick_state previousState = joystick_states_[i];
        joystick_states_[i] = joystick_manager::get_instance().get_state(i);
        joystick_caps caps = joystick_manager::get_instance().get_capabilities(i);

        // Connection state
        bool connected = joystick_states_[i].connected;
        if (previousState.connected ^ connected)
        {
            platform_event event;
            event.type = connected ? platform_event::joystick_connected : platform_event::joystick_disconnected;
            event.joystick_button.joystick_id = i;
            push_event(event);
        }

        if (connected)
        {
            // Axes
            for (unsigned int j = 0; j < joystick::axis_count; ++j)
            {
                if (caps.axes[j])
                {
                    auto axis = static_cast<joystick::axis>(j);
                    float prevPos = previousState.axes[axis];
                    float currPos = joystick_states_[i].axes[axis];
                    if (fabs(currPos - prevPos) >= joystick_threshold_)
                    {
                        platform_event event;
                        event.type = platform_event::joystick_moved;
                        event.joystick_move.joystick_id = i;
                        event.joystick_move.axis = axis;
                        event.joystick_move.position = currPos;
                        push_event(event);
                    }
                }
            }

            // Buttons
            for (unsigned int j = 0; j < caps.button_count; ++j)
            {
                bool prevPressed = previousState.buttons[j];
                bool currPressed = joystick_states_[i].buttons[j];

                if (prevPressed ^ currPressed)
                {
                    platform_event event;
                    event.type = currPressed ? platform_event::joystick_button_pressed : platform_event::joystick_button_released;
                    event.joystick_button.joystick_id = i;
                    event.joystick_button.button = j;
                    push_event(event);
                }
            }
        }
    }
}


////////////////////////////////////////////////////////////
void window_impl::process_sensor_events()
{
    // First update the sensor states
    sensor_manager::get_instance().update();

    for (unsigned int i = 0; i < sensor::count; ++i)
    {
        auto sensor = static_cast<sensor::type>(i);

        // Only process enabled sensors
        if (sensor_manager::get_instance().is_enabled(sensor))
        {
            // Copy the previous value of the sensor and get the new one
            std::array<float, 3> previousValue = sensor_value_[i];
            sensor_value_[i] = sensor_manager::get_instance().get_value(sensor);

            // If the value has changed, trigger an event
            if (sensor_value_[i] != previousValue) // @todo use a threshold?
            {
                platform_event event;
                event.type = platform_event::sensor_changed;
                event.sensor_ev.type = sensor;
                event.sensor_ev.x = sensor_value_[i][0];
                event.sensor_ev.y = sensor_value_[i][1];
                event.sensor_ev.z = sensor_value_[i][2];
                push_event(event);
            }
        }
    }
}

} // namespace priv

} // namespace mml
