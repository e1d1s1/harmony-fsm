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
#endif

}