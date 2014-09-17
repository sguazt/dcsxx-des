/**
 * \file dcs/des/model/qn/queueing_station_node.hpp
 *
 * \brief A service station node with queueing capabilities.
 *
 * Copyright (C) 2012       Distributed Computing System (DCS) Group,
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

#ifndef DCS_DES_MODEL_QN_QUEUEING_STATION_NODE_HPP
#define DCS_DES_MODEL_QN_QUEUEING_STATION_NODE_HPP


#include <boost/smart_ptr.hpp>
#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/model/qn/queueing_strategy.hpp>
#include <dcs/des/model/qn/service_station_node.hpp>
#include <dcs/des/model/qn/output_statistic_category.hpp>
#include <dcs/macro.hpp>


namespace dcs { namespace des { namespace model { namespace qn {

template <typename TraitsT>
class queueing_station_node: public service_station_node<TraitsT>
{
	private: typedef service_station_node<TraitsT> base_type;
	private: typedef queueing_station_node<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename base_type::real_type real_type;
	public: typedef typename base_type::identifier_type identifier_type;
	public: typedef typename base_type::customer_pointer customer_pointer;
	public: typedef typename base_type::service_strategy_pointer service_strategy_pointer;
	public: typedef typename base_type::routing_strategy_pointer routing_strategy_pointer;
	public: typedef queueing_strategy<traits_type> queueing_strategy_type;
	public: typedef ::boost::shared_ptr<queueing_strategy_type> queueing_strategy_pointer;
	//public: typedef ::std::size_t size_type;
	private: typedef typename traits_type::engine_type engine_type;
	private: typedef typename engine_traits<engine_type>::event_type event_type;
	private: typedef typename engine_traits<engine_type>::engine_context_type engine_context_type;
	public: typedef typename engine_traits<engine_type>::event_source_type event_source_type;
	private: typedef typename base_type::service_strategy_type service_strategy_type;
	private: typedef typename service_strategy_type::runtime_info_type runtime_info_type;
	private: typedef typename traits_type::customer_type customer_type;
	private: typedef typename base_type::event_source_pointer event_source_pointer;


	private: static const ::std::string discard_event_source_name;


	//@{ Member functions

	/// A constructor.
	public: queueing_station_node(identifier_type id,
								  ::std::string const& name)
	: base_type(id, name),
	  ptr_queue_(),
	  ndis_(0),
	  ptr_dis_evt_src_(new event_source_type(discard_event_source_name))
	{
		init();
	}


	/// A constructor.
	public: queueing_station_node(identifier_type id,
								  ::std::string const& name,
								  queueing_strategy_pointer const& ptr_queueing,
								  service_strategy_pointer const& ptr_service,
								  routing_strategy_pointer const& ptr_routing)
	: base_type(id, name, ptr_service, ptr_routing),
	  ptr_queue_(ptr_queueing),
	  ndis_(0),
	  ptr_dis_evt_src_(new event_source_type(discard_event_source_name))
	{
		// pre: queueing strategy pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_queueing,
			throw ::std::invalid_argument("[dcs::des::model::qn::queueing_station_node::ctor] Invalid queueing strategy.")
		);

		init();
	}


	/// The copy constructor.
	public: queueing_station_node(queueing_station_node const& that)
	: base_type(that),
	  ptr_queue_(that.ptr_queue_),
	  ndis_(that.ndis_),
	  ptr_dis_evt_src_(new event_source_type(*(that.ptr_dis_evt_src_)))
	{
		init();
	}


	/// The destructor.
	public: virtual ~queueing_station_node()
	{
		finit();
	}


	/// The copy assignment operator.
	public: queueing_station_node& operator=(queueing_station_node const& rhs)
	{
		if (this != &rhs)
		{
			finit();

			base_type::operator=(rhs);

			ptr_queue_ = rhs.ptr_queue_;
			ndis_ = rhs.ndis_;
			ptr_dis_evt_src_ = ::boost::make_shared<event_source_type>(*(rhs.ptr_dis_evt_src_));

			init();
		}

		return *this;
	}


//TODO: maybe in the future
//	public: void preload(ForwardIterT first_ptr_customer, ForwardIterT last_ptr_customer)
//	{
//		while (first_ptr_customer != last_ptr_customer)
//		{
//			ptr_queue->push(*first_ptr_customer);
//
//			++first_ptr_customer;
//		}
//	}


	public: event_source_type const& discard_event_source() const
	{
		// pre: discard event source pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_dis_evt_src_ );

		return *ptr_dis_evt_src_;
	}


	public: event_source_type& discard_event_source()
	{
		// pre: discard event source pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_dis_evt_src_ );

		return *ptr_dis_evt_src_;
	}


	protected: void discard_event_source(event_source_pointer const& ptr_evt_src)
	{
		// pre: discard event source pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_evt_src,
			throw ::std::invalid_argument("[dcs::des::model::qn::queueing_station_node::discard_event_source] Invalid event source.")
		);

		ptr_dis_evt_src_ = ptr_evt_src;
	}


	//@{ Event triggers

	protected: void schedule_discard(customer_pointer const& ptr_customer, real_type delay)
	{
		DCS_DEBUG_TRACE_L(3, "Begin Scheduling DISCARD at Node " << *this << " for Customer " << *ptr_customer << " with Delay: " << delay << " (Clock: " << this->network().engine().simulated_time() << ")"); //XXX

		// precondition: customer pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_customer );
//		// precondition: network pointer must be a valid pointer.
//		DCS_DEBUG_ASSERT( this->network_ptr() );

		this->network().engine().schedule_event(
				ptr_dis_evt_src_,
				this->network().engine().simulated_time()+delay,
				ptr_customer
		);

		DCS_DEBUG_TRACE_L(3, "End Scheduling DISCARD at Node " << *this << " for Customer " << *ptr_customer << " with Delay: " << delay << " (Clock: " << this->network().engine().simulated_time() << ")"); //XXX
	}
 
	//@} Event triggers


	//@{ Interface member functions

//	protected: virtual void do_reset()
//	{
//		base_type::do_reset();
//
//		ndis_ = std::size_t/*zero*/();
//		ptr_queue_->reset();
//	}


