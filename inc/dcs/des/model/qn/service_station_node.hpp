/**
 * \file dcs/des/model/qn/service_station_node.hpp
 *
 * \brief Node representing a service station.
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

#ifndef DCS_DES_MODEL_QN_SERVICE_STATION_NODE_HPP
#define DCS_DES_MODEL_QN_SERVICE_STATION_NODE_HPP


#include <algorithm>
#include <boost/smart_ptr.hpp>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/model/qn/base_routing_strategy.hpp>
#include <dcs/des/model/qn/base_service_strategy.hpp>
//#include <dcs/des/model/qn/input_strategy.hpp>
#include <dcs/des/model/qn/network_node.hpp>
#include <dcs/des/model/qn/network_node_category.hpp>
#include <dcs/macro.hpp>
#include <map>
#include <string>
#include <vector>


namespace dcs { namespace des { namespace model { namespace qn {

template <typename TraitsT>
class service_station_node: public network_node<TraitsT>
{
	private: typedef network_node<TraitsT> base_type;
	private: typedef service_station_node<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	private: typedef typename traits_type::customer_type customer_type;
	public: typedef typename base_type::identifier_type identifier_type;
	public: typedef typename base_type::customer_pointer customer_pointer;
	public: typedef typename base_type::real_type real_type;
	public: typedef typename base_type::uint_type uint_type;//[sguazt] EXP
	private: typedef typename base_type::event_type event_type;
	private: typedef typename base_type::engine_context_type engine_context_type;
	public: typedef typename base_type::event_source_type event_source_type;
	public: typedef typename base_type::event_source_pointer event_source_pointer;
//	public: typedef input_strategy<traits_type> input_strategy_type;
	public: typedef base_service_strategy<traits_type> service_strategy_type;
	public: typedef base_routing_strategy<traits_type> routing_strategy_type;
//	public: typedef queueing_strategy<traits_type> queueing_strategy_type;
//	private: typedef ::boost::shared_ptr<queueing_source_type> queueing_source_pointer;
	public: typedef ::boost::shared_ptr<service_strategy_type> service_strategy_pointer;
	public: typedef ::boost::shared_ptr<routing_strategy_type> routing_strategy_pointer;
	private: typedef typename service_strategy_type::runtime_info_type runtime_info_type;
	private: typedef ::boost::shared_ptr<event_type> event_pointer;
	private: typedef ::std::map<uint_type,event_pointer> customer_event_map;


	private: static const ::std::string service_event_source_name;


	/// A constructor.
	protected: service_station_node(identifier_type id,
									::std::string const& name)
	: base_type(id, name),
	  ptr_srv_(),
	  ptr_route_(),
	  ptr_srv_evt_src_(new event_source_type(service_event_source_name))
	{
		// pre: service strategy pointer must be a valid pointer
		DCS_ASSERT(
			ptr_srv_,
			throw ::std::invalid_argument("[dcs::des::model::qn::service_station_node::ctor] Invalid service strategy.")
		);
		// pre: routing strategy pointer must be a valid pointer
		DCS_ASSERT(
			ptr_route_,
			throw ::std::invalid_argument("[dcs::des::model::qn::service_station_node::ctor] Invalid routing strategy.")
		);

		init();
	}


	/// A constructor.
	protected: service_station_node(identifier_type id,
									::std::string const& name,
									service_strategy_pointer const& ptr_service,
									routing_strategy_pointer const& ptr_routing)
	: base_type(id, name),
	  ptr_srv_(ptr_service),
	  ptr_route_(ptr_routing),
	  ptr_srv_evt_src_(new event_source_type(service_event_source_name))
	{
		// pre: service strategy pointer must be a valid pointer
		DCS_ASSERT(
			ptr_srv_,
			throw ::std::invalid_argument("[dcs::des::model::qn::service_station_node::ctor] Invalid service strategy.")
		);
		// pre: routing strategy pointer must be a valid pointer
		DCS_ASSERT(
			ptr_route_,
			throw ::std::invalid_argument("[dcs::des::model::qn::service_station_node::ctor] Invalid routing strategy.")
		);

		init();
	}


	/// The copy constructor.
	public: service_station_node(service_station_node const& that)
	: base_type(that),
	  ptr_srv_(that.ptr_srv_),
	  ptr_route_(that.ptr_route_),
	  ptr_srv_evt_src_(new event_source_type(*(that.ptr_srv_evt_src_)))
	{
		init();
	}


	/// The destructor.
	public: virtual ~service_station_node()
	{
		finit();
	}


	/// The copy-assignment operator.
	public: service_station_node& operator=(service_station_node const& rhs)
	{
		if (this != &rhs)
		{
			finit();

			base_type::operator=(rhs);

			ptr_srv_ = rhs.ptr_srv_;
			ptr_route_ = rhs.ptr_route_;
			ptr_srv_evt_src_ = ::boost::make_shared<event_source_type>(*(rhs.ptr_srv_evt_src_));

			init();
		}

		return *this;
	}


	public: service_strategy_type const& service_strategy() const
	{
		// pre: service strategy pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_srv_ );

		return *ptr_srv_;
	}


	public: service_strategy_type& service_strategy()
	{
		// pre: service strategy pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_srv_ );

		return *ptr_srv_;
	}


	public: routing_strategy_type const& routing_strategy() const
	{
		// pre: routing strategy pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_route_ );

		return *ptr_route_;
	}


	public: routing_strategy_type& routing_strategy()
	{
		// pre: routing strategy pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_route_ );

		return *ptr_route_;
	}


	public: event_source_type const& service_event_source() const
	{
		// pre: service strategy pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_srv_evt_src_ );

		return *ptr_srv_evt_src_;
	}


	public: event_source_type& service_event_source()
	{
		// pre: service strategy pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_srv_evt_src_ );

		return *ptr_srv_evt_src_;
	}


	public: void capacity_multiplier(real_type m)
	{
		// pre: service strategy pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_srv_ );

		real_type old_m(ptr_srv_->capacity_multiplier());

//		if (old_m != m)
		if (::std::abs(old_m - m) > 1e-5)
		{
			// Server capacity is *really* changed

			ptr_srv_->capacity_multiplier(m);

//XXX: already done inside service-strategy
//			real_type cur_time(this->network().engine().simulated_time());
//
//			// ... -> reschedule active events
//			typedef typename customer_event_map::iterator customer_event_iterator;
//			customer_event_iterator evt_end_it(cust_evt_map_.end());
//			for (customer_event_iterator it = cust_evt_map_.begin(); it != evt_end_it; ++it)
//			{
//				event_pointer ptr_evt(it->second);
//				
//				DCS_DEBUG_ASSERT( it->first == (*ptr_evt).template unfolded_state<customer_pointer>()->id() );
//
//				real_type fire_time;
//				real_type resid_time(ptr_evt->fire_time()-cur_time);
//				fire_time = cur_time+resid_time*old_m/m;
//				this->network().engine().reschedule_event(ptr_evt, fire_time);
//			}
		}
	}


	public: real_type capacity_multiplier()
	{
		// pre: service strategy pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_srv_ );

		return ptr_srv_->capacity_multiplier();
	}


	public: void reschedule_service(customer_type customer, real_type delay)
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Rescheduling Service for  Customer: " << customer);///XXX

		event_pointer ptr_evt(cust_evt_map_.at(customer.id()));
			
		// check: paranoid check
		DCS_DEBUG_ASSERT( customer.id() == (*ptr_evt).template unfolded_state<customer_pointer>()->id() );

		real_type fire_time(this->network().engine().simulated_time()+delay);

		DCS_DEBUG_TRACE_L(3, "(" << this << ") Old Fire-Time: " << ptr_evt->fire_time() << " --> New Fire-Time: " << fire_time);///XXX

		this->network().engine().reschedule_event(ptr_evt, fire_time);

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Rescheduling Service for  Customer: " << customer);///XXX
	}


	public: ::std::vector<customer_pointer> active_customers() const
	{
		typedef typename customer_event_map::const_iterator iterator;

		//update_state();
		//const_cast<self_type*>(this)->update_state();

		::std::vector<customer_pointer> customers;

		iterator evt_end_it(cust_evt_map_.end());
		for (iterator it = cust_evt_map_.begin(); it != evt_end_it; ++it)
		{
			event_pointer ptr_evt(it->second);
			customer_pointer ptr_customer((*ptr_evt).template unfolded_state<customer_pointer>());

			// check: double check
			DCS_DEBUG_ASSERT( it->first == ptr_customer->id() );

			customers.push_back(ptr_customer);
		}

		return customers;
	}


	protected: void service_strategy(service_strategy_pointer const& ptr_strategy)
	{
		// pre: service strategy pointer must be a valid pointer
		DCS_ASSERT(
			ptr_strategy,
			throw ::std::invalid_argument("[dcs::des::model::qn::service_station_node::service_strategy] Invalid service strategy.")
		);

		ptr_srv_ = ptr_strategy;
	}


	protected: void routing_strategy(routing_strategy_pointer const& ptr_strategy)
	{
		// pre: service strategy pointer must be a valid pointer
		DCS_ASSERT(
			ptr_strategy,
			throw ::std::invalid_argument("[dcs::des::model::qn::service_station_node::routing_strategy] Invalid routing strategy.")
		);

		ptr_route_ = ptr_strategy;
	}


	protected: void schedule_service(customer_pointer const& ptr_customer, real_type delay)
	{
		// precondition: customer pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_customer );
//		// precondition: network pointer must be a valid pointer.
//		DCS_DEBUG_ASSERT( this->network_ptr() );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Scheduling SERVICE for Customer at Node " << *this << " for Customer " << *ptr_customer << " with Delay " << delay << " (Clock: " << this->network().engine().simulated_time() << ")"); //XXX

		::boost::shared_ptr<event_type> ptr_evt;//[sguazt] EXP
		//this->network().engine().schedule_event(
		ptr_evt = this->network().engine().schedule_event(//[sguazt] EXP
				ptr_srv_evt_src_,
				this->network().engine().simulated_time()+delay,
				ptr_customer
		);
		cust_evt_map_[ptr_customer->id()] = ptr_evt;//[sguazt] EXP
//		ptr_srv_->info(ptr_customer).start_time(this->network().engine().simulated_time());//EXP

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Scheduling SERVICE for Customer at Node " << *this << " for Customer " << *ptr_customer << " with Delay " << delay << " (Clock: " << this->network().engine().simulated_time() << ")"); //XXX
	}


	protected: virtual void do_enable(bool flag)
	{
		base_type::do_enable(flag);

		ptr_srv_evt_src_->enable(flag);

//		if (flag)
//		{
//			if (!this->enabled())
//			{
//				connect_to_event_sources();
//			}
//		}
//		else
//		{
//			if (this->enabled())
//			{
//				disconnect_from_event_sources();
//			}
//		}
	}


	protected: virtual void do_initialize_simulation()
	{
		base_type::do_initialize_simulation();
	}


	protected: virtual void do_initialize_experiment()
	{
		base_type::do_initialize_experiment();

		ptr_srv_->reset();
		cust_evt_map_.clear();//[sguazt] EXP
	}


	protected: virtual void do_finalize_experiment()
	{
		base_type::do_finalize_experiment();

		typedef typename customer_event_map::iterator customer_event_iterator;
		customer_event_iterator evt_end_it(cust_evt_map_.end());
		for (customer_event_iterator it = cust_evt_map_.begin(); it != evt_end_it; ++it)
		{
			event_pointer ptr_evt(it->second);
			customer_pointer ptr_customer((*ptr_evt).template unfolded_state<customer_pointer>());

			// check: paranoid check
			DCS_DEBUG_ASSERT( it->first == ptr_customer->id() );

			ptr_customer->status(customer_type::node_killed_status);
		}

		ptr_srv_->remove_all();
		cust_evt_map_.clear();
	}


	protected: virtual void do_process_arrival(customer_pointer const& ptr_customer, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ptr_customer);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ctx);
	}


	protected: virtual void do_process_departure(customer_pointer const& ptr_customer, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ptr_customer);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ctx);
	}


	private: virtual network_node_category do_category() const
	{
		return service_station_node_category;
	}


	private: real_type do_busy_time() const
	{
		return ptr_srv_->busy_time();
	}


	private: void init()
	{
		// pre: service event source pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_srv_ );

		ptr_srv_->node(this);

//		if (this->enabled())
//		{
			connect_to_event_sources();
//		}
	}


	private: void finit()
	{
//		if (this->enabled())
//		{
			disconnect_from_event_sources();
//		}
	}


	private: void connect_to_event_sources()
	{
		DCS_DEBUG_ASSERT( ptr_srv_evt_src_ );

//		ptr_ent_evt_src_->connect(
//			::dcs::functional::bind(
//				&self_type::process_entry,
//				this,
//				::dcs::functional::placeholders::_1,
//				::dcs::functional::placeholders::_2
//			)
//		);
		ptr_srv_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_service,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
//		ptr_exi_evt_src_->connect(
//			::dcs::functional::bind(
//				&self_type::process_exit,
//				this,
//				::dcs::functional::placeholders::_1,
//				::dcs::functional::placeholders::_2
//			)
//		);
	}


	private: void disconnect_from_event_sources()
	{
		// pre: service event source pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_srv_evt_src_ );

		ptr_srv_evt_src_->disconnect(
			::dcs::functional::bind(
				&self_type::process_service,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


	private: void process_service(event_type const& evt, engine_context_type& ctx)
	{
		customer_pointer ptr_customer = evt.template unfolded_state<customer_pointer>();

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Processing SERVICE at Node " << *this << " for Customer " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX

		// check: customer pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_customer );

//		real_type runtime(ptr_srv_->info(ptr_customer).runtime());
//		runtime_info_type& rt_info(ptr_srv_->info(ptr_customer));
//		real_type capacity(ptr_srv_->capacity_multiplier()/static_cast<real_type>(ptr_srv_->num_servers()));
//		real_type runtime(rt_info.runtime());

		// Update current customer info...
		ptr_customer->status(customer_type::node_served_status);

		//do_process_service(ptr_customer, ctx);

		// ... And remove it from service
		ptr_srv_->remove(ptr_customer);
		cust_evt_map_.erase(ptr_customer->id());//[sguazt] EXP

		this->last_event_time(ctx.simulated_time());

		do_process_service(ptr_customer, ctx);

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Processing SERVICE at Node " << *this << " for Customer " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX
	}


	private: virtual void do_process_service(customer_pointer const& ptr_customer, engine_context_type& ctx) = 0;


	private: service_strategy_pointer ptr_srv_;
	private: routing_strategy_pointer ptr_route_;
	private: event_source_pointer ptr_srv_evt_src_;
	private: customer_event_map cust_evt_map_;
	private: real_type last_state_update_time_;
};


template <typename TraitsT>
const ::std::string service_station_node<TraitsT>::service_event_source_name("Service at Node");

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_SERVICE_STATION_NODE_HPP
