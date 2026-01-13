// SPDX-License-Identifier: Apache-2.0
#ifndef __ZEPHYR_EVENT_GROUP_HPP
#define __ZEPHYR_EVENT_GROUP_HPP

#include "zephyr/tick_timer.hpp"

namespace zephyr
{
#if 1
using events = uint32_t;
#else
class events
{
  public:
    using value_type = uint32_t;

    constexpr events() : value_(0) {}
    constexpr events(value_type value) : value_(value) {}
    operator value_type&() { return value_; }
    constexpr operator value_type() const { return value_; }
    constexpr events operator&(events other) const { return events(value_ & other.value_); }

    static constexpr events max() { return std::numeric_limits<value_type>::max(); }
    static constexpr events min() { return events(); }

    /// @brief  The value returned by blocking function calls when the wait time expired
    ///         without any relevant flags being set.
    static constexpr events timeout() { return events(); }

  private:
    value_type value_;
};
#endif

/// @brief  This class is a lightweight condition variable, allows threads to block
///         until a combination of flags has been set. The key difference to @ref condition_variable
///         is that here the waiting side chooses the wait strategy:
///          1. whether to wait for a combination of flags (all) or one of many (any)
///          2. whether to consume the flag when receiving it (default) or not (shared)
struct event_group final : public ::k_event
{
    event_group(const event_group&) = delete;
    event_group& operator=(const event_group&) = delete;

    explicit event_group() { ::k_event_init(this); }

    /// @brief  Sets the provided flags in the condition.
    /// @param  flags: the flags to activate
    /// @remark Thread and ISR context callable
    zephyr::events set(zephyr::events flags) { return ::k_event_post(this, flags); }

    /// @brief  Removes the provided flags from the condition.
    /// @param  flags: the flags to deactivate
    /// @remark Thread and ISR context callable
    zephyr::events clear(zephyr::events flags) { return ::k_event_clear(this, flags); }

    /// @brief  Modifies the provided flags in the condition according to the mask.
    /// @param  flags: the flags to activate/deactivate
    /// @param  mask: the mask determining which flags to modify
    /// @remark Thread and ISR context callable
    zephyr::events modify(zephyr::events flags, zephyr::events mask)
    {
        return ::k_event_set_masked(this, flags, mask);
    }

    /// @brief  Reads the current flags status.
    /// @return The currently active flags
    /// @remark Thread and ISR context callable
    zephyr::events get() const { return ::k_event_clear(const_cast<event_group*>(this), 0); }

    /// @brief  Blocks the current thread until any of the provided flags is raised.
    ///         When a flag unblocks the thread, it will be cleared.
    /// @param  flags: selection of flags to wait on
    /// @param  rel_time: duration to wait for the activation
    /// @return the raised flag(s) that caused the activation, or 0 if timed out
    /// @remark Thread context callable
    template <class Rep, class Period>
    zephyr::events wait_any_for(zephyr::events flags,
                                const std::chrono::duration<Rep, Period>& rel_time)
    {
        return ::k_event_wait_safe(this, flags, false, to_timeout(rel_time));
    }

    /// @brief  Blocks the current thread until any of the provided flags is raised.
    ///         When a flag unblocks the thread, it will be cleared.
    /// @param  flags: selection of flags to wait on
    /// @param  abs_time: deadline to wait for the activation
    /// @return the raised flag(s) that caused the activation, or 0 if timed out
    /// @remark Thread context callable
    template <class Clock, class Duration>
    zephyr::events wait_any_until(zephyr::events flags,
                                  const std::chrono::time_point<Clock, Duration>& abs_time)
    {
        return wait_any_for(flags, duration_until(abs_time));
    }

    zephyr::events wait_any(zephyr::events flags) { return wait_any_for(flags, infinity); }

