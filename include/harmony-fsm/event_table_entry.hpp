#pragma once

namespace fsm {
/**
* @class EventTableEntry
* @author Eric D. Schmidt
* @date 3/10/2021
* @brief A structure to hold a single "row" definition of current state -> event -> resultant state changes
*/
template < typename TEvent, typename TState>
struct EventTableEntry
{
  TEvent Trigger;
  TState Current;
  TState Result;
};

}