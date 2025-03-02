#pragma once

#include <chrono>
#include <ratio>
#include <type_traits>

/**
 * Helper functions for timing
 */
namespace Surface::Time
{

unsigned int nanos = std::nano::den;

// Helper for nanosecond to second conversions
double nanos_per_second = static_cast<double>(nanos);

using hr_clock =
    std::conditional<std::chrono::high_resolution_clock::is_steady,
                     std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;
using sys_clock = std::chrono::system_clock;

/**
 * Get the current system time in seconds
 *
 * @returns seconds since epoch
 */
long long get_sys_time()
{
    return std::chrono::duration_cast<std::chrono::seconds>(sys_clock::now().time_since_epoch())
        .count();
}

/**
 * Get current high resolution nanoseconds, intended for duration measurements
 *
 * @returns monotonic nanoseconds
 */
long long get_nanos()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(hr_clock::now().time_since_epoch())
        .count();
}

} // namespace Surface::Time
