#define CATCH_CONFIG_MAIN
#include <chrono>

#include <harmony_fsm/finite_state_machine.hpp>
#include <harmony_fsm/config_parser.hpp>

#include "catch.hpp"
#include "stoplight.h"

using namespace std;

void basic_test( fsm::FiniteStateMachine< EVENT, RUNSTATE >& machine )
{
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

TEST_CASE( "FSM test" )
{
  // start at condition green
  fsm::FiniteStateMachine< EVENT, RUNSTATE > machine_from_table( STOPLIGHT_FSM_TABLE, RUNSTATE::RED );
  basic_test( machine_from_table );

  // test both constructors
  fsm::FiniteStateMachine< EVENT, RUNSTATE > machine_from_map( STOPLIGHT_FSM_MAP, RUNSTATE::RED );
  basic_test( machine_from_table );
}

// test config parser
TEST_CASE( "Parse FSM test" )
{
  auto rules = fsm::EventTableParser::parseCSV< EVENT, RUNSTATE >( "rules.csv" );
  REQUIRE( rules.size() == STOPLIGHT_FSM_TABLE.size() );

  for ( size_t i = 0; i < rules.size(); i++ )
  {
    const auto rule_csv = rules[i];
    const auto rule_hardcoded = STOPLIGHT_FSM_TABLE[i];

    REQUIRE( ( rule_csv.Trigger == rule_hardcoded.Trigger &&
      rule_csv.Current == rule_hardcoded.Current &&
      rule_csv.Result == rule_hardcoded.Result ) );
  }
}
