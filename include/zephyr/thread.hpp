// SPDX-License-Identifier: Apache-2.0
#ifndef __ZEPHYR_THREAD_HPP
#define __ZEPHYR_THREAD_HPP

#include "zephyr/tick_timer.hpp"

namespace zephyr
{
namespace this_thread
{
inline void yield()
{
    ::k_yield();
}

inline void sleep_for(const tick_timer::duration& rel_time)
{
    ::k_sleep(to_timeout(rel_time));
    // note the alternative API: k_usleep()
}

template <class Rep, class Period>
inline void sleep_for(const std::chrono::duration<Rep, Period>& rel_time)
{
    // workaround to prevent this function calling itself
    const auto ticks_sleep_for = static_cast<void (*)(const tick_timer::duration&)>(&sleep_for);
    ticks_sleep_for(std::chrono::ceil<tick_timer::duration>(rel_time));
}

template <class Clock, class Duration>
inline void sleep_until(const std::chrono::time_point<Clock, Duration>& abs_time)
{
    sleep_for(duration_until(abs_time));
}

} // namespace this_thread
} // namespace zephyr

#endif // __ZEPHYR_THREAD_HPP
