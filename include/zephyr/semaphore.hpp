// SPDX-License-Identifier: Apache-2.0
#ifndef __ZEPHYR_SEMAPHORE_HPP
#define __ZEPHYR_SEMAPHORE_HPP

#include "zephyr/tick_timer.hpp"

namespace zephyr
{
template <std::ptrdiff_t COUNT>
struct counting_semaphore final : public ::k_sem
{
    explicit counting_semaphore(std::ptrdiff_t desired)
    {
        __ASSERT_NO_MSG((desired >= 0) and (desired <= COUNT));
        ::k_sem_init(this, desired, COUNT);
    }
    void release(std::ptrdiff_t update = 1)
    {
        __ASSERT_NO_MSG(update >= 0);
        for (; update > 0; --update)
        {
            ::k_sem_give(this);
        }
    }
    void acquire() { ::k_sem_take(this, K_FOREVER); }
    bool try_acquire() { return ::k_sem_take(this, K_NO_WAIT) == 0; }

    template <class Rep, class Period>
    inline bool try_acquire_for(const std::chrono::duration<Rep, Period>& rel_time)
    {
        return ::k_sem_take(this, to_timeout(rel_time)) == 0;
    }

    template <class Clock, class Duration>
    inline bool try_acquire_until(const std::chrono::time_point<Clock, Duration>& abs_time)
    {
        return try_acquire_for(duration_until(abs_time));
    }
};

using binary_semaphore = counting_semaphore<1>;

} // namespace zephyr

#endif // __ZEPHYR_SEMAPHORE_HPP