    /// @brief  Blocks the current thread until all of the provided flags are raised.
    ///         When the thread is unblocked, the required flags will be cleared.
    /// @param  flags: combination of flags to wait on
    /// @param  rel_time: duration to wait for the activation
    /// @return the raised flag(s) that caused the activation, or 0 if timed out
    /// @remark Thread context callable
    template <class Rep, class Period>
    zephyr::events wait_all_for(zephyr::events flags,
                                const std::chrono::duration<Rep, Period>& rel_time)
    {
        return ::k_event_wait_all_safe(this, flags, false, to_timeout(rel_time));
    }

    /// @brief  Blocks the current thread until all of the provided flags are raised.
    ///         When the thread is unblocked, the required flags will be cleared.
    /// @param  flags: combination of flags to wait on
    /// @param  abs_time: deadline to wait for the activation
    /// @return the raised flag(s) that caused the activation, or 0 if timed out
    /// @remark Thread context callable
    template <class Clock, class Duration>
    zephyr::events wait_all_until(zephyr::events flags,
                                  const std::chrono::time_point<Clock, Duration>& abs_time)
    {
        return wait_all_for(flags, duration_until(abs_time));
    }

    zephyr::events wait_all(zephyr::events flags) { return wait_all_for(flags, infinity); }

    /// @brief  Blocks the current thread until any of the provided flags is raised.
    ///         Doesn't modify the flags upon activation.
    /// @param  flags: selection of flags to wait on
    /// @param  rel_time: duration to wait for the activation
    /// @return the raised flag(s) that caused the activation, or 0 if timed out
    /// @remark Thread context callable
    template <class Rep, class Period>
    zephyr::events shared_wait_any_for(zephyr::events flags,
                                       const std::chrono::duration<Rep, Period>& rel_time)
    {
        return ::k_event_wait(this, flags, false, to_timeout(rel_time));
    }

    /// @brief  Blocks the current thread until any of the provided flags is raised.
    ///         Doesn't modify the flags upon activation.
    /// @param  flags: selection of flags to wait on
    /// @param  abs_time: deadline to wait for the activation
    /// @return the raised flag(s) that caused the activation, or 0 if timed out
    /// @remark Thread context callable
    template <class Clock, class Duration>
    zephyr::events shared_wait_any_until(zephyr::events flags,
                                         const std::chrono::time_point<Clock, Duration>& abs_time)
    {
        return shared_wait_any_for(flags, duration_until(abs_time));
    }

    zephyr::events shared_wait_any(zephyr::events flags)
    {
        return shared_wait_any_for(flags, infinity);
    }

    /// @brief  Blocks the current thread until all of the provided flags are raised.
    ///         Doesn't modify the flags upon activation.
    /// @param  flags: combination of flags to wait on
    /// @param  rel_time: duration to wait for the activation
    /// @return the raised flag(s) that caused the activation, or 0 if timed out
    /// @remark Thread context callable
    template <class Rep, class Period>
    zephyr::events shared_wait_all_for(zephyr::events flags,
                                       const std::chrono::duration<Rep, Period>& rel_time)
    {
        return ::k_event_wait_all(this, flags, false, to_timeout(rel_time));
    }

    /// @brief  Blocks the current thread until all of the provided flags are raised.
    ///         Doesn't modify the flags upon activation.
    /// @param  flags: combination of flags to wait on
    /// @param  abs_time: deadline to wait for the activation
    /// @return the raised flag(s) that caused the activation, or 0 if timed out
    /// @remark Thread context callable
    template <class Clock, class Duration>
    zephyr::events shared_wait_all_until(zephyr::events flags,
                                         const std::chrono::time_point<Clock, Duration>& abs_time)
    {
        return shared_wait_all_for(flags, duration_until(abs_time));
    }

    zephyr::events shared_wait_all(zephyr::events flags)
    {
        return shared_wait_all_for(flags, infinity);
    }
};

} // namespace zephyr

#endif // __ZEPHYR_EVENT_GROUP_HPP