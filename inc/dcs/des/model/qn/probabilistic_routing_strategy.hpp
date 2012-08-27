/**
 * \file src/dcs/des/model/qn/probabilistic_routing_strategy.hpp
 *
 * \brief Output strategy which behaves probabilistically.
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

#ifndef DCS_DES_MODEL_QN_BY_STATION_PROBABILISTIC_ROUTING_STRATEGY_HPP
#define DCS_DES_MODEL_QN_BY_STATION_PROBABILISTIC_ROUTING_STRATEGY_HPP


#include <boost/smart_ptr.hpp>
#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/model/qn/base_routing_strategy.hpp>
#include <dcs/math/stats/distribution/discrete.hpp>
#include <dcs/math/stats/function/rand.hpp>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>


namespace dcs { namespace des { namespace model { namespace qn {

/**
 * \brief Output strategy for a given network node. 
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename TraitsT>
class probabilistic_routing_strategy: public base_routing_strategy<TraitsT>
{
	private: typedef base_routing_strategy<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::random_generator_type random_generator_type;
	public: typedef ::boost::shared_ptr<random_generator_type> random_generator_pointer;
//	public: typedef typename base_type::class_size_type class_size_type;
	public: typedef typename base_type::class_identifier_type class_identifier_type;
//	public: typedef typename base_type::node_size_type node_size_type;
	public: typedef typename base_type::node_identifier_type node_identifier_type;
	public: typedef typename base_type::routing_destination_type routing_destination_type;
	public: typedef typename base_type::customer_pointer customer_pointer;
	private: typedef ::std::size_t size_type;
	private: typedef ::std::map<routing_destination_type,real_type> routing_destination_container;
	private: typedef ::std::map<routing_destination_type,routing_destination_container> routing_container;
	private: typedef ::std::vector<routing_destination_type> indexed_routing_destination_container;
	private: typedef ::std::map<routing_destination_type,indexed_routing_destination_container> indexed_routing_container;
	//private: typedef typename routing_container::size_type size_type;
	//private: typedef typename ::dcs::math::stats::discrete_distribution<size_type,real_type> distribution_type;
	private: typedef typename ::dcs::math::stats::discrete_distribution<real_type> distribution_type;
	private: typedef ::std::map<routing_destination_type,distribution_type> distribution_map;
//	private: typedef typename traits_type::network_type network_type;
//	public: typedef network_type* network_pointer;


	public: explicit probabilistic_routing_strategy(random_generator_pointer const& ptr_rng)
	: base_type(),
	  ptr_rng_(ptr_rng)/*,
	  ptr_net_()*/
	{
	}


	// Compiler-generated copy-constructor, copy-assignment, and destructor
	// are fine.


//	public: explicit probabilistic_routing_strategy(network_pointer const& ptr_net)
//	: base_type()/*,
//	  ptr_net_(ptr_net),
//	  nn_(ptr_net_->num_nodes()),
//	  nc_(ptr_net_->num_classes()),
//	  routes_(nn_*nc_*nn_*nc_)*/
//	{
//		// precondition: network pointer must be a valid pointer
//		DCS_ASSERT(
//			ptr_net_,
//			throw ::std::invalid_argument("[dcs::des::model::qn::probabilistic_routing_strategy::ctor] Null pointer to network.")
//		);
//	}


//	public: void network(network_pointer const& ptr_net)
//	{
//		ptr_net_ = ptr_net;
//	}


//	public: network_type const& network() const
//	{
//		return ptr_net_;
//	}


//	public: network_type& network()
//	{
//		return ptr_net_;
//	}


    public: void add_route(node_identifier_type src_node,
						   class_identifier_type src_class,
                           node_identifier_type dst_node,
                           class_identifier_type dst_class,
                           real_type p)
    {
//FIXME
//		// precondition: source node ID must be a valid node ID
//		DCS_ASSERT(
//			src_node != traits_type::invalid_node_id() && src_node < nn_,
//			throw ::std::invalid_argument("[dcs::des::model::qn::probabilistic_routing_strategy] Bad source node identifier.")
//		);
//		// precondition: destination node ID must be a valid node ID
//		DCS_ASSERT(
//			dst_node != traits_type::invalid_node_id() && dst_node < nn_,
//			throw ::std::invalid_argument("[dcs::des::model::qn::probabilistic_routing_strategy] Bad destination node identifier.")
//		);
//		// precondition: source class ID must be a valid class ID
//		DCS_ASSERT(
//			src_class != traits_type::invalid_class_id() && src_class < nc_,
//			throw ::std::invalid_argument("[dcs::des::model::qn::probabilistic_routing_strategy] Bad source class identifier.")
//		);
//		// precondition: destionation class ID must be a valid class ID
//		DCS_ASSERT(
//			dst_class != traits_type::invalid_class_id() && dst_class < nc_,
//			throw ::std::invalid_argument("[dcs::des::model::qn::probabilistic_routing_strategy] Bad destionation class identifier.")
//		);

		DCS_DEBUG_TRACE_L(3, "Adding route: <node: " << src_node << ",class: " << src_class << "> --> <node: " << dst_node << ", class: " << dst_class << ">");//XXX

		routes_[::std::make_pair(src_node, src_class)][::std::make_pair(dst_node, dst_class)] = p;

		// invalidate the distribution map
		distrs_.clear();
	}


//	private: routing_destination_type do_route(customer_pointer const& ptr_customer, ::dcs::math::random::any_generator<real_type> rng)
	private: routing_destination_type do_route(customer_pointer const& ptr_customer)
	{
		// pre: customer pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_customer );

		class_identifier_type c = ptr_customer->current_class();
		node_identifier_type n = ptr_customer->current_node();

		if (distrs_.empty())
		{
			make_distributions();
		}

		typename distribution_map::key_type key = ::std::make_pair(n, c);

		//registry<traits_type>& reg = registry<traits_type>::instance();

		size_type pos = static_cast<size_type>(
			::dcs::math::stats::rand(
				distrs_[key],
				//ptr_net_->random_generator()
				//ptr_customer->network().random_generator()
				//reg.random_generator()
				*ptr_rng_
			)
		);

		routing_destination_type dst = ord_routes_[key].at(pos);

		return dst;
	}


	private: void make_distributions()
	{
		typedef typename routing_container::const_iterator outer_iterator;
		typedef typename routing_destination_container::const_iterator inner_iterator;

		outer_iterator out_end = routes_.end();
		for (outer_iterator out_it = routes_.begin(); out_it != out_end; ++out_it)
		{
			routing_destination_type key = out_it->first;
			::std::vector<real_type> probs;
			indexed_routing_destination_container routes;

			inner_iterator inn_end = routes_[key].end();
			for (inner_iterator inn_it = routes_[key].begin(); inn_it != inn_end; ++inn_it)
			{
				probs.push_back(inn_it->second);
				routes.push_back(inn_it->first);
			}

			distrs_[key] = distribution_type(probs.begin(), probs.end());
			ord_routes_[key] = routes;
		}
	}

     
//	private: network_pointer ptr_net_;
	private: routing_container routes_;
	private: indexed_routing_container ord_routes_;
	private: distribution_map distrs_;
	private: random_generator_pointer ptr_rng_;
};

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_BY_STATION_PROBABILISTIC_ROUTING_STRATEGY_HPP
