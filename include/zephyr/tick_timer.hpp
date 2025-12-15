// SPDX-License-Identifier: Apache-2.0
#ifndef __ZEPHYR_TICK_TIMER_HPP
#define __ZEPHYR_TICK_TIMER_HPP

#include <chrono>
#include <zephyr/kernel.h>

namespace zephyr
{
struct tick_timer
{
    using rep = ::k_ticks_t;
    using period = std::ratio<1, Z_HZ_ticks>;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<tick_timer>;
    static constexpr bool is_steady = true;

    static time_point now() { return time_point{duration{::k_uptime_ticks()}}; }
};

template <class Rep, class Period>
inline constexpr tick_timer::rep to_ticks(const std::chrono::duration<Rep, Period>& rel_time)
{
    return std::chrono::duration_cast<tick_timer::duration>(rel_time).count();
}

inline constexpr tick_timer::rep to_ticks(const tick_timer::time_point& time)
{
    return to_ticks(time.time_since_epoch());
}

template <class Rep, class Period>
inline constexpr k_timeout_t to_timeout(const std::chrono::duration<Rep, Period>& rel_time)
{
    return Z_TIMEOUT_TICKS(to_ticks(rel_time));
}

inline constexpr k_timeout_t to_timeout(const tick_timer::time_point& time)
{
    return K_TIMEOUT_ABS_TICKS(to_ticks(time.time_since_epoch()));
}

template <class Clock, class Duration>
inline constexpr auto duration_until(const std::chrono::time_point<Clock, Duration>& abs_time)
{
    auto d = abs_time - Clock::now();
    return d < Duration::zero() ? Duration::zero() : d;
}

inline constexpr tick_timer::duration infinity{K_TICKS_FOREVER};

} // namespace zephyr

#endif // __ZEPHYR_TICK_TIMER_HPP
