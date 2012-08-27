/**
 * \file examples/src/dcs/des/simple_simulator/main.cpp
 *
 * \brief Application entry-point for the Simple Simulator example.
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

#include <iostream>
#include "system.hpp"


/**
 * \brief Application entry-point for the Simple Simulator example.
 */
int main()
{
	typedef double real_type;
	typedef unsigned long uint_type;

	dcs::examples::des::simple_simulator::system<real_type, uint_type> sys;

	sys.average_task_completion_time(0.025, 0.95, uint_type(1e+5));
	sys.average_task_waiting_time(0.025, 0.95, uint_type(1e+5));

	sys.simulate();

	std::cout << "Simulation Output:" << std::endl;
	std::cout << "Average Task Completion Time: "
			  << sys.average_task_completion_time()
			  << std::endl;
	std::cout << "Average Task Waiting Time: "
			  << sys.average_task_waiting_time()
			  << std::endl;
}
