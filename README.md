# harmony-fsm
A flexible and simple finite state machine in C++

## Basic State Machine Operation

A finite state machine is great way to enforce and allow only specified transitions between operating conditions in your software. You are much less likely to end up with undefined behavior than using what often turns into large switch/case statements or nested code. In harmony-fsm, these transitions are defined in tables with rows defining all the permutations of:
current state -> event -> resultant state

```C++

enum class EVENT
{
  DO_NEXT_CYCLE,
  EMERGENCY_DECLARED,
  EMERGENCY_ENDED
};

enum class RUNSTATE
{
  GREEN,
  YELLOW,
  RED,
  EMERGENCY
};

enum class RUNRESULT
{
  CYCLE_COMPLETE,
  CYCLE_RUNNING,
  FAILED
};

static const std::vector< fsm::EventTableEntry< EVENT, RUNSTATE > > STOPLIGHT_FSM_TABLE = {
    // basic operation
    { EVENT::DO_NEXT_CYCLE, RUNSTATE::GREEN, RUNSTATE::YELLOW },
    { EVENT::DO_NEXT_CYCLE, RUNSTATE::YELLOW, RUNSTATE::RED },
    { EVENT::DO_NEXT_CYCLE, RUNSTATE::RED, RUNSTATE::GREEN },

    // emergency vehicle flashing, can happen at any time
    { EVENT::EMERGENCY_DECLARED, RUNSTATE::GREEN, RUNSTATE::EMERGENCY },
    { EVENT::EMERGENCY_DECLARED, RUNSTATE::YELLOW, RUNSTATE::EMERGENCY },
    { EVENT::EMERGENCY_DECLARED, RUNSTATE::RED, RUNSTATE::EMERGENCY },

    // back to red when emergency is done
    { EVENT::EMERGENCY_ENDED, RUNSTATE::EMERGENCY, RUNSTATE::RED } };

```

Once constructed, you can call FiniteStateMachine::doEvent( const TEvent& trigger ) to evaluate whether a transition is valid and to switch states. 

```C++
// given machine.getCurrentState() == RUNSTATE::RED
if ( machine.doEvent( EVENT::DO_NEXT_CYCLE ) )
{
  ASSERT( machine.getCurrentState() == RUNSTATE::GREEN );
}

```

## Advanced State Management - FiniteStateMachineRunner

The FiniteStateMachineRunner expands on the simplistic model to provide a threaded worker-watchdog model and allows you to run a system at a fixed frequency. THe watchdog thread watches for execution times that exceed a given threshold and handles the results returned from execution and calls for state changes. The worker thread executes the correct function for each state. States and their related functions can be managed either by a map of function pointers, or pass through a common execution branch (switch/case). You can optionally handle exceptions from these functions in the runner.

See the unit tests for examples.

## ROS Support

If you configure cmake with "ROS_TIME" enabled, you can use ros::Time as the clock for loop rate management and timeouts. Otherwise, std::chrono is used.