/**
 * \file dcs/des/model/qn/base_service_strategy.hpp
 *
 * \brief Base class for node service strategies.
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

#ifndef DCS_DES_MODEL_QN_BASE_SERVICE_STRATEGY_HPP
#define DCS_DES_MODEL_QN_BASE_SERVICE_STRATEGY_HPP


#include <boost/smart_ptr.hpp>
#include <cstddef>
#include <dcs/debug.hpp>
#include <dcs/des/model/qn/queueing_network_traits.hpp>
#include <dcs/des/model/qn/runtime_info.hpp>
#include <map>
#include <stdexcept>
#include <vector>


namespace dcs { namespace des { namespace model { namespace qn {

template <typename TraitsT>
class service_station_node;


template <typename TraitsT>
class base_service_strategy
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::random_generator_type random_generator_type;
	public: typedef typename traits_type::customer_type customer_type;
	public: typedef ::boost::shared_ptr<customer_type> customer_pointer;
//	public: typedef runtime_info<real_type> runtime_info_type;
	public: typedef runtime_info<traits_type> runtime_info_type;
	public: typedef ::boost::shared_ptr<runtime_info_type> runtime_info_pointer;
	public: typedef service_station_node<traits_type> service_node_type;
	public: typedef service_node_type* service_node_pointer;
	private: typedef typename customer_type::identifier_type customer_identifier_type;
	private: typedef ::std::map<customer_identifier_type,runtime_info_pointer> runtime_info_map;


	public: base_service_strategy()
	: multiplier_(1),
	  share_(1),
	  rt_infos_(),
	  ptr_node_(0),
	  busy_time_(0),
	  last_state_update_time_(0)
	{
	}


	public: virtual ~base_service_strategy()
	{
	}


	// Compiler-generated copy-constructor and copy-assignment are fine.


//	public: bool can_serve(customer_pointer const& ptr_customer) const
//	{
//		return do_can_serve(ptr_customer);
//	}


	public: void node(service_node_pointer ptr_node)
	{
		ptr_node_ = ptr_node;
	}


	public: service_node_type& node()
	{
		return *ptr_node_;
	}


	public: service_node_type const& node() const
	{
		return *ptr_node_;
	}


	public: void share(real_type val)
	{
		update_state();

		share_ = val;

		do_update_service();
	}


	public: real_type share() const
	{
		return share_;
	}


	public: void capacity_multiplier(real_type m)
	{
		update_state();

		multiplier_ = m;

		do_update_service();
	}


	public: real_type capacity_multiplier() const
	{
		return multiplier_;
	}


	public: bool can_serve() const
	{
		return do_can_serve();
	}


	public: runtime_info_type serve(customer_pointer const& ptr_customer, random_generator_type& rng)
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Serving of Customer: " << *ptr_customer << ".");///XXX

		// pre: customer pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_customer,
			throw ::std::invalid_argument("[dcs::des::model::qn::base_service_strategy::serve] Invalid customer.")
		);

		update_state();

//		real_type runtime = do_serve(ptr_customer, rng);
//
//		// Apply current service share
//		runtime /= multiplier_;
//
//		runtime_info_type rt_info(runtime);
//		rt_info.customer_id(ptr_customer->id());
		runtime_info_type rt_info = do_serve(ptr_customer, rng);
		rt_infos_[ptr_customer->id()] = ::boost::make_shared<runtime_info_type>(rt_info);

		DCS_DEBUG_TRACE_L(3, "Generated new service time: Service Demand: " << rt_info.service_demand() << " --> Runtime: " << rt_info.runtime());//XXX

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Serving of Customer: " << *ptr_customer << ".");///XXX

		return rt_info;
	}


	public: void remove(customer_pointer const& ptr_customer)
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Removal of Customer: " << *ptr_customer << ".");///XXX

		// pre: customer pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_customer,
			throw ::std::invalid_argument("[dcs::des::model::qn::base_service_strategy::remove] Invalid customer.")
		);

		update_state();

		do_remove(ptr_customer);

		rt_infos_.erase(ptr_customer->id());

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Removal of Customer: " << *ptr_customer << ".");///XXX
	}


	public: void remove_all()
	{
		update_state();

		do_remove_all();

		rt_infos_.clear();
	}


	public: runtime_info_type& info(customer_identifier_type id)
	{
		// pre: customer must have already been inserted
		DCS_ASSERT(
			rt_infos_.count(id),
			throw ::std::invalid_argument("[dcs::des::model::qn::base_service_strategy::info] Runtime information not found for customer.")
		);

		return *(rt_infos_.at(id));
	}


	public: runtime_info_type& info(customer_type const& customer)
	{
		// pre: customer pointer must be a valid pointer.
		DCS_ASSERT(
			rt_infos_.count(customer.id()),
			throw ::std::invalid_argument("[dcs::des::model::qn::base_service_strategy::info] Invalid customer.")
		);

		return info(customer.id());
	}


	public: runtime_info_type& info(customer_pointer const& ptr_customer)
	{
		// pre: customer pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_customer,
			throw ::std::invalid_argument("[dcs::des::model::qn::base_service_strategy::info] Invalid customer.")
		);

		return info(ptr_customer->id());
	}


	public: runtime_info_type const& info(customer_identifier_type id) const
	{
		// pre: customer must have already been inserted
		DCS_ASSERT(
			rt_infos_.count(id),
			throw ::std::invalid_argument("[dcs::des::model::qn::base_service_strategy::info] Runtime information not found for customer.")
		);

		return *(rt_infos_.at(id));
	}


	public: runtime_info_type const& info(customer_type const& customer) const
	{
		// pre: customer pointer must be a valid pointer.
		DCS_ASSERT(
			rt_infos_.count(customer.id()),
			throw ::std::invalid_argument("[dcs::des::model::qn::base_service_strategy::info] Invalid customer.")
		);

		return info(customer.id());
	}


	public: runtime_info_type const& info(customer_pointer const& ptr_customer) const
	{
		// pre: customer pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_customer,
			throw ::std::invalid_argument("[dcs::des::model::qn::base_service_strategy::info] Invalid customer.")
		);

		return info(ptr_customer->id());
	}


//	public: real_type suspend(customer_pointer const& ptr_customer)
//	{
//		return do_suspend(ptr_customer);
//	}


//	public: real_type resume(customer_pointer const& ptr_customer)
//	{
//		return do_resume(ptr_customer);
//	}


	public: ::std::vector<runtime_info_type> info() const
	{
		typedef typename runtime_info_map::const_iterator iterator;

		::std::vector<runtime_info_type> res;

		iterator end_it(rt_infos_.end());
		for (iterator it = rt_infos_.begin(); it != end_it; ++it)
		{
			res.push_back(*(it->second));
		}

		return res;
	}


	public: void reset()
	{
		rt_infos_.clear();
		last_state_update_time_ = busy_time_
								= real_type/*zero*/();
