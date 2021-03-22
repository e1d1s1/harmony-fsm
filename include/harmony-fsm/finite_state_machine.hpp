/**
 * @file finite_state_machine.hpp
 * @author Eric D. Schmidt (e1d1s1@hotmail.com)
 * @brief Harmony Finite State Machine template
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

#include <vector>

#include "event_table_entry.hpp"

namespace fsm
{
/**
 * @class FiniteStateMachine
 * @author Eric D. Schmidt
 * @date 3/10/2021
 * @brief Finite state machine used to enforce proper state transitions.
 */
template < typename TEvent, typename TState >
class FiniteStateMachine
{
 public:
  FiniteStateMachine( const FiniteStateMachine& other ) = default;

  /**
   * @brief Construct a new Finite State Machine object.
   * 
   * @param fsm_table 
   * @param init_state 
   */
  FiniteStateMachine( std::vector< EventTableEntry< TEvent, TState > > fsm_table, TState init_state )
    : current_state_( init_state )
    , fsm_table_( fsm_table )
  {
  }
  /**
   * @brief Execute a state machine transition
   * @param trigger
   * @return true if the state change was executed successfully
   */
  virtual bool doEvent( const TEvent& trigger )
  {
    TState res;
    if ( isValid( trigger, res ) )
    {
      current_state_ = res;
      return true;
    }

    return false;
  }

  /**
   * @brief Checks whether the current transition event could yield a new state
   * 
   * @param trigger 
   * @param next_state The next state given this transition
   * @return true 
   * @return false 
   */
  virtual bool isValid( const TEvent& trigger, TState& next_state )
  {
    for ( const auto& entry : fsm_table_ )
    {
      if ( entry.Current == current_state_ )
      {
        if ( entry.Trigger == trigger )
        {
          next_state = entry.Result;
          return true;
        }
      }
    }

    return false;
  }

  virtual TState getCurrentState() const
  {
    return current_state_;
  }

  virtual ~FiniteStateMachine()
  {
  }

 protected:
  TState                                                  current_state_;
  const std::vector< EventTableEntry< TEvent, TState > >  fsm_table_;
};

}  // namespace fsm
