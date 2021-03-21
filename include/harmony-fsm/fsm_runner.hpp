#pragma once

#include <array>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

#include "fsm_rate.hpp"

namespace fsm
{
using UnusedCommandParameter = int;
/**
 * @brief Runs a generic Finite State Machine in a watchdog/worker thread model.
 * Provides callbacks and functions for state machine responses and timeouts.
 */
template < typename TEvent, typename TState, typename TCommandParameter, typename TResult, typename TClock >
class FiniteStateMachineRunner : public FiniteStateMachine< TEvent, TState >
{
 public:
  FiniteStateMachineRunner( const std::vector< EventTableEntry< TEvent, TState > >& fsm_table,
                            TState                                                  init_state,
                            TResult&&                                               init_result,
                            double                                                  frequency,
                            std::function< TResult( const TCommandParameter* ) >    exec_fun           = nullptr,
                            std::function< void( TResult& ) >                       completion_handler = nullptr,
                            std::function< void( const TCommandParameter* ) >       pre_exec_fun       = nullptr,
                            std::function< void( const double ) >                   timeout_handler    = nullptr,
                            std::function< void( const std::exception& ) >          exception_handler  = nullptr )
    : FiniteStateMachine< TEvent, TState >( fsm_table, init_state )
    , rate_( BaseRate< TClock >( frequency ) )
    , execute_fun_( exec_fun )
    , completion_handler_fun_( completion_handler )
    , pre_exec_fun_( pre_exec_fun )
    , timeout_handler_fun_( timeout_handler )
    , exception_handler_fun_( exception_handler )
    , last_worker_result_( std::move( init_result ) )
  {
    shutdown_desired_     = false;
    has_new_command_      = false;
    has_new_result_       = false;
    worker_ready_         = false;
    last_worker_response_ = TClock::toSec();
  }

  virtual ~FiniteStateMachineRunner()
  {
    stop();
    if ( worker_.joinable() )
    {
      worker_.join();
    }

    if ( watchdog_.joinable() )
    {
      watchdog_.join();
    }
  }

  void start()
  {
    shutdown_desired_ = false;
    has_new_command_  = false;
    has_new_result_   = false;
    worker_ready_     = false;
    worker_           = std::thread( &FiniteStateMachineRunner::workerThread, this );
    watchdog_         = std::thread( &FiniteStateMachineRunner::watchdogThread, this );
  }

  void stop()
  {
    shutdown_desired_ = true;
    cond_wakeup_.notify_all();
  }

  bool stopping()
  {
    return shutdown_desired_;
  }

  void setExecFunction( std::function< TResult( const TCommandParameter* ) > execute_fun )
  {
    execute_fun_ = execute_fun;
  }

  void setPreExecFunction( std::function< void( const TCommandParameter* ) > pre_exec_fun )
  {
    pre_exec_fun_ = pre_exec_fun;
  }

  void setCompletionHandler( std::function< void( TResult& ) > completion_handler )
  {
    completion_handler_fun_ = completion_handler;
  }

  void setExceptionHandler( std::function< void( const std::exception& ) > exception_handler )
  {
    exception_handler_fun_ = exception_handler;
  }

  void setTimeoutHandler( std::function< void( const double ) > timeout_handler )
  {
    timeout_handler_fun_ = timeout_handler;
  }

  void setTimeout( double seconds )
  {
    timeout_ = seconds;
  }

  bool doEventAndExecute( TEvent trigger, const TCommandParameter&& target )
  {
    if ( FiniteStateMachine< TEvent, TState >::doEvent( trigger ) )
    {
      updateFSM( std::move( target ) );
      return true;
    }

    return false;
  }

  bool doEventAndExecute( TEvent trigger )
  {
    if ( FiniteStateMachine< TEvent, TState >::doEvent( trigger ) )
    {
      updateFSM();
      return true;
    }

    return false;
  }

