/**
 * @file event_table_entry.hpp
 * @author Eric D. Schmidt (e1d1s1@hotmail.com)
 * @brief Harmony FSM event table structure
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