//	protected: virtual void do_initialize_simulation()
//	{
//		base_type::do_initialize_simulation();
//	}


	protected: virtual void do_initialize_experiment()
	{
		base_type::do_initialize_experiment();

		ndis_ = std::size_t/*zero*/();
		ptr_queue_->reset();
	}


	private: void do_process_arrival(customer_pointer const& ptr_customer, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// pre: pointer to customer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_customer );
		// pre: queueing strategy pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_queue_ );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Do Processing ARRIVAL at Node: " << *this << " for Customer: " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX

		base_type::do_process_arrival(ptr_customer, ctx);

		ptr_customer->change_node(this->id());

//		this->schedule_arrival(ptr_customer, real_type/*zero*/());

		if (ptr_queue_->can_push(ptr_customer))
		{
			ptr_queue_->push(ptr_customer);

//			this->schedule_entry(ptr_customer, real_type/*zero*/());

			// Serve a new customer (if possible)
			serve(ctx);
		}
		else
		{
			this->schedule_discard(ptr_customer, real_type/*zero*/());
		}

		this->accumulate_stat(num_waiting_statistic_category, ptr_queue_->size());

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Do Processing ARRIVAL at Node: " << *this << " for Customer: " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX
	}


//	private: void do_process_entry(customer_pointer const& ptr_customer, engine_context_type& ctx)
//	{
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_customer );
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
//
//		// (Possibly) Serve a new customer
//		serve(ctx);
//	}


	private: void do_process_service(customer_pointer const& ptr_customer, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// pre: pointer to customer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_customer );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Do Processing SERVICE at Node: " << *this << " for Customer: " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX

		this->schedule_departure(ptr_customer, real_type/*zero*/());

		// (Possibly) Serve a new customer
		serve(ctx);

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Do Processing SERVICE at Node: " << *this << " for Customer: " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX
	}


	private: void do_process_departure(customer_pointer const& ptr_customer, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// pre: pointer to customer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_customer );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Do Processing DEPARTURE at Node: " << *this << " for Customer: " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX

		base_type::do_process_arrival(ptr_customer, ctx);

		// Choose the routing destination
		typedef typename base_type::routing_strategy_type routing_strategy_type;
		typename routing_strategy_type::routing_destination_type dst_route;
		dst_route = this->routing_strategy().route(ptr_customer);
//		typedef typename traits_type::random_generator_type random_generator_type;
//		dst_route = this->routing_strategy().template route<random_generator_type&>(ptr_customer,this->network_ptr()->random_generator());
		typename traits_type::class_identifier_type class_id;
		class_id = this->routing_strategy().class_id(dst_route);
		typename traits_type::class_identifier_type node_id;
		node_id = this->routing_strategy().node_id(dst_route);

		// Change the customer class
		ptr_customer->change_class(class_id);

		// Send this customer to the target node
		this->network().get_node(node_id).receive(ptr_customer, real_type/*zero*/());

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Do Processing DEPARTURE at Node: " << *this << " for Customer: " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX
	}


	private: virtual void do_process_discard(customer_pointer const& ptr_customer, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_customer );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Do Processing DISCARD at Node: " << *this << " for Customer: " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Do Processing DISCARD at Node: " << *this << " for Customer: " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX
	}


