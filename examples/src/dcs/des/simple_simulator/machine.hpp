/**
 * \file examples/src/dcs/des/simple_simulator/machine.hpp
 *
 * \brief Machine model.
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

#ifndef DCS_EXAMPLES_DES_SIMPLE_SIMULATOR_MACHINE_HPP
#define DCS_EXAMPLES_DES_SIMPLE_SIMULATOR_MACHINE_HPP


#include <dcs/memory.hpp>
#include "task.hpp"


namespace dcs { namespace examples { namespace des { namespace simple_simulator {

/**
 * \brief Machine model.
 *
 * \author Massimo Canonico, &lt;massimo.canonico@mfn.unipmn.it&gt;
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <
	typename RealT=double,
	typename UIntT=unsigned long
>
class machine
{
	public: typedef RealT real_type;
	public: typedef UIntT uint_type;


	public: machine(uint_type id)
		: id_(id),
		  busy(false),
		  current_task()
	{
		// Empty
	}


	//pointer to the task descriptor of the running task
	private: uint_type id_;
	public: bool busy;
	public: ::dcs::shared_ptr< task<real_type,uint_type> > current_task;
};

}}}} // Namespace dcs::examples::des::simple_simulator


#endif // DCS_EXAMPLES_DES_SIMPLE_SIMULATOR_MACHINE_HPP
