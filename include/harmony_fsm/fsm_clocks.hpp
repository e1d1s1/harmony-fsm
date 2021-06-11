/**
 * @file fsm_clocks.hpp
 * @author Eric D. Schmidt (e1d1s1@hotmail.com)
 * @brief Harmony Finite State Machine clock/time interface
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
    return TROS::now().toSec();
  }
};

using FSMROSClock = FSMROSBaseClock<ros::Time>;

#endif

}