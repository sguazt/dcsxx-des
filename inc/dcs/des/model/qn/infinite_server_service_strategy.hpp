/**
 * \file dcs/des/model/qn/infinite_server_service_strategy.hpp
 *
 * \brief Service station representing an infinite server (or delay) node.
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

#ifndef DCS_DES_MODEL_QN_INFINITE_SERVER_SERVICE_STRATEGY_HPP
#define DCS_DES_MODEL_QN_INFINITE_SERVER_SERVICE_STRATEGY_HPP


#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/model/qn/base_service_strategy.hpp>
#include <dcs/math/constants.hpp>
#include <dcs/math/stats/distribution/any_distribution.hpp>
#include <dcs/math/stats/function/rand.hpp>
#include <map>
#include <vector>


namespace dcs { namespace des { namespace model { namespace qn {

template <typename TraitsT>
class infinite_server_service_strategy: public base_service_strategy<TraitsT>
{
	private: typedef base_service_strategy<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename base_type::real_type real_type;
	public: typedef typename base_type::uint_type uint_type;
	private: typedef typename base_type::customer_type customer_type;
	public: typedef typename base_type::customer_pointer customer_pointer;
	public: typedef ::dcs::math::stats::any_distribution<real_type> distribution_type;
	private: typedef ::std::vector<distribution_type> distribution_container;
	private: typedef ::std::map<uint_type,customer_pointer> server_container;
	private: typedef typename customer_type::identifier_type customer_identifier_type;
	private: typedef typename base_type::random_generator_type random_generator_type;
	private: typedef typename traits_type::class_identifier_type class_identifier_type;
	private: typedef typename base_type::runtime_info_type runtime_info_type;


	public: infinite_server_service_strategy()
	: base_type(),
	  servers_(),
	  next_srv_(0)
	{
	}


	public: template <typename ForwardIterT>
		infinite_server_service_strategy(ForwardIterT first_distr, ForwardIterT last_distr)
	: base_type(),
	  servers_(),
	  distrs_(first_distr, last_distr),
	  next_srv_(0)
	{
	}


	public: template <typename ClassForwardIterT, typename DistrForwardIterT>
		infinite_server_service_strategy(ClassForwardIterT first_class_id, ClassForwardIterT last_class_id, DistrForwardIterT first_distr)
	: base_type(),
	  servers_(),
	  next_srv_(0)
	{
		while (first_class_id != last_class_id)
		{
			class_identifier_type class_id = *first_class_id;

			if (class_id >= distrs_.capacity())
			{
				distrs_.reserve(class_id+1);
			}

			distrs_[class_id] = *first_distr;
			++first_class_id;
			++first_distr;
		}
	}


	// Compiler-generated copy-constructor, copy-assignment, and destructor
	// are fine.


	//@{ Interface member functions

	private: void do_update_service()
	{
		// empty
	}


	private: bool do_can_serve() const
	{
		return true;
	}


	private: runtime_info_type do_serve(customer_pointer const& ptr_customer, random_generator_type& rng)
	{
		// pre: customer pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_customer );

		real_type cur_time(this->node().network().engine().simulated_time());
		real_type svc_time(0);

		typename traits_type::class_identifier_type class_id = ptr_customer->current_class();

        while ((svc_time = ::dcs::math::stats::rand(distrs_[class_id], rng)) < 0) ;

		runtime_info_type rt_info(ptr_customer, cur_time, svc_time);
		rt_info.server_id(next_srv_);
		rt_info.share(this->capacity_multiplier());

		servers_[next_srv_] = ptr_customer;
		next_srv_ = next_server(next_srv_);

		return rt_info;
	}


	private: void do_remove(customer_pointer const& ptr_customer)
	{
		// pre: customer pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_customer );

		// Retrieve the id of this customer
		customer_identifier_type cid(ptr_customer->id());

		// Retrieve the server assigned to this customer
		uint_type sid(this->info(cid).server_id());

		// Erase the associated service info 
		servers_.erase(sid);
		next_srv_ = next_server(sid);
	}


	private: void do_remove_all()
	{
		servers_.clear();
		next_srv_ = uint_type/*zero*/();
	}


	private: void do_reset()
	{
		servers_.clear();
		next_srv_ = uint_type/*zero*/();
	}


	private: uint_type do_num_servers() const
	{
		return ::dcs::math::constants::infinity<uint_type>::value;
	}


	private: uint_type do_num_busy_servers() const
	{
		return servers_.size();
	}


	//@} Interface member functions


	private: uint_type next_server(uint_type start_sid) const
	{
		typedef typename server_container::const_iterator iterator;

		uint_type best_sid(start_sid);

		// Pick the first available server-id.
		iterator end_it(servers_.end());
		for (iterator it = servers_.begin(); it != end_it && servers_.count(best_sid) > 0; ++it)
		{
			best_sid = it->first+1;
		}

		return best_sid;
	}


	//@{ Data members

	private: server_container servers_;
	private: distribution_container distrs_;
	private: uint_type next_srv_;

	//@} Data members
};

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_INFINITE_SERVER_SERVICE_STRATEGY_HPP
