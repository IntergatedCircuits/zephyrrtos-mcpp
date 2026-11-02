// SPDX-License-Identifier: Apache-2.0
#include "zephyr/cpu.hpp"
#include "zephyr/event_group.hpp"
#include "zephyr/message_queue.hpp"
#include "zephyr/polling.hpp"
#include "zephyr/semaphore.hpp"
#include "zephyr/thread.hpp"
#include "zephyr/tick_timer.hpp"
#include "zephyr/work_queue.hpp"

#include <array>
#include <chrono>
#include <tuple>

// The templates of the library are only compiled when they are used, so this function uses all of
// them (with durations and time points of a different period than the tick timer's).
// It is never called, it only needs to compile.
namespace
{
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
K_THREAD_STACK_DEFINE(test_stack, 512);

void thread_entry(void* /*p1*/, void* /*p2*/, void* /*p3*/) {}
void work_handler(zephyr::work* /*work*/) {}
void poll_work_handler(zephyr::work_poll* /*work*/) {}
void delayable_work_handler(zephyr::work_delayable* /*work*/) {}

[[maybe_unused]] void instantiate_templates()
{
    const auto rel_time = std::chrono::milliseconds{10};
    const auto abs_time = zephyr::tick_timer::now() + rel_time;

    std::ignore = zephyr::to_ticks(rel_time);
    std::ignore = zephyr::to_ticks(zephyr::tick_timer::now());
    std::ignore = zephyr::to_timeout(rel_time);
    std::ignore = zephyr::to_timeout(zephyr::tick_timer::now());
    std::ignore = zephyr::duration_until(abs_time);

    {
        zephyr::spinlock lock{};
        const zephyr::critical_section section{lock};
        std::ignore = zephyr::this_cpu::is_in_isr();
    }

    zephyr::counting_semaphore<4> semaphore{0};
    semaphore.release();
    semaphore.acquire();
    std::ignore = semaphore.try_acquire();
    std::ignore = semaphore.try_acquire_for(rel_time);
    std::ignore = semaphore.try_acquire_until(abs_time);

    zephyr::event_group event_group;
    std::ignore = event_group.set(1U);
    std::ignore = event_group.clear(1U);
    std::ignore = event_group.modify(1U, 3U);
    std::ignore = event_group.get();
    std::ignore = event_group.wait_any(1U);
    std::ignore = event_group.wait_any_for(1U, rel_time);
    std::ignore = event_group.wait_any_until(1U, abs_time);
    std::ignore = event_group.wait_all(1U);
    std::ignore = event_group.wait_all_for(1U, rel_time);
    std::ignore = event_group.wait_all_until(1U, abs_time);
    std::ignore = event_group.shared_wait_any(1U);
    std::ignore = event_group.shared_wait_any_for(1U, rel_time);
    std::ignore = event_group.shared_wait_any_until(1U, abs_time);
    std::ignore = event_group.shared_wait_all(1U);
    std::ignore = event_group.shared_wait_all_for(1U, rel_time);
    std::ignore = event_group.shared_wait_all_until(1U, abs_time);

    zephyr::message_queue_instance<int, 4> queue;
    queue.post(1);
    std::ignore = queue.try_post(1);
    std::ignore = queue.try_post_for(1, rel_time);
    std::ignore = queue.try_post_until(1, abs_time);
    std::ignore = queue.get();
    std::ignore = queue.try_get();
    std::ignore = queue.try_get_for(rel_time);
    std::ignore = queue.try_get_until(abs_time);
    std::ignore = queue.peek();
    std::ignore = queue.size();
    std::ignore = queue.free_space();
    std::ignore = queue.empty();
    std::ignore = queue.full();
    queue.flush();

    zephyr::signal signal;
    signal.raise(1);
    std::ignore = signal.check();
    signal.reset();

    std::array<::k_poll_event, 3> events{zephyr::poll_event{signal}, zephyr::poll_event{queue},
                                         zephyr::poll_event{semaphore}};
    std::ignore = zephyr::poll_for(events, zephyr::tick_timer::duration{1});

    zephyr::thread thread{test_stack, thread_entry, nullptr, nullptr, nullptr, 1};
    zephyr::thread delayed_thread{test_stack, thread_entry, nullptr, nullptr, nullptr, 1,
                                  0,          rel_time};
    std::ignore = thread.get_name();
    std::ignore = thread.set_name("test");
    std::ignore = thread.get_priority();
    thread.set_priority(2);
    thread.suspend();
    thread.resume();
    std::ignore = thread.join();
    std::ignore = thread.join(rel_time);
    thread.abort();
    std::ignore = zephyr::thread::current();
    zephyr::this_thread::yield();
    std::ignore = zephyr::this_thread::can_yield();
    std::ignore = zephyr::this_thread::preemptible();
    zephyr::this_thread::sleep_for(zephyr::tick_timer::duration{1});
    zephyr::this_thread::sleep_for(rel_time);
    zephyr::this_thread::sleep_until(abs_time);

    zephyr::work work{work_handler};
    std::ignore = work.submit();
    std::ignore = work.cancel();
    std::ignore = work.is_pending();

    zephyr::work_poll work_poll{poll_work_handler};
    std::array<zephyr::poll_event, 1> work_events{zephyr::poll_event{signal}};
    std::ignore = work_poll.submit(work_events, zephyr::tick_timer::duration{1});
    std::ignore = work_poll.cancel();

    zephyr::work_delayable delayable{delayable_work_handler};
    std::ignore = delayable.is_pending();
    std::ignore = delayable.expiration();
    std::ignore = delayable.remaining_time();
    std::ignore = delayable.schedule(rel_time);
    std::ignore = delayable.reschedule(rel_time);
    std::ignore = delayable.cancel();
}
} // namespace

int main()
{
    return 0;
}
