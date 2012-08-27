/**
 * \file dcs/des/model/qn/networ_node.hpp
 *
 * \brief A generic node of the queueing network.
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

#ifndef DCS_DES_MODEL_QN_NETWORK_NODE_HPP
#define DCS_DES_MODEL_QN_NETWORK_NODE_HPP


#include <boost/smart_ptr.hpp>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/entity.hpp>
#include <dcs/des/model/qn/network_node_category.hpp>
#include <dcs/des/model/qn/output_statistic_category.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>


namespace dcs { namespace des { namespace model { namespace qn {


template <typename TraitsT>
class network_node: public ::dcs::des::entity
{
	//@{ Typedefs


	private: typedef ::dcs::des::entity base_type;
	private: typedef network_node<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	private: typedef typename traits_type::customer_type customer_type;
	public: typedef ::boost::shared_ptr<customer_type> customer_pointer;
	public: typedef typename traits_type::network_type network_type;
	public: typedef network_type* network_pointer;
	public: typedef typename traits_type::node_identifier_type identifier_type;
	protected: typedef typename traits_type::engine_type engine_type;
	protected: typedef typename engine_traits<engine_type>::event_type event_type;
	protected: typedef typename engine_traits<engine_type>::engine_context_type engine_context_type;
	public: typedef typename engine_traits<engine_type>::event_source_type event_source_type;
	public: typedef ::boost::shared_ptr<event_source_type> event_source_pointer;
	public: typedef base_statistic<real_type,uint_type> output_statistic_type;
	public: typedef ::boost::shared_ptr<output_statistic_type> output_statistic_pointer;
	private: typedef ::std::vector<output_statistic_pointer> output_statistic_container;
	private: typedef ::std::map<node_output_statistic_category,output_statistic_container> output_statistic_category_container;


	//@} Typedefs


	//@{ Constants

	private: static const ::std::string arrival_event_source_name;
	private: static const ::std::string departure_event_source_name;


	//@} Constants


	//@{ Member functions


	/// A constructor.
	public: network_node(identifier_type id, ::std::string const& name, network_pointer const& ptr_net = 0, bool enabled = true)
	: base_type(enabled),
	  id_(id),
	  name_(name),
	  ptr_arr_evt_src_(new event_source_type(arrival_event_source_name)),
	  ptr_dep_evt_src_(new event_source_type(departure_event_source_name)),
	  ptr_net_(ptr_net),
	  narr_(0),
	  ndep_(0),
	  last_evt_time_(0)
	{
		DCS_DEBUG_TRACE_L(5, "(" << this << ") BEGIN Constructor.");//XXX

		/// pre: node ID must be a valid node identifier.
		DCS_ASSERT(
			id_ != traits_type::invalid_node_id(),
			throw ::std::invalid_argument("[dcs::des::model::qn::network_node::ctor] Invalid node identifier.")
		);
//		/// pre: pointer to network must be a valid pointer.
//		DCS_ASSERT(
//			ptr_net_,
//			throw ::std::invalid_argument("[dcs::des::model::qn::network_node::ctor] Network not specified.")
//		);

		init();

		DCS_DEBUG_TRACE_L(5, "(" << this << ") END Constructor.");//XXX
	}


	/// The copy constructor.
	public: network_node(network_node const& that)
	: base_type(that),
	  id_(that.id_),
	  name_(that.name_),
	  ptr_arr_evt_src_(new event_source_type(*(that.ptr_arr_evt_src_))),
	  ptr_dep_evt_src_(new event_source_type(*(that.ptr_arr_evt_src_))),
	  ptr_net_(that.ptr_net_),
	  narr_(that.narr_),
	  ndep_(that.ndep_),
	  last_evt_time_(that.last_evt_time_)
	{
		DCS_DEBUG_TRACE_L(5, "(" << this << ") BEGIN Copy constructor.");//XXX

		init();

		DCS_DEBUG_TRACE_L(5, "(" << this << ") END Copy constructor.");//XXX
	}


	/// The destructor.
	public: virtual ~network_node()
	{
		DCS_DEBUG_TRACE_L(5, "(" << this << ") BEGIN Destructor.");//XXX

		finit();

		DCS_DEBUG_TRACE_L(5, "(" << this << ") END Destructor.");//XXX
	}


	/// The copy assignment.
	protected: network_node& operator=(network_node const& rhs)
	{
		DCS_DEBUG_TRACE_L(5, "(" << this << ") BEGIN Copy Assignment.");//XXX

		if (this != &rhs)
		{
			finit();

			base_type::operator=(rhs);

			id_ = rhs.id_;
			name_ = rhs.name_;
			ptr_arr_evt_src_ = event_source_pointer(new event_source_type(*(rhs.ptr_arr_evt_src_)));
			ptr_dep_evt_src_ = event_source_pointer(new event_source_type(*(rhs.ptr_arr_evt_src_)));
			ptr_net_ = rhs.ptr_net_;
			narr_ = rhs.narr_;
			ndep_ = rhs.ndep_;
			last_evt_time_ = rhs.last_evt_time_;

			init();
		}

		DCS_DEBUG_TRACE_L(5, "(" << this << ") END Copy Assignment.");//XXX

		return *this;
	}


	public: identifier_type id() const
	{
		return id_;
	}


	public: void id(identifier_type x)
	{
		id_ = x;
	}


	public: ::std::string const& name() const
	{
		return name_;
	}


	public: void name(::std::string const& s)
	{
		name_ = s;
	}


	public: network_node_category category() const
	{
		return do_category();
	}


	public: void network(network_pointer const& ptr_net)
	{
		// pre: network pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_net,
			throw ::std::invalid_argument("[dcs::des::model::qn::network_node::network] Invalid network.")
		);

		ptr_net_ = ptr_net;
	}


	public: network_type const& network() const
	{
		/// pre: network pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_net_ );

		return *ptr_net_;
	}


	public: network_type& network()
	{
		/// pre: network pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_net_ );

		return *ptr_net_;
	}


	public: void receive(customer_pointer const& ptr_customer, real_type delay)
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Receive");//XXX

		// precondition: customer pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_customer );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") Node #" << this->id() << " (" << this->name() << "): RECEIVED customer: " << *ptr_customer << " WITH delay: " << delay);//XXX

		this->schedule_arrival(ptr_customer, delay);

		//do_receive(ptr_customer, delay);

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Receive");//XXX
	}


	public: event_source_type const& arrival_event_source() const
	{
		// pre: event source pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_arr_evt_src_ );

		return *ptr_arr_evt_src_;
	}


	public: event_source_type& arrival_event_source()
	{
		// pre: event source pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_arr_evt_src_ );

		return *ptr_arr_evt_src_;
	}


	public: event_source_type const& departure_event_source() const
	{
		// pre: event source pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_dep_evt_src_ );

		return *ptr_dep_evt_src_;
	}


	public: event_source_type& departure_event_source()
	{
		// pre: event source pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_dep_evt_src_ );

		return *ptr_dep_evt_src_;
	}


	public: uint_type num_arrivals() const
	{
		return narr_;
	}


	public: uint_type num_departures() const
	{
		return ndep_;
	}


	public: real_type busy_time() const
	{
		return do_busy_time();
	}


	public: real_type utilization() const
	{
		return busy_time()/network().engine().simulated_time();
	}


//	public: void reset()
//	{
//		// Reset statistics
//
//		narr_ = ndep_
//			  = uint_type/*zero*/();
////		typename output_statistic_category_container::const_iterator stat_end_it = stats_.end();
////		for (typename output_statistic_category_container::const_iterator it = stats_.begin();
////			 it != stat_end_it;
////			 ++it)
////		{
////			reset_stat(it->first);
////		}
//
//		do_reset();
//	}


	public: void statistic(node_output_statistic_category category, output_statistic_pointer const& ptr_stat)
	{
		// pre: statistic pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_stat,
			throw ::std::invalid_argument("[dcs::des::model::qn::network_node::statistic] Invalid statistic.")
		);

