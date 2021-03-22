/**
 * @file fsm_runner.hpp
 * @author Eric D. Schmidt (e1d1s1@hotmail.com)
 * @brief Harmony FSM Runner template 
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

#include <array>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <map>

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
  /**
   * @brief Construct a new Finite State Machine Runner object, single function for execution by default
   *
   * @param fsm_table Valid transition table
   * @param init_state Initial state
   * @param init_result The initial result to process when kicked by the completion_handler
   * @param frequency Frequency at which to run the watchdog timer, max speed of runner
   * @param exec_fun Single function through which all states execute
   * @param completion_handler Function through which execution results are processed. Kick into new states here
   * @param pre_exec_fun Function to run before executing any state function
   * @param timeout_handler If an execution function takes longer that timeout (see setTimeout), execute this function
   * @param exception_handler If an exception is thrown during an execution function, it will propogate to this handler
   */
  FiniteStateMachineRunner( std::vector< EventTableEntry< TEvent, TState > >     fsm_table,
                            TState                                               init_state,
                            TResult                                              init_result,
                            double                                               frequency,
                            std::function< TResult( const TCommandParameter* ) > exec_fun           = nullptr,
                            std::function< void( TResult& ) >                    completion_handler = nullptr,
                            std::function< void( const TCommandParameter* ) >    pre_exec_fun       = nullptr,
                            std::function< void( const double ) >                timeout_handler    = nullptr,
                            std::function< void( const std::exception& ) >       exception_handler  = nullptr )
    : FiniteStateMachine< TEvent, TState >( fsm_table, init_state )
    , rate_( BaseRate< TClock >( frequency ) )
    , execute_fun_( exec_fun )
    , completion_handler_fun_( completion_handler )
    , pre_exec_fun_( pre_exec_fun )
    , timeout_handler_fun_( timeout_handler )
    , exception_handler_fun_( exception_handler )
    , last_worker_result_( init_result )
  {
    init();
  }

  /**
   * @brief Construct a new Finite State Machine Runner object, single function for execution by default
   *
   * @param fsm_table Valid transition table
   * @param init_state Initial state
   * @param init_result The initial result to process when kicked by the completion_handler
   * @param frequency Frequency at which to run the watchdog timer, max speed of runner
   * @param exec_fun_map A map of TState vs functions to execute for each state
   * @param completion_handler Function through which execution results are processed. Kick into new states here
   * @param pre_exec_fun Function to run before executing any state function
   * @param timeout_handler If an execution function takes longer that timeout (see setTimeout), execute this function
   * @param exception_handler If an exception is thrown during an execution function, it will propogate to this handler
   */
  FiniteStateMachineRunner( std::vector< EventTableEntry< TEvent, TState > >                         fsm_table,
                            TState                                                                   init_state,
                            TResult                                                                  init_result,
                            double                                                                   frequency,
                            std::map< TState, std::function< TResult( const TCommandParameter* ) > > exec_fun_map,
                            std::function< void( TResult& ) >                                        completion_handler = nullptr,
                            std::function< void( const TCommandParameter* ) >                        pre_exec_fun       = nullptr,
                            std::function< void( const double ) >                                    timeout_handler    = nullptr,
                            std::function< void( const std::exception& ) >                           exception_handler  = nullptr )
    : FiniteStateMachine< TEvent, TState >( fsm_table, init_state )
    , rate_( BaseRate< TClock >( frequency ) )
    , execute_fun_map_( exec_fun_map )
    , completion_handler_fun_( completion_handler )
    , pre_exec_fun_( pre_exec_fun )
    , timeout_handler_fun_( timeout_handler )
    , exception_handler_fun_( exception_handler )
    , last_worker_result_( init_result )
  {
    init();
  }

  FiniteStateMachineRunner(const FiniteStateMachineRunner&) = delete;
  void operator=(const FiniteStateMachineRunner&) = delete;

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

  /**
   * @brief Starts running threads
   * 
   */
  void start()
  {
    shutdown_desired_ = false;
    has_new_command_  = false;
    has_new_result_   = false;
    worker_ready_     = false;
    worker_           = std::thread( &FiniteStateMachineRunner::workerThread, this );
    watchdog_         = std::thread( &FiniteStateMachineRunner::watchdogThread, this );
  }

  /**
   * @brief Issues stop request to threads
   * 
   */
  void stop()
  {
    shutdown_desired_ = true;
    cond_wakeup_.notify_all();
  }

  /**
   * @brief Returns whether threads are currently attempting to stop 
   * 
   * @return true Stop has been requested 
   * @return false Stop has not been requested
   */
  bool stopping()
  {
    return shutdown_desired_;
  }

  /**
   * @brief Set the execution function map. 
   * 
   * @param execute_fun_map map of functions by state
   */
  void setExecFunctionMap( std::map< TState, std::function< TResult( const TCommandParameter* ) > > execute_fun_map )
  {
    execute_fun_map_ = execute_fun_map;
  }

  /**
   * @brief Set the execution function to run in every state
   * 
   * @param execute_fun Execution function to run in every state
   */
  void setExecFunction( std::function< TResult( const TCommandParameter* ) > execute_fun )
  {
    execute_fun_ = execute_fun;
  }

  /**
   * @brief Set the function to execute prior to that state function
   * 
   * @param pre_exec_fun Function to execute prior to that state function
   */
  void setPreExecFunction( std::function< void( const TCommandParameter* ) > pre_exec_fun )
  {
    pre_exec_fun_ = pre_exec_fun;
  }

  /**
   * @brief Set the handler to process the results of execution functions
   * 
   * @param completion_handler Handler to process the results of execution functions
   */
  void setCompletionHandler( std::function< void( TResult& ) > completion_handler )
  {
    completion_handler_fun_ = completion_handler;
  }

  /**
   * @brief Set the handler to process exceptions thrown during execution functions
   * 
   * @param exception_handler Handler to process exceptions thrown during execution functions
   */
  void setExceptionHandler( std::function< void( const std::exception& ) > exception_handler )
  {
    exception_handler_fun_ = exception_handler;
  }

  /**
   * @brief Set the handler to process timeouts as defined by setTimeout
   * 
   * @param exception_handler Handler to process timeouts as defined by setTimeout
   */
  void setTimeoutHandler( std::function< void( const double ) > timeout_handler )
  {
    timeout_handler_fun_ = timeout_handler;
  }

  /**
   * @brief Set the timeout in sec before invoking the timeout handler
   * 
   * @param seconds seconds
   */
  void setTimeout( double seconds )
  {
    timeout_ = seconds;
  }

  /**
   * @brief Steps the state machine and if successfully transitioned, executes the associated function
   * 
   * @param trigger Event trigger
   * @param command Parameter object to pass to execution function
   * @return true Returns true if transition was successful
   * @return false Returns false if transition was not successful
   */
  bool doEventAndExecute( const TEvent& trigger, TCommandParameter&& command )
  {
    if ( FiniteStateMachine< TEvent, TState >::doEvent( trigger ) )
    {
      updateFSM( std::forward<TCommandParameter>( command ) );
      return true;
    }

    return false;
  }

  /**
   * @brief Steps the state machine and if successfully transitioned, executes the associated function
   * 
   * @param trigger Event trigger
   * @return true Returns true if transition was successful
   * @return false Returns false if transition was not successful
   */
  bool doEventAndExecute( const TEvent& trigger )
  {
    if ( FiniteStateMachine< TEvent, TState >::doEvent( trigger ) )
    {
      updateFSM();
      return true;
    }

    return false;
  }

  /**
   * @brief Kicks the runner into execution function and passes the given command parameters
   * 
   * @param command Parameter object to pass to execution function
   */
  void updateFSM( TCommandParameter&& command )
  {
    std::unique_lock< std::mutex > lock( master_command_mutex_ );
    command_         = std::forward<TCommandParameter>( command );
    has_new_command_ = true;
    cond_wakeup_.notify_all();
  }

  /**
   * @brief Kicks the runner into execution function
   * 
   */
  void updateFSM()
  {
    std::unique_lock< std::mutex > lock( master_command_mutex_ );
    has_new_command_ = true;
    cond_wakeup_.notify_all();
  }

 protected:
  std::function< TResult( const TCommandParameter* ) >                     execute_fun_            = nullptr;
  std::function< void( TResult& ) >                                        completion_handler_fun_ = nullptr;
  std::function< void( const TCommandParameter* ) >                        pre_exec_fun_           = nullptr;
  std::function< void( const double ) >                                    timeout_handler_fun_    = nullptr;
  std::function< void( const std::exception& ) >                           exception_handler_fun_  = nullptr;
  std::map< TState, std::function< TResult( const TCommandParameter* ) > > execute_fun_map_;

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

        auto execution_function = execute_fun_;
        if ( execution_function == nullptr )
        {
          auto it_fun = execute_fun_map_.find( FiniteStateMachine< TEvent, TState >::getCurrentState() );
          if ( it_fun != end( execute_fun_map_ ) )
          {
            execution_function = it_fun->second;
          }
        }

        if ( has_new_command_ && execution_function != nullptr )
        {
          if ( pre_exec_fun_ )
          {
            pre_exec_fun_( &command_ );
          }

          if ( !shutdown_desired_ )
          {
            TResult res = execution_function( &command_ );
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

  void init()
  {
    shutdown_desired_     = false;
    has_new_command_      = false;
    has_new_result_       = false;
    worker_ready_         = false;
    last_worker_response_ = TClock::toSec();
  }

  // when receiving a new command, kick the state machine into action
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