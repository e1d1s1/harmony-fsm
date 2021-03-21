#define CATCH_CONFIG_MAIN
#include <chrono>

#include <harmony-fsm/finite_state_machine.hpp>

#include "catch.hpp"
#include "stoplight.h"

using namespace std;

TEST_CASE( "FSM test" )
{
  // start at condition green
  fsm::FiniteStateMachine< EVENT, RUNSTATE > machine( STOPLIGHT_FSM_TABLE, RUNSTATE::RED );

  // this is an improper, undefined transition and should fail
  REQUIRE( machine.doEvent( EVENT::EMERGENCY_ENDED ) == false );
  REQUIRE( machine.getCurrentState() == RUNSTATE::RED );

  // run through defined transitions
  REQUIRE( machine.doEvent( EVENT::DO_NEXT_CYCLE ) == true );
  REQUIRE( machine.getCurrentState() == RUNSTATE::GREEN );

  REQUIRE( machine.doEvent( EVENT::DO_NEXT_CYCLE ) == true );
  REQUIRE( machine.getCurrentState() == RUNSTATE::YELLOW );

  REQUIRE( machine.doEvent( EVENT::DO_NEXT_CYCLE ) == true );
  REQUIRE( machine.getCurrentState() == RUNSTATE::RED );
}
