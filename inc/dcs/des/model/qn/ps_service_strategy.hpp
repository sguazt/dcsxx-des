/**
 * \file dcs/des/model/qn/ps_service_strategy.hpp
 *
 * \brief State-dependent processor sharing service strategy.
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

#ifndef DCS_DES_MODEL_QN_PS_SERVICE_STRATEGY_HPP
#define DCS_DES_MODEL_QN_PS_SERVICE_STRATEGY_HPP


#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/model/qn/base_service_strategy.hpp>
#include <dcs/math/stats/distribution/any_distribution.hpp>
#include <dcs/math/stats/function/rand.hpp>
#include <set>
#include <vector>


namespace dcs { namespace des { namespace model { namespace qn {

/**
 * \brief Processor sharing service strategy.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename TraitsT>
class ps_service_strategy: public base_service_strategy<TraitsT>
{
	private: typedef base_service_strategy<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename base_type::real_type real_type;
	private: typedef typename base_type::customer_type customer_type;
	public: typedef typename base_type::customer_pointer customer_pointer;
	public: typedef ::dcs::math::stats::any_distribution<real_type> distribution_type;
	private: typedef ::std::vector<distribution_type> distribution_container;
	private: typedef typename customer_type::identifier_type customer_identifier_type;
	private: typedef ::std::set<customer_identifier_type> customer_set;
	private: typedef ::std::vector<customer_set> server_container;
	private: typedef typename base_type::random_generator_type random_generator_type;
	private: typedef typename traits_type::class_identifier_type class_identifier_type;
	private: typedef typename base_type::runtime_info_type runtime_info_type;


	public: ps_service_strategy()
	: base_type(),
	  ns_(1),
	  servers_(ns_),
	  num_busy_(0),
	  next_srv_(0)
	{
	}


	public: template <typename ForwardIterT>
		ps_service_strategy(ForwardIterT first_distr, ForwardIterT last_distr)
	: base_type(),
	  ns_(1),
	  servers_(ns_),
	  distrs_(first_distr, last_distr),
	  num_busy_(0),
	  next_srv_(0)
	{
	}


	public: template <typename ClassForwardIterT, typename DistrForwardIterT>
		ps_service_strategy(ClassForwardIterT first_class_id, ClassForwardIterT last_class_id, DistrForwardIterT first_distr)
	: base_type(),
	  ns_(1),
	  servers_(ns_),
	  num_busy_(0),
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


	public: template <typename ForwardIterT>
		ps_service_strategy(::std::size_t num_servers, ForwardIterT first_distr, ForwardIterT last_distr)
	: base_type(),
	  ns_(num_servers),
	  servers_(ns_),
	  num_busy_(0),
	  next_srv_(0)
	{
		while (first_distr != last_distr)
		{
			distrs_.push_back(*first_distr);
			++first_distr;
		}
	}


	public: template <typename ClassForwardIterT, typename DistrForwardIterT>
		ps_service_strategy(::std::size_t num_servers, ClassForwardIterT first_class_id, ClassForwardIterT last_class_id, DistrForwardIterT first_distr)
	: base_type(),
	  ns_(num_servers),
	  servers_(ns_),
	  num_busy_(0),
	  next_srv_(0)
	{
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


	private: real_type common_share() const
	{
		//return this->capacity_multiplier()/static_cast<real_type>(ns_);
		return this->capacity_multiplier();
	}


	//@{ Interface member functions

//	private: bool do_can_serve(customer_pointer const& ptr_customer) const
//	{
//		return servers_.size() < num_busy_;
//	}


	private: void do_update_service()
	{
		typedef typename server_container::const_iterator server_iterator;
		typedef typename customer_set::const_iterator customer_iterator;

		real_type cur_time(this->node().network().engine().simulated_time());

		server_iterator srv_end_it(servers_.end());
		for (server_iterator srv_it = servers_.begin(); srv_it != srv_end_it; ++srv_it)
		{
			customer_set const& customers(*srv_it);

			std::size_t nc(customers.size());
			real_type share(this->common_share()/static_cast<real_type>(nc));
			customer_iterator cust_end_it(customers.end());
			for (customer_iterator cust_it = customers.begin(); cust_it != cust_end_it; ++cust_it)
			{
				runtime_info_type& rt_info(this->info(*cust_it));

				DCS_DEBUG_TRACE_L(3, "Updating Customer: " << rt_info.get_customer() << " - Service demand: " << rt_info.service_demand() << " - Multiplier: " << this->capacity_multiplier() << " - old share: " << rt_info.share() << " - old runtime: " << rt_info.runtime() << " - old completed work: " << rt_info.completed_work() << " - old residual-work: " << rt_info.residual_work());//XXX

				// Increment the residual runtime of this customer by a factor of nc
				rt_info.accumulate_work(cur_time);
				rt_info.share(share);
				// And reschedule the end-of-service
				this->node().reschedule_service(rt_info.get_customer(), rt_info.residual_work()/share);

				DCS_DEBUG_TRACE_L(3, "Updated Customer: " << rt_info.get_customer() << " - Service demand: " << rt_info.service_demand() << " - Multiplier: " << this->capacity_multiplier() << " - new share: " << rt_info.share() << " - new runtime: " << rt_info.runtime() << " - new completed work: " << rt_info.completed_work() << " - new residual-work: " << rt_info.residual_work());//XXX
			}
		}
	}


	private: bool do_can_serve() const
	{
		return true;
	}


	private: runtime_info_type do_serve(customer_pointer const& ptr_customer, random_generator_type& rng)
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Do-Service of Customer: " << *ptr_customer);//XXX

		// pre: customer pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_customer );

		real_type cur_time(this->node().network().engine().simulated_time());
		real_type svc_time(0);
		real_type share(this->common_share());

		typename traits_type::class_identifier_type class_id = ptr_customer->current_class();

        while ((svc_time = ::dcs::math::stats::rand(distrs_[class_id], rng)) < 0) ;

		if (num_busy_ < ns_)
		{
			// The new customer get a dedicated server.
			// Processor-Sharing strategy still does not apply here.

			++num_busy_;
		}
		else
		{
			// The new customer has found all servers busy and hence it has to
			// share a server with other (already running) customers.
			// We need to apply Processor-Sharing strategy and increment the
			// (residual) service time of each customer running on this server
			// by a factor of n, where n is the number of customer (included
			// the last arrived) running on this server.

			std::size_t nc(servers_[next_srv_].size()+1); // +1 ... to take into consideration the just inserted customer

			share /= static_cast<real_type>(nc);

			typedef typename customer_set::const_iterator customer_iterator;

			customer_iterator end_it(servers_[next_srv_].end());
			for (customer_iterator it = servers_[next_srv_].begin(); it != end_it; ++it)
			{
				runtime_info_type& rt_info(this->info(*it));

				DCS_DEBUG_TRACE_L(3, "Updating Customer: " << rt_info.get_customer() << " - Service demand: " << rt_info.service_demand() << " - Multiplier: " << this->capacity_multiplier() << " - old share: " << rt_info.share() << " - old runtime: " << rt_info.runtime() << " - old completed work: " << rt_info.completed_work() << " - old residual-work: " << rt_info.residual_work());//XXX

				// Increment the residual runtime of this customer by a factor of nc
				rt_info.accumulate_work(cur_time);
				rt_info.share(share);
				// And reschedule the end-of-service
				this->node().reschedule_service(rt_info.get_customer(), rt_info.residual_work()/share);

				DCS_DEBUG_TRACE_L(3, "Updated Customer: " << rt_info.get_customer() << " - Service demand: " << rt_info.service_demand() << " - Multiplier: " << this->capacity_multiplier() << " - new share: " << rt_info.share() << " - new runtime: " << rt_info.runtime() << " - new completed work: " << rt_info.completed_work() << " - new residual-work: " << rt_info.residual_work());//XXX
			}
		}

//		svc_time /= this->capacity_multiplier();

		runtime_info_type rt_info(ptr_customer, cur_time, svc_time);
		rt_info.server_id(next_srv_);
		rt_info.share(share);

		servers_[next_srv_].insert(ptr_customer->id());

		next_srv_ = next_server(next_srv_);

		DCS_DEBUG_TRACE_L(3, "(" << this << ") Generated service for customer: " << *ptr_customer << " - Service demand: " << rt_info.service_demand() << " - Multiplier: " << this->capacity_multiplier() << " - Share: " << share << " - Runtime: " << rt_info.runtime() << " - Server: " << next_srv_);//XXX

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Do-Service of Customer: " << *ptr_customer);//XXX

		return rt_info;
	}


	private: void do_remove(customer_pointer const& ptr_customer)
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Do-Remove of Customer: " << *ptr_customer);//XXX

		// precondition: customer pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_customer,
			throw ::std::invalid_argument("[dcs::des::model::qn::ps_service_strategy::do_remove] Null pointer to a customer.")
		);

		// Retrieve the id of this customer
		customer_identifier_type cid(ptr_customer->id());

		// Retrieve the server assigned to this customer
		std::size_t sid(this->info(cid).server_id());

		// Erase the associated service info 
		servers_[sid].erase(cid);
		if (servers_[sid].size() == 0)
		{
			--num_busy_;
		}
		else
		{
			std::size_t nc(servers_[sid].size()+1); // +1 ... to take into consideration the just removed customer
			real_type share(this->common_share()/static_cast<real_type>(nc-1));

			typedef typename customer_set::iterator customer_iterator;

			customer_iterator end_it(servers_[sid].end());
			real_type cur_time(this->node().network().engine().simulated_time());
			for (customer_iterator it = servers_[sid].begin(); it != end_it; ++it)
			{
				runtime_info_type& rt_info(this->info(*it));

				DCS_DEBUG_TRACE_L(3, "Updating Customer: " << rt_info.get_customer() << " - Service demand: " << rt_info.service_demand() << " - Multiplier: " << this->capacity_multiplier() << " - old share: " << rt_info.share() << " - runtime: " << rt_info.runtime() << " - old completed work: " << rt_info.completed_work() << " - old residual-work: " << rt_info.residual_work());//XXX

				// Decrement the residual runtime of this customer by a factor of 1/nc
				rt_info.accumulate_work(cur_time);
				rt_info.share(share);

				// And reschedule the end-of-service
				this->node().reschedule_service(rt_info.get_customer(), rt_info.residual_work()/share);

				DCS_DEBUG_TRACE_L(3, "Updated Customer: " << rt_info.get_customer() << " - Service demand: " << rt_info.service_demand() << " - Multiplier: " << this->capacity_multiplier() << " - new share: " << rt_info.share() << " - runtime: " << rt_info.runtime() << " - new completed work: " << rt_info.completed_work() << " - new residual-work: " << rt_info.residual_work());//XXX
			}
		}

		next_srv_ = next_server(sid);

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Do-Remove of Customer: " << *ptr_customer);//XXX
	}


	private: void do_remove_all()
	{
		servers_.clear();
		servers_.resize(ns_);
		num_busy_ = next_srv_
				  = std::size_t/*zero*/();
	}


	private: void do_reset()
	{
		servers_.clear();
		servers_.resize(ns_);
		num_busy_ = next_srv_
				  = std::size_t/*zero*/();
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


	private: std::size_t next_server(std::size_t start_sid) const
	{
		std::size_t best_sid(start_sid);

		if (ns_ > 1 && servers_[start_sid].size() > 0)
		{
			// choose the server with the smallest number of served customers
			std::size_t best_sid_size(servers_[best_sid].size());
			for (std::size_t i = 1; i < ns_ && best_sid_size > 0; ++i)
			{
				std::size_t sid((start_sid+i) % ns_);
				if (servers_[sid].size() < servers_[best_sid].size())
				{
					best_sid = sid;
					best_sid_size = servers_[best_sid].size();
				}
			}
		}

		return best_sid;
	}


	//@{ Data members

	/// The total number of servers.
	private: std::size_t ns_;
	/// The servers container. For each server, it maintains the list of customers currently running on it.
	private: server_container servers_;
	/// The service distributions container.
	private: distribution_container distrs_;
	/// The number of current busy severs.
	private: std::size_t num_busy_;
	/// The next server used to assign a new customer.
	private: std::size_t next_srv_;

	//@} Data members
};

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_PS_SERVICE_STRATEGY_HPP
