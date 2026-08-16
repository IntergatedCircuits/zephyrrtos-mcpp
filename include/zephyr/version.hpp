// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <cstdint>
#include <string_view>
#if __has_include("zephyr/app_version.h")
#include <zephyr/app_version.h>
#endif
#if __has_include("zephyr/version.h")
#include <zephyr/version.h>
#endif

namespace zephyr
{
struct version_number
{
    std::uint8_t major;
    std::uint8_t minor;
    std::uint8_t patch;
    std::uint8_t tweak;

    constexpr bool empty() const { return to_uint32() == 0; }
    constexpr unsigned diff_depth(const version_number& other) const
    {
        if (major != other.major)
        {
            return 4;
        }
        if (minor != other.minor)
        {
            return 3;
        }
        if (patch != other.patch)
        {
            return 2;
        }
        if (tweak != other.tweak)
        {
            return 1;
        }
        return 0;
    }

    constexpr std::uint32_t to_uint32() const
    {
        return (static_cast<std::uint32_t>(major) << 24) |
               (static_cast<std::uint32_t>(minor) << 16) |
               (static_cast<std::uint32_t>(patch) << 8) | (static_cast<std::uint32_t>(tweak));
    }
    static constexpr version_number from_uint32(std::uint32_t v)
    {
        return version_number{
            .major = static_cast<std::uint8_t>((v >> 24) & 0xFF),
            .minor = static_cast<std::uint8_t>((v >> 16) & 0xFF),
            .patch = static_cast<std::uint8_t>((v >> 8) & 0xFF),
            .tweak = static_cast<std::uint8_t>(v & 0xFF),
        };
    }
};

#ifdef APPVERSION
constexpr version_number app_version()
{
    return version_number::from_uint32(APPVERSION);
}
constexpr std::string_view app_version_string()
{
    return APP_VERSION_EXTENDED_STRING;
}
#endif

#ifdef KERNELVERSION
constexpr version_number kernel_version()
{
    return version_number::from_uint32(KERNELVERSION);
}
constexpr std::string_view kernel_version_string()
{
    return KERNEL_VERSION_EXTENDED_STRING;
}
#endif

} // namespace zephyr
