/**
 * @file config_parser.h
 * @author Eric D. Schmidt (ericdschmidt@gmail.com)
 * @brief Parse rules from config files
 * @date 2021-05-16
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

#include "event_table_entry.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

namespace fsm 
{

/**
 * @brief utility to parse state machine rules from a configuration file
 * 
 */
class EventTableParser
{
  public:
    /**
     * @brief extracts vector of EventTableEntry from a CSV file with optional header keys
     * 
     * @tparam TEvent Event type
     * @tparam TState State type
     * @param csv_filepath path to csv file
     * @param parse_header parses the header line to obtain column index, otherwise trigger,current,result is assumed. Default is true.
     * @return std::vector< EventTableEntry < TEvent, TState> > 
     */
    template < typename TEvent, typename TState>
    static std::vector< EventTableEntry < TEvent, TState> > parseCSV( const std::string& csv_filepath, bool parse_header = true )
    {
      std::vector< EventTableEntry < TEvent, TState> > retval; 
      std::ifstream csv_file( csv_filepath );
      if ( csv_file.is_open() )
      {
        EventTableEntry < TEvent, TState> rule_row;
        std::string line;
        size_t line_cnt = 0;
        unsigned trigger_idx = 0;
        unsigned current_state_idx = 1;
        unsigned resultant_state_idx = 2;

        while ( std::getline( csv_file, line ) )
        {
          const bool is_header = line_cnt == 0 && parse_header;
          std::istringstream iss(line);
          std::vector< std::string > tokens;
          std::string token;
          unsigned idx = 0;
          while ( std::getline( iss, token, ',' ) ) 
          {
            if ( is_header )
            {
              if ( token == "trigger") trigger_idx = idx;
              else if ( token == "current") current_state_idx = idx;
              else if ( token == "result") resultant_state_idx = idx;
            }
            else
            {
              if ( !token.empty() )
              {
                int ivalue = std::stoi( token ); // may throw

                if ( idx == trigger_idx) rule_row.Trigger = static_cast< TEvent >( ivalue );
                else if ( idx == current_state_idx) rule_row.Current = static_cast< TState >( ivalue );
                else if ( idx == resultant_state_idx) rule_row.Result = static_cast< TState >( ivalue );
              }        
            }
            idx++;
          }

          if ( !is_header ) 
          {
            retval.emplace_back( std::move( rule_row ) );
          }
          line_cnt++;
        }
      }

      return retval;
    }
};

}