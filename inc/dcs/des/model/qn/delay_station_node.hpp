/**
 * \file src/dcs/des/model/qn/delay_station_node.hpp
 *
 * \brief Delay (infinite server) service station.
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

#ifndef DCS_DES_MODEL_QN_DELAY_STATION_NODE_HPP
#define DCS_DES_MODEL_QN_DELAY_STATION_NODE_HPP


#include <boost/smart_ptr.hpp>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/model/qn/infinite_server_service_strategy.hpp>
#include <dcs/des/model/qn/network_node_category.hpp>
#include <dcs/des/model/qn/service_station_node.hpp>
//#include <dcs/des/model/qn/passthrough_input_strategy.hpp>
#include <dcs/macro.hpp>
#include <string>


namespace dcs { namespace des { namespace model { namespace qn {

/**
 * \todo To be completed.
 */
template <typename TraitsT>
class delay_station_node: public service_station_node<TraitsT>
{
	private: typedef service_station_node<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename base_type::identifier_type identifier_type;
	public: typedef typename base_type::real_type real_type;
	private: typedef typename traits_type::customer_type customer_type;
	public: typedef typename base_type::customer_pointer customer_pointer;
//	private: typedef passthrough_input_strategy<traits_type> input_strategy_type;
	public: typedef typename base_type::routing_strategy_pointer routing_strategy_pointer;
	private: typedef typename traits_type::engine_type engine_type;
	private: typedef typename engine_traits<engine_type>::event_type event_type;
	private: typedef typename engine_traits<engine_type>::engine_context_type engine_context_type;
	private: typedef typename base_type::service_strategy_type service_strategy_type;
	private: typedef typename service_strategy_type::runtime_info_type runtime_info_type;
	private: typedef typename base_type::service_strategy_pointer service_strategy_pointer;
	private: typedef infinite_server_service_strategy<traits_type> service_strategy_impl_type;


	public: delay_station_node(identifier_type id,
							   ::std::string const& name)
	: base_type(id, name),
	  busy_time_(0)
	{
//		this->input_strategy(
//			input_strategy_pointer(
//				new passthrough_input_strategy<traits_type>()
//			)
//		);
	}


/*
	public: delay_station_node(identifier_type id,
							   ::std::string const& name,
							   service_strategy_pointer const& ptr_service,
							   routing_strategy_pointer const& ptr_routing)
	: base_type(id, name, ptr_service, ptr_routing)
	{
	}
*/


	public: delay_station_node(identifier_type id,
							   ::std::string const& name,
							   routing_strategy_pointer const& ptr_routing)
	: base_type(id, name),
	  busy_time_(0)
	{
		this->routing_strategy(ptr_routing);
	}


	public: template <typename ForwardIterT>
		delay_station_node(identifier_type id,
						   ::std::string const& name,
						   ForwardIterT first_distr,
						   ForwardIterT last_distr,
						   routing_strategy_pointer const& ptr_routing)
	: base_type(id, name),
	  busy_time_(0)
	{
		service_strategy_pointer ptr_service;

		ptr_service = ::boost::make_shared<service_strategy_impl_type>(first_distr, last_distr);

		this->service_strategy(ptr_service);
		this->routing_strategy(ptr_routing);
	}


	private: network_node_category do_category() const
	{
		return delay_station_node_category;
	}


	protected: void do_initialize_experiment()
	{
		base_type::do_initialize_experiment();

		busy_time_ = real_type/*zero*/();
	}


	private: void do_process_arrival(customer_pointer const& ptr_customer, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Do Processing ARRIVAL at Node: " << *this);//XXX

		// pre: customer pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_customer );

//		real_type runtime(0);
//		runtime_info_type rt_info;
//		rt_info = this->service_strategy().serve(ptr_customer,
//													 this->network().random_generator());
//		runtime = this->service_strategy().info(ptr_customer).runtime();
//
//		DCS_DEBUG_TRACE_L(3, "Serving Customer: " << *ptr_customer << " @ runtime: " << runtime);
//
//		this->schedule_service(ptr_customer, runtime);

		ptr_customer->change_node(this->id());

		real_type runtime(0);
		typename traits_type::random_generator_type& ref_rng = this->network().random_generator();
		runtime_info_type rt_info;
		rt_info = this->service_strategy().serve(ptr_customer, ref_rng);
		//runtime = rt_info.runtime()/rt_info.share();
		runtime = rt_info.runtime();

		DCS_DEBUG_TRACE_L(3, "Serving Customer: " << *ptr_customer << " @ runtime: " << runtime);

		this->schedule_service(ptr_customer, runtime);

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Do Processing ARRIVAL at Node: " << *this);//XXX
	}


	private: void do_process_service(customer_pointer const& ptr_customer, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Do Processing SERVICE at Node: " << *this);//XXX

		// pre: customer pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_customer );

//		// Update node stats
//		busy_time_ += ctx.simulated_time() - this->last_event_time();
//
//		// Update current customer info...
//		ptr_customer->status(customer_type::node_served_status);
//		ptr_customer->runtime(ptr_customer->runtime()
//							  + this->service_strategy().info(ptr_customer).runtime());
//
//		// ... And remove it from service
//		this->service_strategy().remove(ptr_customer);

		DCS_DEBUG_TRACE_L(3, "Service of customer: " << *ptr_customer << " is done.");

		this->schedule_departure(ptr_customer, real_type/*zero*/());

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Do Processing SERVICE at Node: " << *this);//XXX
	} 


	private: void do_process_departure(customer_pointer const& ptr_customer, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Do Processing DEPARTURE at Node: " << *this);//XXX

		// pre: customer pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_customer );

        // Choose the routing destination
        typedef typename base_type::routing_strategy_type routing_strategy_type;
        typename routing_strategy_type::routing_destination_type dst_route;
        dst_route = this->routing_strategy().route(ptr_customer);
//		typedef typename traits_type::random_generator_type random_generator_type;
//		dst_route = this->get_routing_strategy().template route<random_generator_type&>(ptr_customer,this->network_ptr()->random_generator());
        typename traits_type::class_identifier_type class_id;
        class_id = this->routing_strategy().class_id(dst_route);
        typename traits_type::class_identifier_type node_id;
        node_id = this->routing_strategy().node_id(dst_route);

        // Change the customer class
        ptr_customer->change_class(class_id);

		DCS_DEBUG_TRACE_L(3, "Sending customer: " << *ptr_customer << " to Node: " << this->network().get_node(node_id));//XXX

        // Send this customer to the target node
        this->network().get_node(node_id).receive(ptr_customer, real_type/*zero*/());

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Do Processing DEPARTURE at Node: " << *this);//XXX
	} 


	private: real_type do_busy_time() const
	{
		return busy_time_;
	}


	private: real_type busy_time_;
};


}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_DELAY_STATION_NODE_HPP
