/**
 * @file fsm_clocks.hpp
 * @author Eric D. Schmidt (e1d1s1@hotmail.com)
 * @brief Harmony Finite State Machine clock/time interface
 * @date 2021-03-22
 * 
 * @copyright Copyright (c) 2021
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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