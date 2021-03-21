#pragma once
#include <chrono>

#ifdef USE_ROS_TIME
#include <ros/ros.h>
#endif

namespace fsm {

template <typename TChrono>
class FSMSTLBaseClock
{
public:
  static double toSec()
  {
    return std::chrono::duration<double>(TChrono::now().time_since_epoch()).count();
  }
};

using FSMSteadyClock = FSMSTLBaseClock<std::chrono::steady_clock>;
using FSMHighResClock = FSMSTLBaseClock<std::chrono::high_resolution_clock>;
using FSMSystemClock = FSMSTLBaseClock<std::chrono::system_clock>;

#ifdef USE_ROS_TIME
template <typename TROS>
class FSMROSBaseClock
{
public:
  static double toSec()
  {
    return TROS::now().toSec());
  }
};

using FSMROSClock = FSMROSBaseClock<ros::Time>;

#endif

}