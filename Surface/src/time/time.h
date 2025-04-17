#pragma once

#include <chrono>
#include <ratio>
#include <type_traits>

/**
 * Helper functions for timing
 */
namespace Surface::Time
{

constexpr unsigned int nanos = std::nano::den;

// Helper for nanosecond to second conversions
constexpr double nanos_per_second = static_cast<double>(nanos);

using hr_clock =
    std::conditional<std::chrono::high_resolution_clock::is_steady,
                     std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;
using sys_clock = std::chrono::system_clock;

/**
 * Get the current system time in seconds
 *
 * @returns seconds since epoch
 */
inline long long get_sys_time()
{
    return std::chrono::duration_cast<std::chrono::seconds>(sys_clock::now().time_since_epoch())
        .count();
}

/**
 * Get current high resolution nanoseconds, intended for duration measurements
 *
 * @returns monotonic nanoseconds
 */
inline long long get_nanos()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(hr_clock::now().time_since_epoch())
        .count();
}

// A simple timer, you can query the duration elapsed since the timer was created. Fully disabled in
// debug mode. Provide a simple "log_to" method that calls the provided function with a char* as the
// first argument.
struct Timer
{
#ifdef DEBUG
    Timer() : start(hr_clock::now())
    {
    }
#else
    Timer() = default;
#endif

    template <typename Writer> inline void log_to(Writer write) const
    {
#ifdef DEBUG
        char unit[2]{0};
        long long scale = 0;

        auto duration = elapsed().count();
        if (duration < 100)
        {
            // Nanos
            unit[0] = 'n';
            unit[1] = 's';

            scale = 0;
        }
        else if (duration < 100000)
        {
            // Micros
            unit[0] = 'u';
            unit[1] = 's';

            scale = 1000;
        }
        else if (duration < 100000000)
        {
            // Millis
            unit[0] = 'm';
            unit[1] = 's';

            scale = 1000000;
        }
        else
        {
            // Seconds
            unit[0] = 's';

            scale = 1000000000;
        }

        long long whole = scale ? duration / scale : duration;
        long long deci = scale ? (duration - (whole * scale)) * 1000 / scale : -1;

        // XXX.Xyy0
        char text[8]{0};
        unsigned char idx = 0;

        if (whole > 9999)
        {
            text[idx] = '>';
            text[++idx] = '9';
            text[++idx] = '9';
            text[++idx] = '9';
            text[++idx] = '9';
        }
        else
        {
            unsigned short s;
            if (whole >= 1000)
            {
                s = 1000;
            }
            else if (whole >= 100)
            {
                s = 100;
            }
            else if (whole >= 10)
            {
                s = 10;
            }
            else
            {
                s = 1;
            }

            while (idx < 4 && s > 0)
            {
                auto c = whole / s;
                text[idx] = (char) ('0' + c);
                ++idx;
                whole -= c * s;
                s /= 10;
            }

            if (idx < 4)
            {
                text[idx] = '.';
                ++idx;
                if (deci >= 100)
                {
                    s = 100;
                }
                else if (deci >= 10)
                {
                    text[idx] = '0';
                    ++idx;
                    s = 10;
                }
                else
                {
                    text[idx] = '0';
                    ++idx;
                    if (idx < 5)
                    {
                        text[idx] = '0';
                        ++idx;
                    }
                    s = 1;
                }

                while (idx < 5 && s > 0)
                {
                    auto c = deci / s;
                    text[idx] = (char) ('0' + c);
                    ++idx;
                    deci -= c * s;
                    s /= 10;
                }
            }
        }

        text[idx] = unit[0];
        text[++idx] = unit[1];

        write(text);
#endif
    }

    inline float seconds() const
    {
#ifdef DEBUG
        return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed()).count() / 1000.f;
#else
        return -1.f;
#endif
    }

    inline float millis() const
    {
#ifdef DEBUG
        return std::chrono::duration_cast<std::chrono::microseconds>(elapsed()).count() / 1000.f;
#else
        return -1.f;
#endif
    }

    inline float micros() const
    {
#ifdef DEBUG
        return elapsed().count() / 1000.f;
#else
        return -1.f;
#endif
    }

    inline long long nanos() const
    {
#ifdef DEBUG
        return elapsed().count();
#else
        return -1;
#endif
    }

    inline hr_clock::duration elapsed() const
    {
#ifdef DEBUG
        return hr_clock::now() - start;
#else
        return hr_clock::duration();
#endif
    }

#ifdef DEBUG
    hr_clock::time_point start;
#endif
};

} // namespace Surface::Time
