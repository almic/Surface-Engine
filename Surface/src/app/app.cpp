#include "app.h"

#include "time/time.h"

namespace Surface
{

double App::get_delta_time() const
{
    return delta_nano / Time::nanos_per_second;
};

double App::get_fixed_update_step() const
{
    return fixed_delta_nano / Time::nanos_per_second;
}

double App::get_max_delta_time() const
{
    return max_delta_nano / Time::nanos_per_second;
};

double App::get_uptime() const
{
    return time_nano / Time::nanos_per_second;
};

int App::run()
{
    started = true;

    setup();

    if (error_code == 0)
    {
        main();
    }

    stopped = true;

    teardown();

    return error_code;
}

void App::main()
{
    if (!started || stopped || stopping)
    {
        return;
    }

    long long current_time = Time::get_nanos();
    unsigned long long accum = 0;

    do
    {
        ++tick;

        long long new_time = Time::get_nanos();
        unsigned long long d = new_time - current_time;
        if (max_delta_nano > 0 && d > max_delta_nano)
        {
            delta_nano = max_delta_nano;
        }
        else
        {
            delta_nano = d;
        }

        // Normal update
        update();

        if (stopping >= 2)
        {
            break;
        }

        // Fixed updates
        if (fixed_delta_nano == 0)
        {
            accum = 0;

            update_fixed();
        }
        else
        {
            accum += delta_nano;
            while (accum >= fixed_delta_nano)
            {
                update_fixed();

                if (stopping >= 2)
                {
                    break;
                }

                accum -= fixed_delta_nano;
            }
        }

        if (stopping >= 2)
        {
            break;
        }

        // Post fixed updates with accumulator
        post_update_fixed(accum);

        if (stopping >= 2)
        {
            break;
        }

        // Render
        render();

        // Track real time without max delta
        time_nano += d;
    }
    while (stopping == 0);
}

int App::set_error(int error, bool override)
{
    if (override || error_code == 0)
    {
        error_code = error;
    }

    return error_code;
}

void App::set_fixed_update_step(double seconds)
{
    fixed_delta_nano = static_cast<unsigned long long>(seconds * Time::nanos);
}

void App::set_max_delta_time(double seconds)
{
    max_delta_nano = static_cast<unsigned long long>(seconds * Time::nanos);
}

bool App::stop(bool immediate)
{
    if (stopping == (immediate ? 2 : 1))
    {
        return false;
    }

    stopping = immediate ? 2 : 1;
    return true;
}

} // namespace Surface
