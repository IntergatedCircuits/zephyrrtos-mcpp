// SPDX-License-Identifier: Apache-2.0
#ifndef __ZEPHYR_POLLING_HPP
#define __ZEPHYR_POLLING_HPP

#include "zephyr/message_queue.hpp"
#include "zephyr/semaphore.hpp"

namespace zephyr
{
struct signal final : public ::k_poll_signal
{
    signal() { ::k_poll_signal_init(this); }

    void raise(int result) { ::k_poll_signal_raise(this, result); }
    void reset() { ::k_poll_signal_reset(this); }

    std::optional<int> check()
    {
        unsigned int signaled;
        std::optional<int> result{0};
        ::k_poll_signal_check(this, &signaled, &(*result));
        if (signaled == 0U)
        {
            result.reset();
        }
        return result;
    }
};

struct poll_event final : public ::k_poll_event
{
    explicit poll_event(::k_poll_signal& s)
    {
        ::k_poll_event_init(this, K_POLL_TYPE_SIGNAL, K_POLL_MODE_NOTIFY_ONLY, &s);
    }
    explicit poll_event(::k_msgq& msgq)
    {
        ::k_poll_event_init(this, K_POLL_TYPE_MSGQ_DATA_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY, &msgq);
    }
    explicit poll_event(::k_sem& sem)
    {
        ::k_poll_event_init(this, K_POLL_TYPE_SEM_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY, &sem);
    }

    void reset_state() { this->state = K_POLL_STATE_NOT_READY; }
};

template <std::size_t N>
inline auto poll_for(std::array<::k_poll_event, N>& events, tick_timer::duration timeout)
{
    return ::k_poll(events.data(), events.size(), to_timeout(timeout));
}

} // namespace zephyr

#endif // __ZEPHYR_POLLING_HPP
