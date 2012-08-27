/**
 * \file src/dcs/des/model/qn/deterministic_routing_strategy.hpp
 *
 * \brief Output strategy which behaves deterministically.
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

#ifndef DCS_DES_MODEL_QN_BY_STATION_DETERMINISTIC_ROUTING_STRATEGY_HPP
#define DCS_DES_MODEL_QN_BY_STATION_DETERMINISTIC_ROUTING_STRATEGY_HPP


#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/model/qn/base_routing_strategy.hpp>
#include <map>
#include <utility>
#include <vector>


namespace dcs { namespace des { namespace model { namespace qn {

/**
 * \brief Output strategy for a given network node. 
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename TraitsT>
class deterministic_routing_strategy: public base_routing_strategy<TraitsT>
{
	private: typedef base_routing_strategy<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename base_type::class_identifier_type class_identifier_type;
	public: typedef typename base_type::node_identifier_type node_identifier_type;
	public: typedef typename base_type::routing_destination_type routing_destination_type;
	public: typedef typename base_type::customer_pointer customer_pointer;
	private: typedef ::std::map<routing_destination_type,routing_destination_type> routing_container;


	public: deterministic_routing_strategy()
	: base_type()
	{
	}


	// Compiler-generated copy-constructor, copy-assignment, and destructor
	// are fine.


    public: void add_route(node_identifier_type src_node,
						   class_identifier_type src_class,
                           node_identifier_type dst_node,
                           class_identifier_type dst_class)
    {
//FIXME
//		// precondition: source node ID must be a valid node ID
//		DCS_ASSERT(
//			src_node != traits_type::invalid_node_id() && src_node < nn_,
//			throw ::std::invalid_argument("[dcs::des::model::qn::deterministic_routing_strategy] Bad source node identifier.")
//		);
//		// precondition: destination node ID must be a valid node ID
//		DCS_ASSERT(
//			dst_node != traits_type::invalid_node_id() && dst_node < nn_,
//			throw ::std::invalid_argument("[dcs::des::model::qn::deterministic_routing_strategy] Bad destination node identifier.")
//		);
//		// precondition: source class ID must be a valid class ID
//		DCS_ASSERT(
//			src_class != traits_type::invalid_class_id() && src_class < nc_,
//			throw ::std::invalid_argument("[dcs::des::model::qn::deterministic_routing_strategy] Bad source class identifier.")
//		);
//		// precondition: destionation class ID must be a valid class ID
//		DCS_ASSERT(
//			dst_class != traits_type::invalid_class_id() && dst_class < nc_,
//			throw ::std::invalid_argument("[dcs::des::model::qn::deterministic_routing_strategy] Bad destionation class identifier.")
//		);

		DCS_DEBUG_TRACE_L(3, "Adding route: <node: " << src_node << ",class: " << src_class << "> --> <node: " << dst_node << ", class: " << dst_class << ">");//XXX

		routes_[::std::make_pair(src_node, src_class)] = ::std::make_pair(dst_node, dst_class);
	}


	private: routing_destination_type do_route(customer_pointer const& ptr_customer)
	{
		// paranoid-check: null
		DCS_DEBUG_ASSERT( ptr_customer );

		class_identifier_type c = ptr_customer->current_class();
		node_identifier_type n = ptr_customer->current_node();

		routing_destination_type key(::std::make_pair(n, c));

		// paranoid-check: existence
		DCS_DEBUG_ASSERT( routes_.count(key) );

		return routes_.at(key);
	}


	private: routing_container routes_;
}; // deterministic_routing_strategy

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_BY_STATION_DETERMINISTIC_ROUTING_STRATEGY_HPP
