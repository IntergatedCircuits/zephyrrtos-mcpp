// SPDX-License-Identifier: Apache-2.0
#ifndef __ZEPHYR_WORK_QUEUE_HPP
#define __ZEPHYR_WORK_QUEUE_HPP

#include "zephyr/message_queue.hpp"
#include "zephyr/polling.hpp"
#include <etl/uncopyable.h>

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

} // namespace zephyr

#endif // __ZEPHYR_WORK_QUEUE_HPP
