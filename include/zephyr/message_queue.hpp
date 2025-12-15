// SPDX-License-Identifier: Apache-2.0
#ifndef __ZEPHYR_MESSAGE_QUEUE_HPP
#define __ZEPHYR_MESSAGE_QUEUE_HPP

#include <optional>
#include <span>
#include "zephyr/tick_timer.hpp"

namespace zephyr
{
template <typename T>
struct message_queue : public ::k_msgq
{
    void post(const T& msg) { ::k_msgq_put(this, &msg, K_FOREVER); }
    bool try_post(const T& msg) { return ::k_msgq_put(this, &msg, K_NO_WAIT) == 0; }
    template <class Rep, class Period>
    inline bool try_post_for(const T& msg, const std::chrono::duration<Rep, Period>& rel_time)
    {
        return ::k_msgq_put(this, &msg, to_timeout(rel_time)) == 0;
    }
    template <class Clock, class Duration>
    inline bool try_post_until(const T& msg,
                               const std::chrono::time_point<Clock, Duration>& abs_time)
    {
        return try_post_for(duration_until(abs_time));
    }

    T get()
    {
        T msg;
        ::k_msgq_get(this, static_cast<void*>(&msg), K_FOREVER);
        return msg;
    }
    std::optional<T> try_get()
    {
        std::optional<T> msg{T()};
        if (::k_msgq_get(this, static_cast<void*>(&(*msg)), K_NO_WAIT) != 0)
        {
            msg.reset();
        }
        return msg;
    }
    template <class Rep, class Period>
    std::optional<T> try_get_for(const std::chrono::duration<Rep, Period>& rel_time)
    {
        std::optional<T> msg{T()};
        if (::k_msgq_get(this, static_cast<void*>(&(*msg)), to_timeout(rel_time)) != 0)
        {
            msg.reset();
        }
        return msg;
    }
    template <class Clock, class Duration>
    std::optional<T> try_get_until(const std::chrono::time_point<Clock, Duration>& abs_time)
    {
        return try_get_for(duration_until(abs_time));
    }

    void flush() { ::k_msgq_purge(this); }

    std::optional<T> peek() const
    {
        std::optional<T> msg{T()};
        if (::k_msgq_peek(const_cast<::k_msgq*>(this), static_cast<void*>(&(*msg))) != 0)
        {
            msg.reset();
        }
        return msg;
    }

    std::size_t size() const { return ::k_msgq_num_used_get(const_cast<::k_msgq*>(this)); }
    std::size_t free_space() const { return ::k_msgq_num_free_get(const_cast<::k_msgq*>(this)); }
    std::size_t max_size()
    {
        ::k_msgq_attrs attrs{};
        k_msgq_get_attrs(this, &attrs);
        return attrs.max_msgs;
    }
    bool empty() const { return ::k_msgq_num_used_get(const_cast<::k_msgq*>(this)) == 0; }
    bool full() const { return ::k_msgq_num_free_get(const_cast<::k_msgq*>(this)) == 0; }

  protected:
    message_queue(const std::span<char>& buffer)
    {
        ::k_msgq_init(this, buffer.data(), sizeof(T), buffer.size());
    }
};

template <typename T, std::size_t SIZE, std::size_t ALIGN = alignof(T)>
struct message_queue_instance final : public message_queue<T>
{
    message_queue_instance() : message_queue<T>(msgq_buffer_) {}

  private:
    char msgq_buffer_[SIZE * sizeof(T)] alignas(ALIGN);
};

} // namespace zephyr

#endif // __ZEPHYR_MESSAGE_QUEUE_HPP
