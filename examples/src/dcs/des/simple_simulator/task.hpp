/**
 * \file examples/src/dcs/des/simple_simulator/task.hpp
 *
 * \brief Task model.
 *
 * Copyright (C) 2009-2010  Distributed Computing System (DCS) Group, Computer
 * Science Department - University of Piemonte Orientale, Alessandria (Italy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \author Massimo Canonico, &lt;massimo.canonico@mfn.unipmn.it&gt;
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */

#ifndef DCS_EXAMPLES_DES_SIMPLE_SIMULATOR_TASK_HPP
#define DCS_EXAMPLES_DES_SIMPLE_SIMULATOR_TASK_HPP

namespace dcs { namespace examples { namespace des { namespace simple_simulator {

/**
 * \brief Task model.
 *
 * \author Massimo Canonico, &lt;massimo.canonico@mfn.unipmn.it&gt;
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <
	typename RealT=double,
	typename UIntT=unsigned long
>
class task
{
	public: typedef RealT real_type;
	public: typedef UIntT uint_type;

    public: task(real_type arrival, real_type exec)
			: arrival_time(arrival),
			  execution_time(exec),
			  remaining_time(exec),
			  last_restart_time(0),
			  queue_time(0),
			  id_(num_tasks_)
	{
		++num_tasks_;
	}


	public: real_type arrival_time;
	public: real_type execution_time;
	//time remaining to complete its execution
	public: real_type remaining_time;
	//time of last restart after the checkpoint
	public: real_type last_restart_time;
	//time it spent waiting in the queue
	public: real_type queue_time;
	private: uint_type id_;
	private: static uint_type num_tasks_; // = 0;
};

template <typename RealT, typename UIntT>
UIntT task<RealT,UIntT>::num_tasks_ = 0;

}}}} // Namespace dcs::examples::des::simple_simulator

#endif // DCS_EXAMPLES_DES_SIMPLE_SIMULATOR_TASK_HPP
