#pragma once

#include <windows.h>

class high_resolution_timer
{
public:
	high_resolution_timer()
	{
		LONGLONG counts_per_sec;
		QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&counts_per_sec));
		seconds_per_count = 1.0 / static_cast<double>(counts_per_sec);

		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&this_time));
		base_time = this_time;
		last_time = this_time;
	}
	~high_resolution_timer() = default;
	high_resolution_timer(const high_resolution_timer&) = delete;
	high_resolution_timer& operator=(const high_resolution_timer&) = delete;
	high_resolution_timer(high_resolution_timer&&) noexcept = delete;
	high_resolution_timer& operator=(high_resolution_timer&&) noexcept = delete;

	// Returns the total time elapsed since Reset() was called, NOT counting any
	// time when the clock is stopped.
	float time_stamp() const  // in seconds
	{
		// If we are stopped, do not count the time that has passed since we stopped.
		// Moreover, if we previously already had a pause, the distance 
		// stop_time - base_time includes paused time, which we do not want to count.
		// To correct this, we can subtract the paused time from mStopTime:  
		//
		//                     |<--paused_time-->|
		// ----*---------------*-----------------*------------*------------*------> time
		//  base_time       stop_time        start_time     stop_time    this_time

		if (stopped)
		{
			return static_cast<float>(((stop_time - paused_time) - base_time) * seconds_per_count);
		}

		// The distance this_time - mBaseTime includes paused time,
		// which we do not want to count.  To correct this, we can subtract 
		// the paused time from this_time:  
		//
		//  (this_time - paused_time) - base_time 
		//
		//                     |<--paused_time-->|
		// ----*---------------*-----------------*------------*------> time
		//  base_time       stop_time        start_time     this_time
		else
		{
			return static_cast<float>(((this_time - paused_time) - base_time) * seconds_per_count);
		}
	}

	float time_interval() const  // in seconds
	{
		return static_cast<float>(delta_time);
	}

	void reset() // Call before message loop.
	{
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&this_time));
		base_time = this_time;
		last_time = this_time;

		stop_time = 0;
		stopped = false;
	}

	void start() // Call when unpaused.
	{
		LONGLONG start_time;
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&start_time));

		// Accumulate the time elapsed between stop and start pairs.
		//
		//                     |<-------d------->|
		// ----*---------------*-----------------*------------> time
		//  base_time       stop_time        start_time     
		if (stopped)
		{
			paused_time += (start_time - stop_time);
			last_time = start_time;
			stop_time = 0;
			stopped = false;
		}
	}

	void stop() // Call when paused.
	{
		if (!stopped)
		{
			QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&stop_time));
			stopped = true;
		}
	}

	void tick() // Call every frame.
	{
		if (stopped)
		{
			delta_time = 0.0;
			return;
		}

		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&this_time));
		// Time difference between this frame and the previous.
		delta_time = (this_time - last_time) * seconds_per_count;

		// Prepare for next frame.
		last_time = this_time;

		// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
		// processor goes into a power save mode or we get shuffled to another
		// processor, then mDeltaTime can be negative.
		if (delta_time < 0.0)
		{
			delta_time = 0.0;
		}
	}

private:
	double seconds_per_count{ 0.0 };
	double delta_time{ 0.0 };

	LONGLONG base_time{ 0LL };
	LONGLONG paused_time{ 0LL };
	LONGLONG stop_time{ 0LL };
	LONGLONG last_time{ 0LL };
	LONGLONG this_time{ 0LL };

	bool stopped{ false };
};
