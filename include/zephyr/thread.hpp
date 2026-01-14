// SPDX-License-Identifier: Apache-2.0
#ifndef __ZEPHYR_THREAD_HPP
#define __ZEPHYR_THREAD_HPP

#include "zephyr/tick_timer.hpp"
#include <system_error>
#include <zephyr/kernel.h>

namespace zephyr
{
class thread final : public ::k_thread
{
    thread(const thread&) = delete;
    thread& operator=(const thread&) = delete;

  public:
    static thread* current() { return reinterpret_cast<thread*>(::k_current_get()); }

    using priority = int;

    const char* get_name() const { return ::k_thread_name_get(const_cast<thread*>(this)); }
    auto set_name(const char* name) { return std::errc(::k_thread_name_set(this, name)); }

    priority get_priority() const { return ::k_thread_priority_get(const_cast<thread*>(this)); }
    auto set_priority(priority prio) { return ::k_thread_priority_set(this, prio); }

    template <class Rep, class Period>
    auto join(const std::chrono::duration<Rep, Period>& timeout = infinity)
    {
        return std::errc(::k_thread_join(this, to_timeout(timeout)));
    }

    void suspend() { ::k_thread_suspend(this); }
    void resume() { ::k_thread_resume(this); }

    void abort() { ::k_thread_abort(this); }

    template <size_t STACK_SIZE, class Rep = tick_timer::rep, class Period = tick_timer::period>
    thread(k_thread_stack_t (&stack)[STACK_SIZE], k_thread_entry_t entry, void* p1, void* p2,
           void* p3, int prio, uint32_t options = {},
           const std::chrono::duration<Rep, Period>& delay = tick_timer::duration{0})
    {
        ::k_thread_create(this, stack, STACK_SIZE - K_KERNEL_STACK_RESERVED, entry, p1, p2, p3,
                          prio, options, to_timeout(delay));
    }
    ~thread() = default;
};

namespace this_thread
{
inline void yield()
{
    ::k_yield();
}

inline bool can_yield()
{
    return ::k_can_yield();
}

inline bool preemptible()
{
    return ::k_is_preempt_thread();
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
