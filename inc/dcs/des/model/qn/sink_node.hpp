/**
 * \file dcs/des/model/qn/sink_node.hpp
 *
 * \brief A sink node.
 *
 * Copyright (C) 2009-2012  Distributed Computing System (DCS) Group,
 *                          Computer Science Institute,
 *                          Department of Science and Technological Innovation,
 *                          University of Piemonte Orientale,
 *                          Alessandria (Italy).
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
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_DES_MODEL_QN_SINK_NODE_HPP
#define DCS_DES_MODEL_QN_SINK_NODE_HPP


#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/model/qn/network_node.hpp>
#include <dcs/des/model/qn/network_node_category.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <limits>
#include <string>


namespace dcs { namespace des { namespace model { namespace qn {


template <typename TraitsT>
class sink_node: public network_node<TraitsT>
{
	private: typedef network_node<TraitsT> base_type;
	private: typedef sink_node<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename base_type::identifier_type identifier_type;
	private: typedef typename traits_type::real_type real_type;
	private: typedef typename traits_type::engine_type engine_type;
	private: typedef typename traits_type::customer_type customer_type;
	private: typedef typename engine_traits<engine_type>::engine_context_type engine_context_type;
	public: typedef typename base_type::customer_pointer customer_pointer;


	public: sink_node(identifier_type id, ::std::string const& name)
	: base_type(id, name)
	{
	}


	// Compuler-generated copy-constructor, copy-assignment, and destructor
	// are fine.


	private: network_node_category do_category() const
	{
		return sink_node_category;
	}


	private: void do_process_arrival(customer_pointer const& ptr_customer, engine_context_type& ctx)
	{
		// precondition: customer pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_customer );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Do Processing ARRIVAL at Node: " << *this << " of Customer: " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX

		ptr_customer->status(customer_type::died_status);
		ptr_customer->change_node(this->id());
		ptr_customer->departure_time(ctx.simulated_time());

		this->schedule_departure(ptr_customer, real_type/*zero*/());

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Do Processing ARRIVAL at Node: " << *this << " of Customer: " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX
	}


	private: void do_process_departure(customer_pointer const& ptr_customer, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// precondition: customer pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_customer );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Do Processing DEPARTURE at Node: " << *this << " of Customer: " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX

		this->network().schedule_departure(ptr_customer, real_type/*zero*/());

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Do Processing DEPARTURE at Node: " << *this << " of Customer: " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX
	}


	private: real_type do_busy_time() const
	{
		return ::std::numeric_limits<real_type>::quiet_NaN();
	}
};

}}}} // Namespace dcs::des::model::qn

#endif // DCS_DES_MODEL_QN_SINK_NODE_HPP
