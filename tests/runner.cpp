#define CATCH_CONFIG_RUNNER
#include <chrono>
#include <harmony-fsm/finite_state_machine.hpp>

#include "catch.hpp"
#include "stoplight.h"

using namespace std;
using dseconds = std::chrono::duration< double >;

TEST_CASE( "rate test" )
{
  fsm::SteadyRate rate( 10 );

  auto start = chrono::steady_clock::now();
  int  cnt   = 0;
  while ( chrono::steady_clock::now() - start < dseconds( 1.00 ) )
  {
    rate.sleep();
    cnt++;
  }

  REQUIRE( cnt == 10 );
}

#ifdef USE_ROS_TIME
TEST_CASE( "ROS rate test" )
{
  fsm::ROSRate rate( 10 );

  auto start = chrono::steady_clock::now();
  int  cnt   = 0;
  while ( chrono::steady_clock::now() - start < dseconds( 1.00 ) )
  {
    rate.sleep();
    cnt++;
  }

  REQUIRE( cnt == 10 );
}
#endif

void run_test( bool byFuncMap )
{
  bool caughtException     = false;
  bool redCycled           = false;
  bool yellowCycled        = false;
  bool triggerEmergency    = false;
  int  greenCycleCount     = 0;
  int  emergencyCycleCount = 0;
  int  greenCompletedCount = 0;
  bool handledException    = false;
  bool timedOut            = false;
  int  preExecCount        = 0;

#ifdef USE_ROS_TIME
  fsm::FiniteStateMachineRunner< EVENT, RUNSTATE, fsm::UnusedCommandParameter, RUNRESULT, fsm::FSMROSClock > runner(
#else
  fsm::FiniteStateMachineRunner< EVENT, RUNSTATE, fsm::UnusedCommandParameter, RUNRESULT, fsm::FSMSteadyClock > runner(
#endif
      STOPLIGHT_FSM_TABLE, RUNSTATE::RED, RUNRESULT::CYCLE_RUNNING, 10 );

  auto start = chrono::steady_clock::now();

  runner.setExceptionHandler( [&]( const std::exception& ex ) {
    cerr << "Exception handled: " << ex.what() << endl;
    handledException = true;
  } );

  runner.setPreExecFunction( [&]( const fsm::UnusedCommandParameter* param ) {
    preExecCount++;
    if ( greenCompletedCount >= 4 )
    {
      throw std::runtime_error( "Testing exception handler" );
    }

    if ( triggerEmergency )
    {
      cout << "Preempted state, declare emergency flash mode" << endl;
      runner.doEvent( EVENT::EMERGENCY_DECLARED );
      triggerEmergency = false;
    }
  } );

  runner.setTimeoutHandler( [&]( double timestamp_sec ) {
    std::cerr << "Timeout occurred at " << timestamp_sec << std::endl;
    timedOut = true;
  } );

  runner.setTimeout( 10 );

  StopLightOperation operation( byFuncMap, std::move( runner ) );
  operation.RedExecuted       = [&]() { redCycled = true; };
  operation.YellowExecuted    = [&]() { yellowCycled = true; };
  operation.GreenExecuted     = [&]() { greenCycleCount++; };
  operation.EmergencyExecuted = [&]() { emergencyCycleCount++; };
  operation.StateCompleted    = [&]( RUNSTATE state ) {
    if ( state == RUNSTATE::GREEN )
    {
      greenCompletedCount++;
    }
  };

  fsm::SteadyRate rate( 10 );
  while ( chrono::steady_clock::now() - start < dseconds( 90 ) )
  {
    rate.sleep();
    if ( handledException && timedOut )
    {
      break;
    }
    else if ( emergencyCycleCount == 0 && greenCompletedCount >= 2 )
    {
      // trigger emergency
      triggerEmergency = true;
    }
    else if ( emergencyCycleCount >= 10 )
    {
      operation.ResetEmergency = true;
    }
  }

  operation.stop();
  cout << "Test time: " << chrono::duration_cast< dseconds >( chrono::steady_clock::now() - start ).count() << "s" << endl;

  REQUIRE( redCycled );
  REQUIRE( yellowCycled );
  REQUIRE( greenCycleCount > 0 );
  REQUIRE( handledException );
  REQUIRE( timedOut );
  REQUIRE( emergencyCycleCount > 0 );

  REQUIRE( operation.CompletionHistory[0].first == RUNSTATE::RED );
  REQUIRE( operation.CompletionHistory[1].first == RUNSTATE::GREEN );
  REQUIRE( operation.CompletionHistory[2].first == RUNSTATE::YELLOW );
  REQUIRE( operation.CompletionHistory[3].first == RUNSTATE::RED );
}

TEST_CASE( "runner_test_single_exec" )
{
  run_test( false );
}

TEST_CASE( "runner_test_map_exec" )
{
  run_test( true );
}

int main (int argc, char * argv[]) 
{
#ifdef USE_ROS_TIME
  ros::init(argc, argv, "runner");
  ros::NodeHandle nh;
#endif
  return Catch::Session().run( argc, argv );
}