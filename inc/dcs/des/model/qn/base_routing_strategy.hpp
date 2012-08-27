/**
 * \file dcs/des/model/qn/base_routing_strategy.hpp
 *
 * \brief Base class for node routing strategies.
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

#ifndef DCS_DES_MODEL_QN_BASE_ROUTING_STRATEGY_HPP
#define DCS_DES_MODEL_QN_BASE_ROUTING_STRATEGY_HPP


#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
//#include <dcs/math/random/any_generator.hpp>
#include <stdexcept>
#include <utility>


namespace dcs { namespace des { namespace model { namespace qn {

template <typename TraitsT>
class base_routing_strategy
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::node_type node_type;
	public: typedef typename traits_type::class_identifier_type class_identifier_type;
	public: typedef typename traits_type::node_identifier_type node_identifier_type;
	public: typedef ::std::pair<node_identifier_type,class_identifier_type> routing_destination_type;
	public: typedef typename node_type::customer_pointer customer_pointer;


	public: base_routing_strategy()
	{
	}


	// Compiler-generated copy-constructor, copy-assignment.


	public: virtual ~base_routing_strategy()
	{
	}


//	public: template <typename UniformRandomGeneratorT>
//		routing_destination_type route(customer_pointer const& ptr_customer, UniformRandomGeneratorT rng)
//	{
//		return do_route(ptr_customer,
//						::dcs::math::random::make_any_generator<UniformRandomGeneratorT>(rng));
//	}


	public: routing_destination_type route(customer_pointer const& ptr_customer)
	{
		// pre: customer pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_customer,
			throw ::std::invalid_argument("[dcs::des::model::qn::base_routing_strategy::can_push] Invalid customer.")
		);

		return do_route(ptr_customer);
	}


	public: node_identifier_type node_id(routing_destination_type const& pair) const
	{
		return pair.first;
	}


	public: class_identifier_type class_id(routing_destination_type const& pair) const
	{
		return pair.second;
	}


//	private: virtual routing_destination_type do_route(customer_pointer const& ptr_customer, ::dcs::math::random::any_generator<typename traits_type::real_type> rng) = 0;
	private: virtual routing_destination_type do_route(customer_pointer const& ptr_customer) = 0;
};

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_BASE_ROUTING_STRATEGY_HPP