//	private: void do_process_exit(customer_pointer const& ptr_customer, engine_context_type& ctx)
//	{
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_customer );
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
//	}


	//@} Interface member functions


	//@{ Event handlers

	private: void process_discard(event_type const& evt, engine_context_type& ctx)
	{
		customer_pointer ptr_customer = evt.template unfolded_state<customer_pointer>();

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Processing DISCARD at Node: " << *this << " for Customer: " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX

		// check: customer pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_customer );

		++ndis_;

		this->network().schedule_discard(ptr_customer, real_type/*zero*/());

		do_process_discard(ptr_customer, ctx);

		this->last_event_time(ctx.simulated_time());

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Processing DISCARD at Node: " << *this << " for Customer: " << *ptr_customer << " (Clock: " << ctx.simulated_time() << ")."); //XXX
	}

	//@} Event handlers


	private: void init()
	{
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
		// pre: discard event source pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_dis_evt_src_ );

		ptr_dis_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_discard,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


	private: void disconnect_from_event_sources()
	{
		// pre: discard event source pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_dis_evt_src_ );

		ptr_dis_evt_src_->disconnect(
			::dcs::functional::bind(
				&self_type::process_discard,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


	private: void serve(engine_context_type const& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ctx);

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Serving new Customer at Node: " << *this << ".");//XXX

		// pre: queueing strategy pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_queue_ );

		if (!ptr_queue_->empty() && this->service_strategy().can_serve())
		{
			customer_pointer ptr_customer = ptr_queue_->peek();
			ptr_queue_->pop();

			// check: customer pointer must be a valid pointer
			DCS_DEBUG_ASSERT( ptr_customer );

			real_type runtime(0);
			typename traits_type::random_generator_type& ref_rng = this->network().random_generator();
			//runtime = this->service_strategy().serve(ptr_customer, ref_rng);
			runtime_info_type rt_info;
			rt_info = this->service_strategy().serve(ptr_customer, ref_rng);
			//runtime = this->service_strategy().info(ptr_customer).runtime();
			//runtime = rt_info.runtime()/rt_info.share();
			//runtime = rt_info.runtime()/(rt_info.share()*this->service_strategy().capacity_multiplier());
			runtime = rt_info.runtime();
//			this->service_strategy().info(ptr_customer).start_time(ctx.simulated_time());//EXP
			//ptr_customer->runtime(runtime+ptr_customer->runtime());

			DCS_DEBUG_TRACE_L(3, "Serving Customer: " << *ptr_customer << " @ runtime: " << runtime);

			this->schedule_service(ptr_customer, runtime);
		}
#ifdef DCS_DEBUG
		else
		{
			if (ptr_queue_->empty())
			{
				DCS_DEBUG_TRACE_L(3, "Cannot serve: no customer to be served.");
			}
			else
			{
				DCS_DEBUG_TRACE_L(3, "Cannot serve: all servers are busy.");
			}
		}
#endif // DCS_DEBUG

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Serving new Customer at Node: " << *this << ".");//XXX
	}


	protected: void do_enable(bool flag)
	{
		base_type::do_enable(flag);

		ptr_dis_evt_src_->enable(flag);

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


	//@} Member functions


	//@{ Data members

	/// The queueing strategy.
	private: queueing_strategy_pointer ptr_queue_;
	/// The number of discarded customers in current experiment.
	private: std::size_t ndis_;
	/// The source of DISCARD-CUSTOMER events.
	private: event_source_pointer ptr_dis_evt_src_;

	//@} Data members
}; // queueing_station_node


template <typename TraitsT>
const ::std::string queueing_station_node<TraitsT>::discard_event_source_name("Discard from Node");

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_QUEUEING_STATION_NODE_HPP
