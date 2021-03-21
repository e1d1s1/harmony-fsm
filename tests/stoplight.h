#include <functional>
#include <harmony-fsm/finite_state_machine.hpp>
#include <harmony-fsm/fsm_rate.hpp>
#include <harmony-fsm/fsm_runner.hpp>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

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

class StopLightOperation
{
 public:
  StopLightOperation(
      fsm::FiniteStateMachineRunner< EVENT, RUNSTATE, fsm::UnusedCommandParameter, RUNRESULT, fsm::FSMSteadyClock >&& runner )
    : runner_( runner )
  {
    timers_.emplace( RUNSTATE::RED, fsm::SteadyTimer( 5 ) );
    timers_.emplace( RUNSTATE::YELLOW, fsm::SteadyTimer( 3 ) );
    timers_.emplace( RUNSTATE::GREEN, fsm::SteadyTimer( 5 ) );

    runner_.setExecFunction( std::bind( &StopLightOperation::execute, this, std::placeholders::_1 ) );
    runner_.setCompletionHandler( std::bind( &StopLightOperation::handleResult, this, std::placeholders::_1 ) );

    runner_.start();
  }

  ~StopLightOperation() { runner_.stop(); }

  RUNRESULT execute( const fsm::UnusedCommandParameter* param )
  {
    switch ( runner_.getCurrentState() )
    {
      case RUNSTATE::RED:
        doRed();
        return timers_[RUNSTATE::RED].isElapsed() ? RUNRESULT::CYCLE_COMPLETE : RUNRESULT::CYCLE_RUNNING;
        break;

      case RUNSTATE::YELLOW:
        doYellow();
        return timers_[RUNSTATE::YELLOW].isElapsed() ? RUNRESULT::CYCLE_COMPLETE : RUNRESULT::CYCLE_RUNNING;
        break;

      case RUNSTATE::GREEN:
        doGreen();
        return timers_[RUNSTATE::YELLOW].isElapsed() ? RUNRESULT::CYCLE_COMPLETE : RUNRESULT::CYCLE_RUNNING;
        break;

      case RUNSTATE::EMERGENCY:
        doEmergency();
        return ResetEmergency ? RUNRESULT::CYCLE_COMPLETE : RUNRESULT::CYCLE_RUNNING;
        break;
    }

    return RUNRESULT::FAILED;
  }

  void stop()
  {
    runner_.stop();
  }

  void doRed( const fsm::UnusedCommandParameter* param = nullptr )
  {
    History.emplace_back( RUNSTATE::RED, fsm::FSMSteadyClock::toSec() );
    std::cout << "RED EXECUTE" << std::endl;
    if ( RedExecuted )
      RedExecuted();
  }

  void doYellow( const fsm::UnusedCommandParameter* param = nullptr )
  {
    History.emplace_back( RUNSTATE::YELLOW, fsm::FSMSteadyClock::toSec() );
    std::cout << "YELLOW EXECUTE" << std::endl;
    if ( YellowExecuted )
      YellowExecuted();
  }

  void doGreen( const fsm::UnusedCommandParameter* param = nullptr )
  {
    History.emplace_back( RUNSTATE::GREEN, fsm::FSMSteadyClock::toSec() );
    std::cout << "GREEN EXECUTE" << std::endl;
    if ( GreenExecuted )
      GreenExecuted();
  }

  void doEmergency( const fsm::UnusedCommandParameter* param = nullptr )
  {
    History.emplace_back( RUNSTATE::EMERGENCY, fsm::FSMSteadyClock::toSec() );
    std::cout << "EMERGENCY EXECUTE" << std::endl;
    if ( EmergencyExecuted )
      EmergencyExecuted();
  }

  void resetTimers()
  {
    timers_[RUNSTATE::RED].reset();
    timers_[RUNSTATE::YELLOW].reset();
    timers_[RUNSTATE::GREEN].reset();
  }

  void handleResult( const RUNRESULT& result )
  {
    if ( result == RUNRESULT::FAILED )
    {
      throw std::runtime_error( "unexpected failure during execution of state: " + std::to_string( (unsigned)runner_.getCurrentState() ) );
    }
    else if ( result == RUNRESULT::CYCLE_COMPLETE )
    {
      std::cout << "STATE completed" << std::endl;
      if (StateCompleted)
      {
        StateCompleted(runner_.getCurrentState());
        CompletionHistory.emplace_back( runner_.getCurrentState(), fsm::FSMSteadyClock::toSec() );
      }
      auto evt = EVENT::DO_NEXT_CYCLE;
      if ( runner_.getCurrentState() == RUNSTATE::EMERGENCY )
      {
        evt            = EVENT::EMERGENCY_ENDED;
        ResetEmergency = false;
      }

      resetTimers();

      runner_.doEventAndExecute( evt );
    }
    else
    {
      // kick current state again
      runner_.updateFSM();
    }
  }

  bool ResetEmergency   = false;

  std::function< void( void ) > RedExecuted    = nullptr;
  std::function< void( void ) > YellowExecuted = nullptr;
  std::function< void( void ) > GreenExecuted  = nullptr;
  std::function< void( void ) > EmergencyExecuted  = nullptr;
  std::function< void( RUNSTATE ) > StateCompleted = nullptr;

  std::vector< std::pair< RUNSTATE, double > >                                                                   History;
  std::vector< std::pair< RUNSTATE, double > >                                                                   CompletionHistory;

 private:
  std::map< RUNSTATE, fsm::SteadyTimer >                                                                         timers_;
  fsm::FiniteStateMachineRunner< EVENT, RUNSTATE, fsm::UnusedCommandParameter, RUNRESULT, fsm::FSMSteadyClock >& runner_;
  
};