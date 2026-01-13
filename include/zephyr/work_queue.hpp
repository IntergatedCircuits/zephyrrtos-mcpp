// SPDX-License-Identifier: Apache-2.0
#ifndef __ZEPHYR_WORK_QUEUE_HPP
#define __ZEPHYR_WORK_QUEUE_HPP

#include "zephyr/message_queue.hpp"
#include "zephyr/polling.hpp"

namespace zephyr
{

struct work final : public ::k_work
{
    explicit work(void (*fn)(work*))
    {
        ::k_work_init(this, reinterpret_cast<k_work_handler_t>(fn));
    }
    auto submit() { return ::k_work_submit(this); }
    auto cancel() { return ::k_work_cancel(this); }
    auto is_pending() const { return ::k_work_is_pending(this); }
};

struct work_poll final : public ::k_work_poll
{
    explicit work_poll(void (*fn)(work_poll*))
    {
        ::k_work_poll_init(this, reinterpret_cast<k_work_handler_t>(fn));
    }

    auto submit(const std::span<poll_event>& events, tick_timer::duration timeout)
    {
        return ::k_work_poll_submit(this, events.data(), static_cast<int>(events.size()),
                                    to_timeout(timeout));
    }

    auto cancel() { return ::k_work_poll_cancel(this); }
};

struct work_delayable final : public ::k_work_delayable
{
    explicit work_delayable(void (*fn)(work_delayable*))
    {
        ::k_work_init_delayable(this, reinterpret_cast<k_work_handler_t>(fn));
    }

    auto is_pending() const { return ::k_work_delayable_is_pending(this); }

    auto expiration() const
    {
        return tick_timer::time_point(tick_timer::duration(::k_work_delayable_expires_get(this)));
    }
    auto remaining_time() const
    {
        return tick_timer::duration(::k_work_delayable_remaining_get(this));
    }
    template <class Rep, class Period>
    int schedule(const std::chrono::duration<Rep, Period>& rel_time)
    {
        return ::k_work_schedule(this, to_timeout(rel_time));
    }
    template <class Rep, class Period>
    int reschedule(const std::chrono::duration<Rep, Period>& rel_time)
    {
        return ::k_work_schedule(this, to_timeout(rel_time));
    }

    auto cancel() { return ::k_work_cancel_delayable(this); }
};

} // namespace zephyr

#endif // __ZEPHYR_WORK_QUEUE_HPP
