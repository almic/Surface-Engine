#pragma once

namespace Surface
{

/**
 * A robust Application class that defines an application loop, supporting variable and fixed
 * timesteps, and methods which run in pre-defined order on each loop.
 *
 * This is meant to be implemented by subclasses.
 */
class App
{
  private:
    // error code returned by `App::run()`
    int error_code = 0;

    // if App::run() has been called
    bool started = false;

    // if the application has stopped
    bool stopped = false;

    // whether the application is scheduled to stop, and when to stop.
    // 0 = not stopping, 1 = stop after loop, 2 = stop after current method
    unsigned char stopping = 0;

    // current application loop tick, initially the max value
    unsigned long tick = -1;

    // delta time in nanoseconds since the last main loop
    unsigned long long delta_nano = 0;

    // max delta time in nanoseconds between ticks
    unsigned long long max_delta_nano = 0;

    // fixed update delta time in nanoseconds, precomputed from fixed_update_step
    unsigned long long fixed_delta_nano = 0;

    // application run time tracked in nanoseconds, updated at the end of each main loop
    unsigned long long time_nano = 0;

  public:
    /**
     * Get the current delta time in seconds that elapsed since the last tick.
     *
     * If `fixed_update_step` is greater than zero and you are within `App::update_fixed()`, you
     * should use `fixed_update_step` for all calculations instead.
     */
    double get_delta_time() const;

    /**
     * @returns the current error code of the application, `0` means no error
     */
    int get_error() const
    {
        return error_code;
    };

    /**
     * Get the fixed update step used for `App::update_fixed()`.
     *
     * When this is zero, `App::update_fixed()` is called once per tick, otherwise it may be called
     * zero or more times per tick.
     */
    double get_fixed_update_step() const;

    /**
     * Get the maximum delta time between ticks.
     *
     * When this is greater than zero, `get_delta_time()` will never return larger values.
     */
    double get_max_delta_time() const;

    /**
     * Get the total running time of the application.
     */
    double get_uptime() const;

    /**
     * @returns the current tick of the application, a maximum value (`-1`) means the application
     * has not been started
     */
    int tick_count() const
    {
        return tick;
    };

    /**
     * @returns `true` if `App::run()` is currently executing
     */
    bool is_running() const
    {
        return !stopped;
    };

    /**
     * @returns `true` if `App::run()` has been called
     */
    bool is_started() const
    {
        return started;
    };

    /**
     * @returns `true` if the application is scheduled to stop and has not already stopped
     */
    bool is_stopping() const
    {
        return !stopped && stopping > 0;
    };

    /**
     * Runs the application loop, this call returns when the application stops.
     * If an error code is set before stopping, this returns that error code.
     *
     * @returns error code, `0` means no error
     */
    int run();

    /**
     * Set an error code to return when the application stops. The error code can also be retrieved
     * with `get_error()`. This should be called before `App::stop()`, and you should call
     * `App::stop()` after calling this method!
     *
     * Has no effect if the application is already stopping and the current error code is non-zero,
     * unless `override` is `true`.
     *
     * @param error the error value to set
     * @param override if an error is already set, `true` will override that error
     *
     * @returns the current error code
     */
    int set_error(int error, bool override = false);

    /**
     * Set the fixed update step for `App::update_fixed()`.
     *
     * @param seconds fixed update step time, set to zero to call `App::update_fixed()` once per
     * tick
     */
    void set_fixed_update_step(double seconds);

    /**
     * Set the maximum delta time between ticks, in seconds.
     *
     * @param seconds max delta time between ticks
     */
    void set_max_delta_time(double seconds);

    /**
     * Schedules the application to stop execution at the end of the current loop. Pass `true` to
     * stop the application after the current method completes.
     *
     * Calling this method multiple times has no effect unless the first call was to stop after the
     * current loop and a later call is to stop immediately.
     *
     * @param immediate stop execution after the current method completes
     *
     * @returns `true` if the call has changed the stopping status
     */
    bool stop(bool immediate = false);

  private:
    /**
     * Called once in `App::run()` before `App::main()`
     */
    virtual void setup()
    {
        return;
    };

    /**
     * Contains the main loop of the application, called by `App::run()` after `App::setup()`
     * completes with no error.
     *
     * If you override this method, you should increment `tick` at the start of each loop, and end
     * the loop when `stopping` is non-zero.
     */
    virtual void main();

    /**
     * Called once per tick in `App::main_loop()`. For simple apps, this may contain all logic.
     */
    virtual void update() = 0;

    /**
     * Called in `App::main_loop()` after `App::update()`. When `fixed_update_step > 0`, this may be
     * called zero or more times according to the elapsed time between ticks and the
     * `fixed_update_step` value.
     *
     * This method should assume that `fixed_update_step` time has elapsed when called.
     */
    virtual void update_fixed()
    {
        return;
    };

    /**
     * Called after all calls to `App::update_fixed()` have completed, passing in the accumulated
     * nanoseconds less than `App::get_fixed_update_step()`. This is for advanced applications that
     * may want to interpolate some state with the extra time.
     */
    virtual void post_update_fixed(unsigned long long accumulated_nanos)
    {
        return;
    };

    /**
     * Called once per tick after `App::update()` and `App::update_fixed()` completes.
     */
    virtual void render()
    {
        return;
    };
};

} // namespace Surface
