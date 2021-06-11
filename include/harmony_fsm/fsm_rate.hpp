/**
 * @file fsm_rate.hpp
 * @author Eric D. Schmidt (e1d1s1@hotmail.com)
 * @brief Harmony Finite State Machine rates/timers
 * @date 2021-03-22
 * 
 * @copyright Copyright (c) 2021
 * 
 * This is free and unencumbered software released into the public domain.
 * 
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 * 
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * 
 * For more information, please refer to <https://unlicense.org>
 */

#pragma once

#include <type_traits>
#include <thread>

#include "fsm_clocks.hpp"

namespace fsm {
/**
 * @brief Heavily influence by ROS Rate, Duration, but for std::chrono
 *
 * @tparam TClock
 */
template < typename TClock >
class BaseRate
{
 public:
  BaseRate( double frequency ) 
  : start_( TClock::toSec() )
  , expected_cycle_time_( 1.0 / frequency )
  {}

  void sleep()
  {
    double expected_end = start_ + expected_cycle_time_;

    double actual_end = TClock::toSec();

    // deal with a backwards jump
    if ( actual_end < start_ )
    {
      expected_end = actual_end + expected_cycle_time_;
    }

    // next sleep duration
    double sleep_duration = expected_end - actual_end;

    // set actual loop time
    actual_cycle_time_ = actual_end - start_;

    start_ = expected_end;

    if ( sleep_duration <= 0 )
    {
      // jump or past cycle, reset cycle
      if ( actual_end > expected_end + expected_cycle_time_ )
      {
        start_ = actual_end;
      }
    }

    sleep( sleep_duration );
  }

 protected:
  double expected_cycle_time_;
  double actual_cycle_time_ = 0;
  double start_;

 private:
  void sleep(double duration)
  {
    std::this_thread::sleep_for( std::chrono::duration<double>( duration ) );
  }
};

template <typename TClock>
class BaseTimer : public BaseRate<TClock>
{
  public:
    BaseTimer() : BaseRate<TClock>(0) {}
    BaseTimer( double time_secs ) : BaseRate<TClock>( 1 / time_secs ) {}
    void reset()
    {
      BaseRate<TClock>::start_ = TClock::toSec();
    }

    void set_timeout( double time_secs ) { BaseRate<TClock>::expected_cycle_time_ = time_secs; }

    bool isElapsed()
    {
      return TClock::toSec() - BaseRate<TClock>::start_ >= BaseRate<TClock>::expected_cycle_time_;
    }
};

using SteadyRate = BaseRate< FSMSteadyClock >;
using HighResRate = BaseRate< FSMHighResClock >;
using SystemRate = BaseRate< FSMSystemClock >;

using SteadyTimer = BaseTimer< FSMSteadyClock >;
using HighResTimer = BaseTimer< FSMHighResClock >;
using SystemTimer = BaseTimer< FSMSystemClock >;

#ifdef USE_ROS_TIME
using ROSRate = BaseRate< FSMROSClock >;

using ROSTimer = BaseTimer< FSMROSClock >;
#endif

}