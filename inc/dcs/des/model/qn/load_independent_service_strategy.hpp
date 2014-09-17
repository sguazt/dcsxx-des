/**
 * \file dcs/des/model/qn/load_independent_service_strategy.hpp
 *
 * \brief Load-independent service strategy.
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

#ifndef DCS_DES_MODEL_QN_LOAD_INDEPENDENT_SERVICE_STRATEGY_HPP
#define DCS_DES_MODEL_QN_LOAD_INDEPENDENT_SERVICE_STRATEGY_HPP


#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/model/qn/base_service_strategy.hpp>
#include <dcs/math/stats/distribution/any_distribution.hpp>
#include <dcs/math/stats/function/rand.hpp>
#include <dcs/math/traits/float.hpp>
//#include <map>
#include <vector>


namespace dcs { namespace des { namespace model { namespace qn {

template <typename TraitsT>
class load_independent_service_strategy: public base_service_strategy<TraitsT>
{
	private: typedef base_service_strategy<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename base_type::real_type real_type;
	private: typedef typename base_type::customer_type customer_type;
	public: typedef typename base_type::customer_pointer customer_pointer;
	public: typedef ::dcs::math::stats::any_distribution<real_type> distribution_type;
	private: typedef ::std::vector<distribution_type> distribution_container;
	private: typedef ::std::map<std::size_t,customer_pointer> server_container;
	private: typedef typename customer_type::identifier_type customer_identifier_type;
//	private: typedef ::std::map<customer_identifier_type,std::size_t> customer_server_map;
	private: typedef typename base_type::random_generator_type random_generator_type;
	private: typedef typename traits_type::class_identifier_type class_identifier_type;
	private: typedef typename base_type::runtime_info_type runtime_info_type;


	public: load_independent_service_strategy()
	: base_type(),
	  ns_(1),
	  servers_(),
	  num_busy_(0),
	  old_share_(0),
	  old_multiplier_(0)
	{
//		servers_.reserve(ns_);
	}


	public: template <typename ForwardIterT>
		load_independent_service_strategy(ForwardIterT first_distr, ForwardIterT last_distr)
	: base_type(),
	  ns_(1),
	  servers_(),
	  distrs_(first_distr, last_distr),
	  num_busy_(0),
	  old_share_(0),
	  old_multiplier_(0)
	{
//		servers_.reserve(ns_);
	}


	public: template <typename ClassForwardIterT, typename DistrForwardIterT>
		load_independent_service_strategy(ClassForwardIterT first_class_id, ClassForwardIterT last_class_id, DistrForwardIterT first_distr)
	: base_type(),
	  ns_(1),
	  servers_(),
	  num_busy_(0),
	  old_share_(0),
	  old_multiplier_(0)
	{
//		servers_.reserve(ns_);

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


	public: template <typename ForwardIterT>
		load_independent_service_strategy(::std::size_t num_servers, ForwardIterT first_distr, ForwardIterT last_distr)
	: base_type(),
	  ns_(num_servers),
	  servers_(),
	  num_busy_(0),
	  old_share_(0),
	  old_multiplier_(0)
	{
//		servers_.reserve(ns_);

		while (first_distr != last_distr)
		{
			distrs_.push_back(*first_distr);
			++first_distr;
		}
	}


	public: template <typename ClassForwardIterT, typename DistrForwardIterT>
		load_independent_service_strategy(::std::size_t num_servers, ClassForwardIterT first_class_id, ClassForwardIterT last_class_id, DistrForwardIterT first_distr)
	: base_type(),
	  ns_(num_servers),
	  servers_(),
	  num_busy_(0),
	  old_share_(0),
	  old_multiplier_(0)
	{
//		servers_.reserve(ns_);

		while (first_class_id != last_class_id)
		{
			if (*first_class_id >= distrs_.capacity())
			{
				distrs_.reserve(*first_class_id+1);
			}

			distrs_[*first_class_id] = *first_distr;
			++first_class_id;
			++first_distr;
		}
	}


	// Compiler-generated copy-constructor, copy-assignment, and destructor
	// are fine.


//	private: real_type common_share() const
//	{
//		//return this->capacity_multiplier()/static_cast<real_type>(ns_);
//		return this->capacity_multiplier();
//	}


	//@{ Interface member functions

	//TODO: Test-Me!!
	private: void do_update_service()
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Do-Update-Service (Clock: " << this->node().network().engine().simulated_time() << ")");//XXX

		// Check if there is at least one busy server.
		if (num_busy_ > 0)
		{
			typedef typename server_container::const_iterator iterator;

			real_type new_share(this->share());
			real_type new_multiplier(this->capacity_multiplier());

			// Check if we really need to update currently running customers
			if (::dcs::math::float_traits<real_type>::approximately_equal(old_share_, new_share)
				&& ::dcs::math::float_traits<real_type>::approximately_equal(old_multiplier_, new_multiplier))
			{
				// Share is not changed -> avoid to update customers and reschedule their end-of-service events

				DCS_DEBUG_TRACE_L(3, "(" << this << ") Share/Multiplier not changed: " << old_share_ << " vs. " << new_share << " / " << old_multiplier_ << " vs. " << new_multiplier);

				return;
			}

			real_type cur_time(this->node().network().engine().simulated_time());
			iterator end_it(servers_.end());
			for (iterator it = servers_.begin(); it != end_it; ++it)
			{
				std::size_t sid(it->first);
				customer_pointer ptr_customer(it->second);

				// paranoid-check: null
				DCS_DEBUG_ASSERT( ptr_customer );

				DCS_DEBUG_TRACE_L(3, "(" << this << ") Running Customer ID: " << ptr_customer->id());

				runtime_info_type& rt_info(this->info(ptr_customer));

				// paranoid-check: paranoid check
				DCS_DEBUG_ASSERT( rt_info.server_id() == sid );

				DCS_DEBUG_TRACE_L(3, "Updating Customer: " << rt_info.get_customer() << " - Service demand: " << rt_info.service_demand() << " - Multiplier: " << this->capacity_multiplier() << " - old share: " << rt_info.share() << " - old runtime: " << rt_info.runtime() << " - old completed work: " << rt_info.completed_work() << " - old residual-work: " << rt_info.residual_work() << " (Clock: " << this->node().network().engine().simulated_time() << ")");//XXX

				// Increment the accumulated work done by this customer to date...
				rt_info.accumulate_work(cur_time);
				// ... Update the capacity multiplier,...
				rt_info.capacity_multiplier(new_multiplier);
				// ... Update the share,...
				rt_info.share(new_share);
				// ... Compute the new residual work
				real_type new_residual_time(rt_info.residual_work()/new_multiplier);
				// ... And reschedule the end-of-service
				this->node().reschedule_service(*ptr_customer, new_residual_time);

				DCS_DEBUG_TRACE_L(3, "Updated Customer: " << rt_info.get_customer() << " - Service demand: " << rt_info.service_demand() << " - Multiplier: " << this->capacity_multiplier() << " - new share: " << rt_info.share() << " - new runtime: " << rt_info.runtime() << " - new completed work: " << rt_info.completed_work() << " - new residual-work: " << rt_info.residual_work() << " (Clock: " << this->node().network().engine().simulated_time() << ")");//XXX
			}

			old_share_ = new_share;
			old_multiplier_ = new_multiplier;
		}

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Do-Update-Service (Clock: " << this->node().network().engine().simulated_time() << ")");//XXX
	}


//	private: bool do_can_serve(customer_pointer const& ptr_customer) const
//	{
//		return servers_.size() < num_busy_;
//	}


	private: bool do_can_serve() const
	{
		//return servers_.size() > num_busy_;
		return ns_ > num_busy_;
	}


	//private: real_type do_serve(customer_pointer const& ptr_customer, random_generator_type& rng)
	private: runtime_info_type do_serve(customer_pointer const& ptr_customer, random_generator_type& rng)
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Service");//XXX

		// pre: make sure to not exceed the max # customer that can be served
		DCS_DEBUG_ASSERT( num_busy_ < ns_ );

		// pre: customer pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_customer );

		real_type cur_time(this->node().network().engine().simulated_time());
		real_type svc_time(0);

		typename traits_type::class_identifier_type class_id = ptr_customer->current_class();

        while ((svc_time = ::dcs::math::stats::rand(distrs_[class_id], rng)) < 0) ;

//		svc_time /= this->capacity_multiplier();

		runtime_info_type rt_info(ptr_customer, cur_time, svc_time);
		rt_info.server_id(num_busy_);
		rt_info.share(this->share());
		rt_info.capacity_multiplier(this->capacity_multiplier());

		servers_[num_busy_] = ptr_customer;
//		customers_servers_[ptr_customer->id()] = num_busy_;

		++num_busy_;

		DCS_DEBUG_TRACE_L(3, "(" << this << ") Generated service for customer: " << *ptr_customer << " - Service Demand: " << rt_info.service_demand() << " --> Runtime: " << svc_time);//XXX

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Service");//XXX

		return rt_info;
	}


	private: void do_remove(customer_pointer const& ptr_customer)
	{
		// precondition: customer pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_customer,
			throw ::std::invalid_argument("[dcs::des::model::qn::load_independent_service_strategy::do_remove] Null pointer to a customer.")
		);
//		// precondition: customer must already be assigned for service
//		DCS_ASSERT(
//			customers_servers_.count(ptr_customer->id()) != 0,
//			throw ::std::logic_error("[dcs::des::model::qn::load_independent_service_strategy::do_remove] Customer not in service.")
//		);

		// Retrieve the id of this customer
		customer_identifier_type cid(ptr_customer->id());

		// Retrieve the server assigned to this customer
//		std::size_t s_id = customers_servers_[c_id];
		std::size_t sid(this->info(cid).server_id());

		// Erase the associated service info 
		servers_.erase(sid);
//		servers_[sid].reset();
		--num_busy_;
	}


	private: void do_remove_all()
	{
		servers_.clear();
//		servers_.resize(ns_);
		num_busy_ = std::size_t/*zero*/();
	}


	private: void do_reset()
	{
		servers_.clear();
//		servers_.resize(ns_);
		num_busy_ = std::size_t/*zero*/();
		old_share_ = this->share();
		old_multiplier_ = this->capacity_multiplier();
//		customers_servers_.clear();
	}


	private: std::size_t do_num_servers() const
	{
		return ns_;
	}


	private: std::size_t do_num_busy_servers() const
	{
		return num_busy_;
	}

	//@} Interface member functions


	//@{ Data members

	private: std::size_t ns_;
	private: server_container servers_;
	private: distribution_container distrs_;
	private: std::size_t num_busy_;
	private: real_type old_share_;
	private: real_type old_multiplier_;
//	private: customer_server_map customers_servers_;

	//@} Data members
};

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_LOAD_INDEPENDENT_SERVICE_STRATEGY_HPP