  void updateFSM( TCommandParameter&& target )
  {
    std::unique_lock< std::mutex > lock( master_command_mutex_ );
    command_         = std::move( target );
    has_new_command_ = true;
    cond_wakeup_.notify_all();
  }

  void updateFSM()
  {
    std::unique_lock< std::mutex > lock( master_command_mutex_ );
    has_new_command_ = true;
    cond_wakeup_.notify_all();
  }

 protected:
  std::function< TResult( const TCommandParameter* ) > execute_fun_            = nullptr;
  std::function< void( TResult& ) >                    completion_handler_fun_ = nullptr;
  std::function< void( const TCommandParameter* ) >    pre_exec_fun_           = nullptr;
  std::function< void( const double ) >                timeout_handler_fun_    = nullptr;
  std::function< void( const std::exception& ) >       exception_handler_fun_  = nullptr;

 private:
  /**
   * @brief Runs a state machine step when the conditional variable is kicked by the UpdateCommand function
   */
  void workerThread()
  {
    try
    {
      while ( !shutdown_desired_ )
      {
        std::unique_lock< std::mutex > lock( master_command_mutex_ );

        worker_ready_ = true;
        cond_wakeup_.wait( lock, [&]() { return has_new_command_ || shutdown_desired_; } );

        if ( has_new_command_ && execute_fun_ != nullptr )
        {
          if ( pre_exec_fun_ )
          {
            pre_exec_fun_( &command_ );
          }

          if ( !shutdown_desired_ )
          {
            TResult res = execute_fun_( &command_ );
            time_mutex_.lock();
            last_worker_response_ = TClock::toSec();
            time_mutex_.unlock();

            result_mutex_.lock();
            last_worker_result_ = std::move( res );
            result_mutex_.unlock();
            has_new_result_ = true;
          }
        }

        has_new_command_ = false;
      }
    }
    catch ( std::exception& ex )
    {
      if ( exception_handler_fun_ )
      {
        exception_handler_fun_( ex );
      }
    }
  }

  /**
   * @brief Controls the forward progression of the state machine, deals with timeouts
   */
  void watchdogThread()
  {
    bool firstpass = true;
    while ( !shutdown_desired_ )
    {
      if ( !worker_ready_ )
      {
        while ( !worker_ready_ && !shutdown_desired_ )
        {
          rate_.sleep();  // wait for spinup
        }
        has_new_result_ = true;  // initial kick
        cond_wakeup_.notify_all();
      }
      else
      {
        if ( firstpass )
        {
          has_new_result_ = true;
        }
      }

      rate_.sleep();

      // TODO: check for outside requests, interrupts ?

      // check for new result
      if ( has_new_result_ )
      {
        std::unique_lock< std::mutex > lock( result_mutex_ );
        if ( completion_handler_fun_ )
        {
          completion_handler_fun_( last_worker_result_ );
        }
        has_new_result_ = false;
      }

      // check for timeout
      if ( timeout_handler_fun_ )
      {
        std::unique_lock< std::mutex > lock( time_mutex_ );
        if ( ( TClock::toSec() - last_worker_response_ ) > timeout_ )
        {
          timeout_handler_fun_( last_worker_response_ );
        }
      }

      firstpass = false;
    }
  }

  // when recieving a new command, kick the state machine into action
  std::condition_variable cond_wakeup_;

  // track how much time the worker thread is taking during a state machine step
  std::thread watchdog_;

  // control in the worker thread
  std::thread         worker_;
  std::mutex          master_command_mutex_;
  std::atomic< bool > has_new_command_;
  std::atomic< bool > worker_ready_;

  // track time in state machine step, process timeouts on the watchdog thread
  std::mutex         time_mutex_;
  double             last_worker_response_;
  double             timeout_;
  BaseRate< TClock > rate_;

  // process the results on the watchdog thread
  std::mutex          result_mutex_;
  TResult             last_worker_result_;
  std::atomic< bool > has_new_result_;

  // the command. Candidate for std::variant
  TCommandParameter command_;

  bool shutdown_desired_;
};
}  // namespace fsm