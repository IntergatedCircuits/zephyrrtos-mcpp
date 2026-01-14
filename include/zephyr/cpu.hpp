// SPDX-License-Identifier: Apache-2.0
#ifndef __ZEPHYR_CPU_HPP
#define __ZEPHYR_CPU_HPP

#include "zephyr/tick_timer.hpp"
#include <zephyr/spinlock.h>

namespace zephyr
{
using spinlock = ::k_spinlock;

class critical_section
{
  public:
    critical_section(spinlock& lock) : lock_(lock) { restore_ = ::k_spin_lock(&lock_); }
    ~critical_section() { ::k_spin_unlock(&lock_, restore_); }

  private:
    spinlock& lock_;
    ::k_spinlock_key_t restore_{};
};

namespace this_cpu
{
/// @brief  Determines if the current execution context is inside
///         an interrupt service routine.
/// @return true if the current execution context is ISR, false otherwise
bool is_in_isr()
{
    return ::k_is_in_isr();
}

} // namespace this_cpu
} // namespace zephyr

#endif // __ZEPHYR_CPU_HPP