//		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Adding statistic: " << *ptr_stat << " - (" << ptr_stat << ") for Category: " << category << " for Node: " << *this);//XXX

		stats_[category].push_back(ptr_stat);

//		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Adding statistic: " << *ptr_stat << " - (" << ptr_stat << ") for Category: " << category << " for Node: " << *this);//XXX
	}


	public: ::std::vector<output_statistic_pointer> statistic(node_output_statistic_category category) const
	{
		// pre: existent statistic
		if (!this->check_stat(category))
		{
			throw ::std::logic_error("[dcs::des::model::qn::network_node::statistic] No statistic associated to the given category.");
		}

		return stats_.at(category);
	}


	public: void initialize_simulation()
	{
		// Reset simulation-level statistics
		typename output_statistic_category_container::const_iterator stat_end_it = stats_.end();
		for (typename output_statistic_category_container::const_iterator it = stats_.begin();
			 it != stat_end_it;
			 ++it)
		{
			reset_stat(it->first);
		}

		do_initialize_simulation();
	}


	public: void initialize_experiment()
	{
		// Reset experiment-level statistics
		narr_ = ndep_
			  = uint_type/*zero*/();

		last_evt_time_ = real_type/*zero*/();

		do_initialize_experiment();
	}


	public: void finalize_experiment()
	{
		do_finalize_experiment();

		// Update stats that cannot be collected during the simulation.
		// For instance: if we collect busy-time during simulation at the
		// end of simulation we get as final statistic
		//     busy-time/#collected-samples
		// This is clearly wrong since busy time is simply the sum of times.
		real_type sim_time(this->network().engine().simulated_time());
		accumulate_stat(busy_time_statistic_category, this->busy_time());
//		accumulate_stat(busy_area_statistic_category, this->busy_area());//EXP
		//accumulate_stat(utilization_statistic_category, this->utilization());
		accumulate_stat(utilization_statistic_category, this->busy_time()/sim_time);
		accumulate_stat(throughput_statistic_category, ndep_/sim_time);
		accumulate_stat(num_arrivals_statistic_category, narr_);
		accumulate_stat(num_departures_statistic_category, ndep_);
	}


	protected: void arrival_event_source(event_source_pointer const& ptr_evt_src)
	{
		// pre: event source pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_evt_src,
			throw ::std::invalid_argument("[dcs::des::model::qn::network_node::arrival_event_source] Invalid event source.")
		);

		ptr_arr_evt_src_ = ptr_evt_src;
	}


	protected: void departure_event_source(event_source_pointer const& ptr_evt_src)
	{
		// pre: event source pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_evt_src,
			throw ::std::invalid_argument("[dcs::des::model::qn::network_node::departure_event_source] Invalid event source.")
		);

		ptr_dep_evt_src_ = ptr_evt_src;
	}


	protected: network_pointer network_ptr()
	{
		return ptr_net_;
	}


	protected: network_pointer const& network_ptr() const
	{
		return ptr_net_;
	}


	protected: void last_event_time(real_type time)
	{
		// pre: time >= 0
		DCS_ASSERT(
			time >= 0,
			throw ::std::invalid_argument("[dcs::des::model::qn::network_node::last_event_time] Time must be >= 0.")
		);

		last_evt_time_ = time;
	}


	protected: real_type last_event_time() const
	{
		return last_evt_time_;
	}


	//@{ Event triggers

	protected: void schedule_arrival(customer_pointer const& ptr_customer, real_type delay)
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") Begin Scheduling ARRIVAL at Node " << *this << " for Customer " << *ptr_customer << " with Delay: " << delay << " (Clock: " << ptr_net_->engine().simulated_time() << ")"); //XXX

		// pre: customer pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_customer );
		// pre: network pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_net_ );
		// pre: event source pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_arr_evt_src_ );

		ptr_net_->engine().schedule_event(
				ptr_arr_evt_src_,
				ptr_net_->engine().simulated_time()+delay,
				ptr_customer
		);

		DCS_DEBUG_TRACE_L(3, "(" << this << ") End Scheduling ARRIVAL at Node " << *this << " for Customer " << *ptr_customer << " with Delay: " << delay << " (Clock: " << ptr_net_->engine().simulated_time() << ")"); //XXX
	}


	protected: void schedule_departure(customer_pointer const& ptr_customer, real_type delay)
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") Begin Scheduling DEPARTURE at Node " << *this << " for Customer " << *ptr_customer << " with Delay: " << delay << " (Clock: " << ptr_net_->engine().simulated_time() << ")"); //XXX

		// precondition: customer pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_customer );
		// precondition: network pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_net_ );
		// pre: event source pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_dep_evt_src_ );

		ptr_net_->engine().schedule_event(
				ptr_dep_evt_src_,
				ptr_net_->engine().simulated_time()+delay,
				ptr_customer
		);

		DCS_DEBUG_TRACE_L(3, "(" << this << ") End Scheduling DEPARTURE at Node " << *this << " for Customer " << *ptr_customer << " with Delay: " << delay << " (Clock: " << ptr_net_->engine().simulated_time() << ")"); //XXX
	}

	//@} Event triggers


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
		// pre: arrival event source pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_arr_evt_src_ );
		// pre: departure event source pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_dep_evt_src_ );

		ptr_arr_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_arrival,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ptr_dep_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_departure,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


	private: void disconnect_from_event_sources()
	{
		// pre: pointer to arrival event source must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_arr_evt_src_ );
		// pre: pointer to departure event source must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_dep_evt_src_ );

		ptr_arr_evt_src_->disconnect(
			::dcs::functional::bind(
				&self_type::process_arrival,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ptr_dep_evt_src_->disconnect(
			::dcs::functional::bind(
				&self_type::process_departure,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


	//@{ Event handlers

//	private: void process_begin_of_sim(event_type const& evt, engine_context_type& ctx)
//	{
//		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Processing BEGIN-OF-SIMULATION event at Node " << *this << " for Customer " << *ptr_customer << " at Clock: " << ptr_net_->engine().simulated_time()); //XXX
//
//		initialize_simulation();
//
//		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Processing BEGIN-OF-SIMULATION event at Node " << *this << " for Customer " << *ptr_customer << " at Clock: " << ptr_net_->engine().simulated_time()); //XXX
//	}


//	private: void process_sys_init(event_type const& evt, engine_context_type& ctx)
//	{
//		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Processing SYSTEM-INITIALIZATION event at Node " << *this << " for Customer " << *ptr_customer << " at Clock: " << ptr_net_->engine().simulated_time()); //XXX
//
//		initialize_experiment();
//
//		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Processing SYSTEM-INITIALIZATION event at Node " << *this << " for Customer " << *ptr_customer << " at Clock: " << ptr_net_->engine().simulated_time()); //XXX
//	}


	private: void process_arrival(event_type const& evt, engine_context_type& ctx)
	{
		customer_pointer ptr_customer = evt.template unfolded_state<customer_pointer>();

		DCS_DEBUG_TRACE_L(3, "(" << this << ") Begin Processing ARRIVAL at Node " << *this << " for Customer " << *ptr_customer << " at Clock: " << ptr_net_->engine().simulated_time()); //XXX

		// check: customer pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_customer );

		++narr_;

		ptr_customer->node_arrival_time(id_, ctx.simulated_time());

		do_process_arrival(ptr_customer, ctx);

		last_event_time(ctx.simulated_time());

		DCS_DEBUG_TRACE_L(3, "(" << this << ") End Processing ARRIVAL at Node " << *this << " for Customer " << *ptr_customer << " at Clock: " << ptr_net_->engine().simulated_time()); //XXX
	}


	private: void process_departure(event_type const& evt, engine_context_type& ctx)
	{
		customer_pointer ptr_customer = evt.template unfolded_state<customer_pointer>();

		DCS_DEBUG_TRACE_L(3, "(" << this << ") Begin Processing DEPARTURE at Node " << *this << " for Customer " << *ptr_customer << " at Clock: " << ptr_net_->engine().simulated_time()); //XXX

		// check: customer pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_customer );

		// Update statistics

		++ndep_;

		if (this->category() != source_node_category
			&& this->category() != sink_node_category)
		{
//			accumulate_stat(throughput_statistic_category,
//							ndep_/ctx.simulated_time());
//			accumulate_stat(response_time_statistic_category,
//							ctx.simulated_time() - ptr_customer->arrival_time());
			accumulate_stat(response_time_statistic_category,
							ctx.simulated_time() - ptr_customer->node_arrival_times(id_).back());
		}

		ptr_customer->node_departure_time(id_, ctx.simulated_time());

		// Handle specialized behavior

		do_process_departure(ptr_customer, ctx);

		last_event_time(ctx.simulated_time());

		DCS_DEBUG_TRACE_L(3, "(" << this << ") End Processing DEPARTURE at Node " << *this << " for Customer " << *ptr_customer << " at Clock: " << ptr_net_->engine().simulated_time()); //XXX
	}

	//@} Event handlers


	/// Check if the given statistic category is valid.
	private: bool check_stat(node_output_statistic_category category) const
	{
		typedef typename output_statistic_category_container::size_type size_type;

		return stats_.count(category) > 0
			&& !stats_.at(category).empty();
	}


	/// Accumulate the given value for all the statistics associated to the
	/// given category.
	protected: void accumulate_stat(node_output_statistic_category category, real_type value)
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Accumulating statistic value: " << value << " for category: " << category);//XXX

		// Make sure that some statistic for this category has been registered.
		if (!this->check_stat(category))
		{
			DCS_DEBUG_TRACE_L(3, "Statistic not requested.");//XXX
			return;
		}

//		::std::for_each(
//			stats_[category].begin(),
//			stats_[category].end(),
//			::dcs::functional::bind(
//				&output_statistic_type::operator(),
//				::dcs::functional::placeholders::_1,
//				value
//			)
//		);
		typedef typename output_statistic_container::iterator stats_iterator;
		stats_iterator stats_end_it(stats_[category].end());
		for (stats_iterator stats_it = stats_[category].begin(); stats_it != stats_end_it; ++stats_it)
		{
			output_statistic_pointer ptr_stat(*stats_it);

			(*ptr_stat)(value);
		}

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Accumulating statistic value: " << value << " for category: " << category);//XXX
	}


	/// Reset all the statistics associated to the given category.
	private: void reset_stat(node_output_statistic_category category)
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Resetting statistic for category: " << category);//XXX

		// Make sure that some statistic for this category has been registered.
		if (!this->check_stat(category))
		{
			return;
		}

		::std::for_each(
			stats_[category].begin(),
			stats_[category].end(),
			::dcs::functional::bind(
				&output_statistic_type::reset,
				::dcs::functional::placeholders::_1
			)
		);

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Resetting statistic for category: " << category);//XXX
	}


	//@{ Interface member functions

	private: virtual network_node_category do_category() const = 0;


//	protected: virtual void do_reset() { }


	protected: virtual void do_initialize_simulation() { }


	protected: virtual void do_initialize_experiment() { }


	protected: virtual void do_finalize_experiment() { }


	protected: virtual void do_enable(bool flag)
	{
		// Enable/Disable stats

		typedef typename output_statistic_category_container::iterator cat_stat_iterator;
		typedef typename output_statistic_container::iterator stat_iterator;

		cat_stat_iterator cat_stat_end_it(stats_.end());
		for (cat_stat_iterator cat_stat_it = stats_.begin(); cat_stat_it != cat_stat_end_it; ++cat_stat_it)
		{
			stat_iterator stat_end_it(cat_stat_it->second.end());
			for (stat_iterator stat_it = cat_stat_it->second.begin(); stat_it != stat_end_it; ++stat_it)
			{
				(*stat_it)->enable(flag);
			}
		}

		// Enable/Disable event sources

		ptr_arr_evt_src_->enable(flag);
		ptr_dep_evt_src_->enable(flag);

//		// Connect/Disconnect to/from  event sources
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


	private: virtual void do_process_arrival(customer_pointer const& ptr_customer, engine_context_type& ctx) = 0;


	private: virtual void do_process_departure(customer_pointer const& ptr_customer, engine_context_type& ctx) = 0;


	private: virtual real_type do_busy_time() const = 0;


//	private: virtual real_type do_utilization() const = 0;

	//@} Interface member functions

	//@} Member functions


	//@{ Data members


	private: identifier_type id_;
	private: ::std::string name_;
	/// ARRIVAL event source: arrival of a customer at the node
	private: event_source_pointer ptr_arr_evt_src_;
	/// DEPARTURE event source: departure of a customer from the node
	private: event_source_pointer ptr_dep_evt_src_;
//	/// DISCARD event source: discard of a customer from the node
//	private: event_source_pointer ptr_dis_evt_src_;
	private: network_pointer ptr_net_;
	/// The number of arrived customers to this node.
	private: uint_type narr_;
	/// The number of (successfully) departed customers from this node.
	private: uint_type ndep_;
//	/// The number of discarded customers from this node.
//	private: uint_type ndis_;
	/// Output statistics grouped by their category.
	private: output_statistic_category_container stats_;
	/// The time of the last processed event.
	private: real_type last_evt_time_;


	//@} Data members
}; // network_node


template <typename TraitsT>
const ::std::string network_node<TraitsT>::arrival_event_source_name("Arrival to Node");


template <typename TraitsT>
const ::std::string network_node<TraitsT>::departure_event_source_name("Departure to Node");


template <
	typename CharT,
	typename CharTraitsT,
	typename QueueNetTraitsT
>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, network_node<QueueNetTraitsT> const& node)
{
	return os << "<"
			  <<   "ID: " << node.id()
			  << ", Name: " << node.name()
			  << ", Category: " << node.category()
			  << ">";
}

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_NETWORK_NODE_HPP
