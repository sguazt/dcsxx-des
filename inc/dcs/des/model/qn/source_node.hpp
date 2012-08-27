/**
 * \file dcs/des/model/qn/source_station.hpp
 *
 * \brief A source node.
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

#ifndef DCS_DES_MODEL_QN_SOURCE_STATION_HPP
#define DCS_DES_MODEL_QN_SOURCE_STATION_HPP


#include <boost/smart_ptr.hpp>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/model/qn/base_routing_strategy.hpp>
#include <dcs/des/model/qn/customer_class_category.hpp>
#include <dcs/des/model/qn/network_node.hpp>
#include <dcs/des/model/qn/network_node_category.hpp>
#include <dcs/functional/bind.hpp>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>


namespace dcs { namespace des { namespace model { namespace qn {

template <typename TraitsT>
class source_node: public network_node<TraitsT>
{
	private: typedef network_node<TraitsT> base_type;
	private: typedef source_node<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename base_type::identifier_type identifier_type;
	public: typedef typename traits_type::class_identifier_type class_identifier_type;
	public: typedef typename traits_type::class_type class_type;
	public: typedef typename base_type::customer_pointer customer_pointer;
	public: typedef base_routing_strategy<TraitsT> routing_strategy_type;
	public: typedef ::boost::shared_ptr<routing_strategy_type> routing_strategy_pointer;
	private: typedef typename base_type::real_type real_type;
	public: typedef ::std::set<class_identifier_type> class_container;
	private: typedef typename traits_type::engine_type engine_type;
	private: typedef typename engine_traits<engine_type>::event_type event_type;
	private: typedef typename engine_traits<engine_type>::engine_context_type engine_context_type;


	/// A constructor.
	public: source_node(identifier_type id,
						::std::string const& name,
						//network_pointer const&  ptr_net,
						routing_strategy_pointer const& ptr_output)
	: base_type(id, name/*, ptr_net*/),
	  ptr_route_(ptr_output)
	{
		/// precondition: pointer to output strategy must be a valid pointer.
		DCS_ASSERT(
			ptr_route_,
			throw ::std::invalid_argument("[dcs::des::model::qn::source_node::ctor] Network not specified.")
		);
	}


//	public: source_node(::std::string const& name,
//						routing_strategy_pointer const& ptr_output)
//	: base_type(name),
//	  ptr_route_(ptr_output)
//	{
//	}


	// Compiler-generated copy-constructor, copy-assignment, and destructor
	// are fine.


	/// Add a classs for which this node is the customer generator.
	public: void add_class(class_type const& c)
	{
		if (c.category() != open_customer_class_category)
		{
			::std::invalid_argument("[dcs::des::model::qn::source_node::add_class] Customer class must be open.");
		}

		classes_.insert(c.id());
	}


	/// Return the category for this node.
	private: network_node_category do_category() const
	{
		return source_node_category;
	}


	/// Process the arrival of a new customer
	private: void do_process_arrival(customer_pointer const& ptr_customer, engine_context_type& ctx)
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Do Processing ARRIVAL at Node: " << *this << " of Customer: " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX

		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// precondition: customer pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_customer );

		// precondition: customer be belong to an open class
		DCS_DEBUG_ASSERT(
			this->network().get_class(ptr_customer->current_class()).category()
			==
			open_customer_class_category
		);

		// Notify the departure of the customer from this node
		this->schedule_departure(ptr_customer, real_type/*zero*/());

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Do Processing ARRIVAL at Node: " << *this << " of Customer: " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX
	}


	/// Process the departure of a new customer
	private: void do_process_departure(customer_pointer const& ptr_customer, engine_context_type& ctx)
	{
		// precondition: customer pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_customer );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Do Processing DEPARTURE at Node: " << *this << " of Customer: " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX

		// Change the current node of the given customer
		ptr_customer->change_node(this->id());

		// Set-up arrival time
		real_type iatime(0);
		iatime = ptr_customer->arrival_time();
		ptr_customer->arrival_time(iatime + ctx.simulated_time());

		// Directly forward customer to a node

		// Select the target node
		typedef typename routing_strategy_type::routing_destination_type routing_destination_type;
		routing_destination_type route_pair = ptr_route_->route(ptr_customer);
//		typedef typename traits_type::random_generator_type random_generator_type;
//		routing_destination_type route_pair = ptr_route_->template route<random_generator_type&>(ptr_customer, this->network_ptr()->random_generator());
		class_identifier_type class_id = ptr_route_->class_id(route_pair);
		identifier_type node_id = ptr_route_->node_id(route_pair);

		// Change the current class of the given customer
		ptr_customer->change_class(class_id);

		// Notify the arrival of the customer into the network
		this->network().schedule_arrival(ptr_customer, iatime);

		DCS_DEBUG_TRACE_L(3, "Sending Customer " << *ptr_customer << " to Node: " << this->network().get_node(node_id));//XXX

		// Send this customer to the target node
		this->network().get_node(node_id).receive(ptr_customer, iatime);

		// Generate a new customer arrival (with the same class of the given customer)
		this->generate(ptr_customer->current_class(), iatime);

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Do Processing DEPARTURE at Node: " << *this << " of Customer: " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX
	}


	private: real_type do_busy_time() const
	{
		return ::std::numeric_limits<real_type>::quiet_NaN();
	}


	/// Generate a new customer for the given class.
	private: void generate(class_identifier_type class_id, real_type delay)
	{
		customer_pointer ptr_customer = this->make_customer(class_id);

		// check: customer pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_customer );

		this->receive(ptr_customer, delay);
	}


	/// Create a new customer for the given class.
	private: customer_pointer make_customer(class_identifier_type class_id)
	{
		class_type const& ref_class = this->network().get_class(class_id);
		customer_pointer ptr_customer = ref_class.make_customer();

		return ptr_customer;
	}


	/// Container for classes for which this node is the source.
	private: class_container classes_;
	/// Pointer to the routing strategy.
	private: routing_strategy_pointer ptr_route_;
};

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_SOURCE_STATION_HPP