//Don't reset multiplier: let the client do this
//		multiplier_ = 1;

		do_reset();
	}


	public: std::size_t num_servers() const
	{
		return do_num_servers();
	}


	public: std::size_t num_busy_servers() const
	{
		return do_num_busy_servers();
	}


	public: real_type busy_time() const
	{
		return busy_time_;
	}


	protected: void update_state()
	{
		typedef typename runtime_info_map::const_iterator iterator;

		real_type cur_time(this->node().network().engine().simulated_time());

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Updating State:  Node: " << this->node() << " -- Last-Update Time: " << last_state_update_time_ << " (Clock: " << cur_time << ")");//XXX

		if (cur_time > last_state_update_time_)
		{
			real_type start_busy_time(cur_time);

			iterator end_it(rt_infos_.end());
			for (iterator it = rt_infos_.begin(); it != end_it; ++it)
			{
				runtime_info_type& rt_info(*(it->second));

				DCS_DEBUG_TRACE_L(3, "Updating Customer: " << rt_info.get_customer() << " - share: " << rt_info.share() << " - start-time: " << rt_info.start_time() << " - elapsed-time: " << (cur_time-::std::max(rt_info.start_time(),last_state_update_time_)));//XXX

				if (rt_info.start_time() == cur_time)
				{
					continue;
				}

				real_type share(rt_info.share());
//				if (finalize)
//				{
//					ptr_customer->status(customer_type::node_killed_status);
//				}
				real_type start_time(::std::max(rt_info.start_time(), last_state_update_time_));
				if (start_busy_time > start_time)
				{
					start_busy_time = start_time;
				}
//				real_type elapsed_time(cur_time-start_time);
//				ptr_customer->runtime(ptr_customer->runtime()+elapsed_time);
				server_utilization_profile<real_type> profile;
				profile(start_time, cur_time, share);
				rt_info.utilization_profile(profile);
//				busy_time_ += elapsed_time*share;

//				DCS_DEBUG_TRACE_L(3, "Updated Busy Time: " << busy_time_);//XXX

				DCS_DEBUG_TRACE_L(3, "Updated Customer: " << rt_info.get_customer() << " - share: " << rt_info.share() << " - start-time: " << rt_info.start_time() << " - elapsed-time: " << (cur_time-::std::max(rt_info.start_time(),last_state_update_time_)));//XXX
			}

			busy_time_ += cur_time-start_busy_time;

			DCS_DEBUG_TRACE_L(3, "Updated Busy Time: " << busy_time_);//XXX

			last_state_update_time_ = cur_time;
		}

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Updating State:  Node: " << this->node() << " -- Last-Update Time: " << last_state_update_time_ << " (Clock: " << cur_time << ")");//XXX
	}


	//@{ Interface member functions

	private: virtual void do_update_service() = 0;


//	private: virtual bool do_can_serve(customer_pointer const& ptr_customer) const = 0;


	private: virtual bool do_can_serve() const = 0;


	//private: virtual real_type do_serve(customer_pointer const& ptr_customer, random_generator_type& rng) = 0;
	private: virtual runtime_info_type do_serve(customer_pointer const& ptr_customer, random_generator_type& rng) = 0;


	private: virtual void do_remove(customer_pointer const& ptr_customer) = 0;


	private: virtual void do_remove_all() = 0;


	//private: virtual runtime_info_type info(customer_pointer const& ptr_customer) const = 0;


//	private: virtual real_type do_suspend(customer_pointer const& ptr_customer) = 0;


//	private: virtual real_type do_resume(customer_pointer const& ptr_customer) = 0;

	private: virtual void do_reset() = 0;


	private: virtual std::size_t do_num_servers() const = 0;


	private: virtual std::size_t do_num_busy_servers() const = 0;

	//@} Interface member functions


	//@{ Data members

	/// Resource capacity multiplier: the real resource capacity will be the
	/// result of product between this quantity and the service time (>= 0).
	private: real_type multiplier_;
	private: real_type share_; ///< The fraction of resource.
	/// Maintain information about running times.
	private: runtime_info_map rt_infos_;
	/// Pointer the node using this service strategy.
	private: service_node_pointer ptr_node_;
	private: real_type busy_time_;
	private: real_type last_state_update_time_;

	//@} Data members
};


}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_BASE_SERVICE_STRATEGY_HPP
