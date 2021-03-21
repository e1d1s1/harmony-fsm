#pragma once

#include "event_table_entry.hpp"
#include <vector>

namespace fsm {

/**
* @class FiniteStateMachine
* @author Eric D. Schmidt
* @date 3/10/2021
* @brief Finite state machine used to enforce proper state transitions. 
*/
template <typename TEvent, typename TState>
class FiniteStateMachine
{
public:
  FiniteStateMachine(const FiniteStateMachine& other) = default;
  FiniteStateMachine(const std::vector< EventTableEntry< TEvent, TState > >& fsm_table, TState init_state) : current_state_(init_state),
    fsm_table_(fsm_table)
  {}
  /**
  * @brief Execute a state machine transition
  * @param trigger
  * @return true if the state change was executed successfully
  */
  virtual bool doEvent(TEvent trigger)
  {
    for (const auto& entry : fsm_table_)
    {
      if (entry.Current == current_state_)
      {
        if (entry.Trigger == trigger)
        {
          current_state_ = entry.Result;
          return true;
        }
      }
    }

    return false;
  }

  virtual TState getCurrentState() const { return current_state_; }

  virtual ~FiniteStateMachine() {}

protected:
  TState current_state_;
  const std::vector< EventTableEntry< TEvent, TState > >& fsm_table_;
};


}
