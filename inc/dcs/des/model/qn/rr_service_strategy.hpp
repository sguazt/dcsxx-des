/**
 * \file dcs/des/model/qn/rr_service_strategy.hpp
 *
 * \brief State-dependent round-robin service strategy.
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

#ifndef DCS_DES_MODEL_QN_RR_SERVICE_STRATEGY_HPP
#define DCS_DES_MODEL_QN_RR_SERVICE_STRATEGY_HPP


#include <boost/smart_ptr.hpp>
#include <cmath>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/model/qn/base_service_strategy.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/math/stats/distribution/any_distribution.hpp>
#include <dcs/math/stats/function/rand.hpp>
#include <dcs/math/traits/float.hpp>
#include <deque>
#include <limits>
#include <map>
#include <vector>


namespace dcs { namespace des { namespace model { namespace qn {

namespace detail { namespace /*<unnamed>*/ {

template <typename RealT, typename UIntT>
struct quantum_expiry_event_state
{
	typedef RealT real_type;
	typedef UIntT uint_type;

	uint_type sid; ///< The server ID where the QUANTUM-EXPIRY event happens.
	real_type work; ///< The amount of work (independent by the share) done during the time the quantum is owned (generally is the same as the quantum length).
	bool early_expiry; ///< Flag to indicate either or not the expiration of the quantum is premature.
	real_type update_time;
	real_type max_fire_time;
};

}} // Namespace detail::<unnamed>


/**
 * \brief Round-robin service strategy.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename TraitsT>
class rr_service_strategy: public base_service_strategy<TraitsT>
{
	private: typedef base_service_strategy<TraitsT> base_type;
	private: typedef rr_service_strategy<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename base_type::real_type real_type;
	public: typedef typename base_type::uint_type uint_type;
	private: typedef typename base_type::customer_type customer_type;
	public: typedef typename base_type::customer_pointer customer_pointer;
	public: typedef ::dcs::math::stats::any_distribution<real_type> distribution_type;
	private: typedef ::std::vector<distribution_type> distribution_container;
	private: typedef typename customer_type::identifier_type customer_identifier_type;
	private: typedef ::std::deque<customer_identifier_type> customer_container;
	private: typedef ::std::vector<customer_container> server_container;
	private: typedef typename base_type::random_generator_type random_generator_type;
	private: typedef typename traits_type::class_identifier_type class_identifier_type;
	private: typedef typename base_type::runtime_info_type runtime_info_type;
	private: typedef typename traits_type::engine_type engine_type;
	private: typedef typename engine_traits<engine_type>::event_source_type event_source_type;
	private: typedef typename engine_traits<engine_type>::event_type event_type;
	private: typedef typename engine_traits<engine_type>::engine_context_type engine_context_type;
	private: typedef ::boost::shared_ptr<event_source_type> event_source_pointer;
	private: typedef ::boost::shared_ptr<event_type> event_pointer;
	private: typedef detail::quantum_expiry_event_state<real_type,uint_type> quantum_expiry_event_state_type;
	private: typedef ::std::map<uint_type,event_pointer> server_event_map;


	public: explicit rr_service_strategy(real_type quantum=1.0e-5)
	: base_type(),
	  q_(quantum),
	  ns_(1),
	  servers_(ns_),
	  num_busy_(0),
	  next_srv_(0),
	  ptr_quantum_expiry_evt_src_(new event_source_type("RR Quantum Exceeded")),
	  old_share_(0),
	  old_multiplier_(0)
	{
		init();
	}

	public: template <typename ForwardIterT>
		rr_service_strategy(real_type quantum, ForwardIterT first_distr, ForwardIterT last_distr)
	: base_type(),
	  q_(quantum),
	  ns_(1),
	  servers_(ns_),
	  distrs_(first_distr, last_distr),
	  num_busy_(0),
	  next_srv_(0),
	  ptr_quantum_expiry_evt_src_(new event_source_type("RR Quantum Exceeded")),
	  old_share_(0),
	  old_multiplier_(0)
	{
		init();
	}

	public: template <typename ClassForwardIterT, typename DistrForwardIterT>
		rr_service_strategy(real_type quantum, ClassForwardIterT first_class_id, ClassForwardIterT last_class_id, DistrForwardIterT first_distr)
	: base_type(),
	  q_(quantum),
	  ns_(1),
	  servers_(ns_),
	  num_busy_(0),
	  next_srv_(0),
	  ptr_quantum_expiry_evt_src_(new event_source_type("RR Quantum Exceeded")),
	  old_share_(0),
	  old_multiplier_(0)
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

		init();
	}

	public: template <typename ForwardIterT>
		rr_service_strategy(real_type quantum, ::std::size_t num_servers, ForwardIterT first_distr, ForwardIterT last_distr)
	: base_type(),
	  q_(quantum),
	  ns_(num_servers),
	  servers_(ns_),
	  num_busy_(0),
	  next_srv_(0),
	  ptr_quantum_expiry_evt_src_(new event_source_type("RR Quantum Exceeded")),
	  old_share_(0),
	  old_multiplier_(0)
	{
		while (first_distr != last_distr)
		{
			distrs_.push_back(*first_distr);
			++first_distr;
		}

		init();
	}

	public: template <typename ClassForwardIterT, typename DistrForwardIterT>
		rr_service_strategy(real_type quantum, ::std::size_t num_servers, ClassForwardIterT first_class_id, ClassForwardIterT last_class_id, DistrForwardIterT first_distr)
	: base_type(),
	  q_(quantum),
	  ns_(num_servers),
	  servers_(ns_),
	  num_busy_(0),
	  next_srv_(0),
	  ptr_quantum_expiry_evt_src_(new event_source_type("RR Quantum Exceeded")),
	  old_share_(0),
	  old_multiplier_(0)
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

		init();
	}

	public: real_type quantum() const
	{
		return q_;
	}


	public: event_source_type& quantum_expiry_event_source()
	{
		return *ptr_quantum_expiry_evt_src_;
	}

	public: event_source_type const& quantum_expiry_event_source() const
	{
		return *ptr_quantum_expiry_evt_src_;
	}

	private: void do_update_service()
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Do-Update-Service (Clock: " << this->node().network().engine().simulated_time() << ")");//XXX

		typedef typename server_event_map::iterator server_event_iterator;

		const real_type new_share(this->share());
		const real_type new_multiplier(this->capacity_multiplier());

		// Check if we need to update currently running customers
		if (::dcs::math::float_traits<real_type>::approximately_equal(old_share_, new_share)
			&& ::dcs::math::float_traits<real_type>::approximately_equal(old_multiplier_, new_multiplier))
		{
			// Share is not changed
			return;
		}

		engine_type& engine(this->node().network().engine());
		const real_type cur_time(engine.simulated_time());

		server_event_iterator end_it(srv_evt_map_.end());
		for (server_event_iterator it = srv_evt_map_.begin(); it != end_it; ++it)
		{
			const uint_type sid(it->first);
			event_pointer ptr_evt(it->second);

			// check: pointer to event must be a valid pointer
			DCS_DEBUG_ASSERT( ptr_evt );

			DCS_DEBUG_TRACE_L(3, "(" << this << ") Server ID: " << sid << " - QUANTUM-EXPIRY Event: " << *ptr_evt);

			quantum_expiry_event_state_type state = ptr_evt->template unfolded_state<quantum_expiry_event_state_type>();

			real_type new_fire_time(0);
			real_type time_to_fire(ptr_evt->fire_time()-cur_time);
			//real_type elapsed_time(cur_time-state.update_time);
			//real_type work_done(elapsed_time*old_share_);
			real_type work_done(state.work-time_to_fire*old_multiplier_);

			customer_identifier_type cid(servers_[sid].front());

			DCS_DEBUG_TRACE_L(3, "(" << this << ") Running Customer ID: " << cid);

			runtime_info_type& rt_info(this->info(cid));

			DCS_DEBUG_TRACE_L(3, "Updating Customer: " << rt_info.get_customer() << " - Service demand: " << rt_info.service_demand() << " - Multiplier: " << this->capacity_multiplier() << " - Quantum: " << this->quantum() << " - Elapsed Time: " << (cur_time-state.update_time) << " - Work done: " << work_done << " - old share: " << rt_info.share() << " - runtime: " << rt_info.runtime() << " - old completed work: " << rt_info.completed_work() << " - old residual-work: " << rt_info.residual_work() << " (Clock: " << engine.simulated_time() << ")");//XXX

			// Accumulate the work done to date (with the old share)
			//real_type old_residual_time(rt_info.residual_work()/old_share_);
			rt_info.accumulate_work2(work_done);
			//rt_info.accumulate_work_time(elapsed_time);
			real_type new_residual_time(rt_info.residual_work()/new_multiplier);

			// Set the new share
			rt_info.share(new_share);

			DCS_DEBUG_TRACE_L(3, "Updated Customer: " << rt_info.get_customer() << " - Service demand: " << rt_info.service_demand() << " - Multiplier: " << this->capacity_multiplier() << " - Quantum: " << this->quantum() << " - new share: " << rt_info.share() << " - new runtime: " << rt_info.runtime() << " - new completed work: " << rt_info.completed_work() << " - new residual-work: " << rt_info.residual_work() << " (Clock: " << engine.simulated_time() << ")");//XXX

			// Compute the maximum work time can be done before the quantum expires
			real_type max_residual_time(state.max_fire_time-cur_time);

			DCS_DEBUG_TRACE_L(3, "Updating State -> old work: " << state.work << " - old uptime-time: " << state.update_time << " - old early-expiry: " << ::std::boolalpha << state.early_expiry << " - old max-fire-time: " << state.max_fire_time << " (Clock: " << engine.simulated_time() << ")");//XXX

			state.update_time = cur_time;
			if (::dcs::math::float_traits<real_type>::definitely_less(new_residual_time, max_residual_time))
			{
				new_fire_time = cur_time+new_residual_time;
				state.early_expiry = true;
				state.work = rt_info.residual_work();
			}
			else
			{
				new_fire_time = state.max_fire_time;
				state.early_expiry = false;
				state.work = max_residual_time*new_share;
			}

			DCS_DEBUG_TRACE_L(3, "Updated State -> work: " << state.work << " - old uptime-time: " << state.update_time << " - old early-expiry: " << ::std::boolalpha << state.early_expiry << " - old max-fire-time: " << state.max_fire_time << " (Clock: " << engine.simulated_time() << ")");//XXX


			ptr_evt->state() = state;

			if (!::dcs::math::float_traits<real_type>::approximately_equal(new_fire_time, ptr_evt->fire_time()))
			{
				DCS_DEBUG_TRACE_L(3, "(" << this << ") Node: " << this->node() << " -- Reschedling QUANTUM-EXPIRTY -> Old Share: " << old_share_ << " - Current Share: " << this->share() << " - old work: " << (ptr_evt->template unfolded_state<quantum_expiry_event_state_type>().work) << " - new work: " << state.work << " - old fire-time: " << ptr_evt->fire_time() << " - new fire-time: " << new_fire_time << " (Clock: " << engine.simulated_time() << ")");//XXX

				// check: make sure the quantum does not last more than it should take
				//DCS_DEBUG_ASSERT( ::dcs::math::float_traits<real_type>::definitely_greater_equal(new_fire_time, state.max_fire_time) );
				DCS_DEBUG_ASSERT( ::dcs::math::float_traits<real_type>::definitely_greater_equal(state.max_fire_time, new_fire_time) );

//if (dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type> const&>(this->node().network().engine()).num_replications() == 2 && this->node().network().engine().simulated_time()>28900)//XXX
//{//XXX
//::std::cerr << "(" << this << ") Node: " << this->node() << " -- Reschedling QUANTUM-EXPIRTY -> Current Share: " << this->share() << " - old residual: " << old_residual << " - new residual: " << new_residual << " - old work: " << state.work << " - new work: " << ((state.work-old_residual)+new_residual) << " (Clock: " << this->node().network().engine().simulated_time() << ")" << ::std::endl;//XXX
//}//XXX
				// Reschedule the QUANTUM-EXPIRY event
				engine.reschedule_event(ptr_evt, new_fire_time);
			}
		}

		old_share_ = new_share;

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Do-Update-Service (Clock: " << this->node().network().engine().simulated_time() << ")");//XXX
	}

	private: bool do_can_serve() const
	{
		return true;
	}

	private: runtime_info_type do_serve(customer_pointer const& ptr_customer, random_generator_type& rng)
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Do-Service of Customer: " << *ptr_customer << " (Clock: " << this->node().network().engine().simulated_time() << ")");//XXX

		// pre: customer pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_customer );

		const real_type cur_time(this->node().network().engine().simulated_time());
		const real_type share(this->share());
		const real_type multiplier(this->capacity_multiplier());
		real_type svc_time(0);

		typename traits_type::class_identifier_type class_id = ptr_customer->current_class();

        while ((svc_time = ::dcs::math::stats::rand(distrs_[class_id], rng)) < 0) ;

		if (num_busy_ < ns_)
		{
			// The new customer get a dedicated server.
			// Round-Robin strategy still does not apply here.

			++num_busy_;
		}

		runtime_info_type rt_info(ptr_customer, cur_time, svc_time);
		rt_info.server_id(next_srv_);
		rt_info.share(share);
		rt_info.capacity_multiplier(multiplier);
		//rt_info.temporary(true);

		servers_[next_srv_].push_back(ptr_customer->id());

		// Check if we need to schedule the QUANTUM-EXPIRY event
		if (servers_[next_srv_].size() == 1)
		{
			// This is the first customer processed by this server.
			// So schedule the QUANTUM-EXPIRY event

			real_type delay(0);
			quantum_expiry_event_state_type state;
			state.sid = next_srv_;

			const real_type cust_quantum(quantum()*this->share());

			// Check if the actual quantum should be shorter than the regular
			// one. This happens when the runtime of the customer owing the
			// quantum is shorted than the quantum itself.
			if (::dcs::math::float_traits<real_type>::definitely_greater(cust_quantum, rt_info.runtime()))
			{
				state.work = rt_info.service_demand();
				state.early_expiry = true;
				delay = rt_info.runtime();
			}
			else
			{
				state.work = quantum()*share;
				state.early_expiry = false;
				delay = quantum();
			}
			state.update_time = cur_time;

			schedule_quantum_expiry(state, delay);
		} // else the event has been already scheduled before
//		else
//		{
//			//FIXME: this is a dirty trick!
//			// Make sure the runtime for this customer is sufficiently long to last until its turn
//			// Use the following relation:
//			//   runtime > quantum ---> service_demand/(a*share) > quantum*queue_length
//			// Hence:
//			//   a > service_demand/(share*quantum*queue_length)
//			// For instance:
//			//   a = service_demand/(share*(quantum*queue_length+1))
//
//			//rt_info.share(::std::min(share,svc_time/(quantum()*(servers_[next_srv_].size()+1))));
//			uint_type nc(servers_[next_srv_].size()+1);
//			rt_info.share(rt_info.service_demand()/(share*(quantum()*nc+1)));
//		}

		next_srv_ = next_server(next_srv_);

		//FIXME: this is a dirty trick, used for letting the strategy to schedule to real end-of-service.
		//       A share of zero means a runtime of Infinite
		rt_info.share(0); //EXPERIMENTAL

		DCS_DEBUG_TRACE_L(3, "(" << this << ") Generated service for customer: " << *ptr_customer << " - Service demand: " << rt_info.service_demand() << " - Multiplier: " << this->capacity_multiplier() << " - Share: " << share << " - Runtime: " << rt_info.runtime() << " - Server: " << next_srv_ << " (Clock: " << this->node().network().engine().simulated_time() << ")");//XXX
//if (dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type> const&>(this->node().network().engine()).num_replications() == 2 && this->node().network().engine().simulated_time()>28900)//XXX
//{//XXX
//::std::cerr << "(" << this << ") Node: " << this->node() << " -- Generated service for customer: " << *ptr_customer << " - Service demand: " << rt_info.service_demand() << " - Multiplier: " << this->capacity_multiplier() << " - Share: " << share << " - Runtime: " << rt_info.runtime() << " - Server: " << next_srv_ << " (Clock: " << this->node().network().engine().simulated_time() << ")" << ::std::endl;//XXX
//}//XXX

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Do-Service of Customer: " << *ptr_customer << " (Clock: " << this->node().network().engine().simulated_time() << ")");//XXX

		return rt_info;
	}

	private: void do_remove(customer_pointer const& ptr_customer)
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Do-Remove of Customer: " << *ptr_customer << " (Clock: " << this->node().network().engine().simulated_time() << ")");//XXX
//if (dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type> const&>(this->node().network().engine()).num_replications() == 2 && this->node().network().engine().simulated_time()>28900)//XXX
//{//XXX
//::std::cerr << "(" << this << ") Node: " << this->node() << " -- BEGIN Do-Remove of Customer: " << *ptr_customer << " (Clock: " << this->node().network().engine().simulated_time() << ")" << ::std::endl;//XXX
//}//XXX

		// precondition: customer pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_customer,
			throw ::std::invalid_argument("[dcs::des::model::qn::rr_service_strategy::do_remove] Null pointer to a customer.")
		);

		// Retrieve the id of this customer
		customer_identifier_type cid(ptr_customer->id());

		// Retrieve the server assigned to this customer
		uint_type sid(this->info(cid).server_id());

//		// check: the customer removed is the customer currently in execution
//		DCS_DEBUG_ASSERT( cid == servers_[sid].front() );
//
//		// Erase the associated service info 
//		servers_[sid].pop_front();
		if (servers_[sid].size() == 0)
		{
			--num_busy_;
		}

		next_srv_ = next_server(sid);

//if (dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type> const&>(this->node().network().engine()).num_replications() == 2 && this->node().network().engine().simulated_time()>28900)//XXX
//{//XXX
//::std::cerr << "(" << this << ") Node: " << this->node() << " -- END Do-Remove of Customer: " << *ptr_customer << " (Clock: " << this->node().network().engine().simulated_time() << ")" << ::std::endl;//XXX
//}//XXX
		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Do-Remove of Customer: " << *ptr_customer << " (Clock: " << this->node().network().engine().simulated_time() << ")");//XXX
	}

	private: void do_remove_all()
	{
		servers_.clear();
		servers_.resize(ns_);
		num_busy_ = next_srv_
				  = uint_type/*zero*/();
		srv_evt_map_.clear(); //FIXME: should we really do this?
	}

	private: void do_reset()
	{
		servers_.clear();
		servers_.resize(ns_);
		num_busy_ = next_srv_
				  = uint_type/*zero*/();
		srv_evt_map_.clear();
		old_share_ = this->share();
		old_multiplier_ = this->capacity_multiplier();
	}

	private: uint_type do_num_servers() const
	{
		return ns_;
	}

	private: uint_type do_num_busy_servers() const
	{
		return num_busy_;
	}

	private: void init()
	{
		ptr_quantum_expiry_evt_src_->connect(
				::dcs::functional::bind(
					&self_type::process_quantum_expiry,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
	}

	private: uint_type next_server(uint_type start_sid) const
	{
		uint_type best_sid(start_sid);

		if (ns_ > 1 && servers_[start_sid].size() > 0)
		{
			// choose the server with the smallest number of served customers
			uint_type best_sid_size(servers_[best_sid].size());
			for (uint_type i = 1; i < ns_ && best_sid_size > 0; ++i)
			{
				uint_type sid((start_sid+i) % ns_);
				if (servers_[sid].size() < servers_[best_sid].size())
				{
					best_sid = sid;
					best_sid_size = servers_[best_sid].size();
				}
			}
		}

		return best_sid;
	}

	private: void schedule_quantum_expiry(quantum_expiry_event_state_type& state, real_type delay)
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Scheduling QUANTUM-EXPIRY for State <sid: " << state.sid << ",work: " << state.work << "> and Customer: " << servers_[state.sid].front() << " at Node: " << this->node() << " with Delay: " << delay << " (Clock: " << this->node().network().engine().simulated_time() << ")"); //XXX

		real_type cur_time(this->node().network().engine().simulated_time());

		event_pointer ptr_evt;

		// To prevent successive reschedulings to go beyond the quantum limit...
		state.max_fire_time = cur_time+quantum();

		ptr_evt = this->node().network().engine().schedule_event(
				ptr_quantum_expiry_evt_src_, 
				cur_time+delay,
				state
			);

		DCS_DEBUG_TRACE_L(3, "Node: " << this->node() << " -- Scheduled next QUANTUM-EXPIRY: " << *ptr_evt << " at " << (this->node().network().engine().simulated_time()+delay) << " (Clock: " << this->node().network().engine().simulated_time() << ")");//XXX

//if (dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type> const&>(this->node().network().engine()).num_replications() == 2 && this->node().network().engine().simulated_time()>28900)//XXX
//{//XXX
//::std::cerr << "Node: " << this->node() << " -- Scheduled next QUANTUM-EXPIRY: " << *ptr_evt << " at " << (this->node().network().engine().simulated_time()+delay) << " (Clock: " << this->node().network().engine().simulated_time() << ")" << ::std::endl;//XXX
//}//XXX

		srv_evt_map_[state.sid] = ptr_evt;

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Scheduling QUANTUM-EXPIRY for State <sid: " << state.sid << ",work: " << state.work << "> and Customer: " << servers_[state.sid].front() << " at Node: " << this->node() << " with Delay: " << delay << " (Clock: " << this->node().network().engine().simulated_time() << ")"); //XXX
	}

	private: void process_quantum_expiry(event_type const& evt, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		quantum_expiry_event_state_type state = evt.template unfolded_state<quantum_expiry_event_state_type>();

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Processing QUANTUM-EXPIRY with State <sid: " << state.sid << ", work: " << state.work << ", update_time: " << state.update_time << "> at Node: " << this->node() << " (Clock: " << this->node().network().engine().simulated_time() << ")"); //XXX

		DCS_DEBUG_ASSERT( servers_[state.sid].size() > 0 );

		real_type cur_time(this->node().network().engine().simulated_time());
		uint_type sid(state.sid);
		customer_identifier_type cid(servers_[sid].front());
		servers_[sid].pop_front();

		DCS_DEBUG_TRACE_L(3, "Current Customer ID: " << cid);//XXX
//if (dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type> const&>(this->node().network().engine()).num_replications() == 2 && this->node().network().engine().simulated_time()>28900)//XXX
//{//XXX
//::std::cerr << "Node: " << this->node() << " -- Current Customer ID: " << cid << " (Clock: " << this->node().network().engine().simulated_time() << ")" << ::std::endl;//XXX
//}//XXX

		runtime_info_type& rt_info(this->info(cid));

		DCS_DEBUG_TRACE_L(3, "Updating Customer: " << rt_info.get_customer() << " - Service demand: " << rt_info.service_demand() << " - Multiplier: " << this->capacity_multiplier() << " - Quantum: " << this->quantum() << " - State.Work: " << state.work << " - old share: " << rt_info.share() << " - runtime: " << rt_info.runtime() << " - old completed work: " << rt_info.completed_work() << " - old residual-work: " << rt_info.residual_work() << " (Clock: " << this->node().network().engine().simulated_time() << ")");//XXX
//if (dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type> const&>(this->node().network().engine()).num_replications() == 2 && this->node().network().engine().simulated_time()>28900)//XXX
//{//XXX
//::std::cerr << "Node: " << this->node() << " -- Updating Customer: " << rt_info.get_customer() << " - Service demand: " << rt_info.service_demand() << " - Multiplier: " << this->capacity_multiplier() << " - Quantum: " << this->quantum() << " - State.Work: " << state.work << " - old share: " << rt_info.share() << " - runtime: " << rt_info.runtime() << " - old completed work: " << rt_info.completed_work() << " - old residual-work: " << rt_info.residual_work() << " (Clock: " << this->node().network().engine().simulated_time() << ")" << ::std::endl;//XXX
//}//XXX

//		// check: paranoid check
//		DCS_DEBUG_ASSERT( ::std::abs(state.work-(cur_time-state.update_time)*this->share()) <= ::std::numeric_limits<real_type>::epsilon() );

		rt_info.share(this->share());
		rt_info.accumulate_work2(state.work);
		//rt_info.accumulate_work_time(cur_time-state.update_time);

		real_type residual_work(rt_info.residual_work());
		if (::dcs::math::float_traits<real_type>::definitely_greater(residual_work, static_cast<real_type>(0)))
		{
			// This customer still need some more time to finish.
			// Execution will continue on the next RR tournament.
			servers_[sid].push_back(cid);
		} // else this customer is done

		DCS_DEBUG_TRACE_L(3, "Updated Customer: " << rt_info.get_customer() << " - Service demand: " << rt_info.service_demand() << " - Multiplier: " << this->capacity_multiplier() << " - Quantum: " << this->quantum() << " - new share: " << rt_info.share() << " - new runtime: " << rt_info.runtime() << " - new completed work: " << rt_info.completed_work() << " - new residual-work: " << rt_info.residual_work() << " (Clock: " << this->node().network().engine().simulated_time() << ")");//XXX
//if (dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type> const&>(this->node().network().engine()).num_replications() == 2 && this->node().network().engine().simulated_time()>28900)//XXX
//{//XXX
//::std::cerr <<  "Node: " << this->node() << " -- Updated Customer: " << rt_info.get_customer() << " - Service demand: " << rt_info.service_demand() << " - Multiplier: " << this->capacity_multiplier() << " - Quantum: " << this->quantum() << " - new share: " << rt_info.share() << " - new runtime: " << rt_info.runtime() << " - new completed work: " << rt_info.completed_work() << " - new residual-work: " << rt_info.residual_work() << " (Clock: " << this->node().network().engine().simulated_time() << ")" << ::std::endl;//XXX
//}//XXX

		// Look ahead for the residual time of the next customer to make sure it
		// will not execute more than it need.
		if (servers_[sid].size() > 0)
		{
			customer_identifier_type next_cid = servers_[sid].front();

			DCS_DEBUG_TRACE_L(3, "Next Customer ID: " << next_cid << " (Clock: " << this->node().network().engine().simulated_time() << ")");//XXX
//if (dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type> const&>(this->node().network().engine()).num_replications() == 2 && this->node().network().engine().simulated_time()>28900)//XXX
//{//XXX
//::std::cerr << "Node: " << this->node() << " -- Next Customer ID: " << next_cid << " (Clock: " << this->node().network().engine().simulated_time() << ")" << ::std::endl;//XXX
//}//XXX

			runtime_info_type& next_rt_info(this->info(next_cid));
			next_rt_info.share(this->share());
			next_rt_info.capacity_multiplier(this->capacity_multiplier());
			real_type residual_time(next_rt_info.residual_work()/this->capacity_multiplier());
			real_type delay(0);
			//state.work = ::std::min(next_rt_info.residual_work(), quantum());
			if (::dcs::math::float_traits<real_type>::definitely_greater(quantum(), residual_time))
			{
				DCS_DEBUG_TRACE("Quantum (" << quantum() << ") > Real Residual Work Time (" << residual_time << ")");//XXX

				state.work = next_rt_info.residual_work();
				state.early_expiry = true;
				delay = residual_time;
			}
			else
			{
				DCS_DEBUG_TRACE("Quantum (" << quantum() << ") < Real Residual Work Time (" << residual_time << ")");//XXX

				state.work = quantum()*this->share();
				state.early_expiry = false;
				delay = quantum();
			}
			state.update_time = cur_time;

			DCS_DEBUG_TRACE_L(3, "Next Customer: " << next_rt_info.get_customer() << " - Service demand: " << next_rt_info.service_demand() << " - Multiplier: " << this->capacity_multiplier() << " - Quantum: " << this->quantum() << " - share: " << next_rt_info.share() << " - runtime: " << next_rt_info.runtime() << " - completed work: " << next_rt_info.completed_work() << " - residual-work: " << next_rt_info.residual_work() << " (Clock: " << this->node().network().engine().simulated_time() << ")");//XXX
//if (dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type> const&>(this->node().network().engine()).num_replications() == 2 && this->node().network().engine().simulated_time()>28900)//XXX
//{//XXX
//::std::cerr << "Node: " << this->node() << " -- Next Customer: " << next_rt_info.get_customer() << " - Service demand: " << next_rt_info.service_demand() << " - Multiplier: " << this->capacity_multiplier() << " - Quantum: " << this->quantum() << " - share: " << next_rt_info.share() << " - runtime: " << next_rt_info.runtime() << " - completed work: " << next_rt_info.completed_work() << " - residual-work: " << next_rt_info.residual_work() << " (Clock: " << this->node().network().engine().simulated_time() << ")" << ::std::endl;//XXX
//::std::cerr << "Node: " << this->node() << " -- Scheduling next QUANTUM-EXPIRY for Customer ID: " << cid << " at " << (this->node().network().engine().simulated_time()+state.work) << " (Clock: " << this->node().network().engine().simulated_time() << ")" << ::std::endl;//XXX
//}//XXX

			schedule_quantum_expiry(state, delay);
		}
		else
		{
			srv_evt_map_.erase(sid);
		}

////if (dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type> const&>(this->node().network().engine()).num_replications() == 2 && this->node().network().engine().simulated_time()>28900)//XXX
////{//XXX
////::std::cerr << "Node: " << this->node() << " -- Rescheduling End-of-Service of Customer ID: " << cid << " at " << (this->node().network().engine().simulated_time()+delay) << " (Clock: " << this->node().network().engine().simulated_time() << ")" << ::std::endl;//XXX
////}//XXX
//		this->node().reschedule_service(rt_info.get_customer(), delay);
		if (::dcs::math::float_traits<real_type>::approximately_equal(residual_work, static_cast<real_type>(0)))
		{
			DCS_DEBUG_TRACE_L(3, "Node: " << this->node() << " -- Rescheduling End-of-Service of Customer ID: " << cid << " NOW (Clock: " << this->node().network().engine().simulated_time() << ")");//XXX
//if (dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type> const&>(this->node().network().engine()).num_replications() == 2 && this->node().network().engine().simulated_time()>28900)//XXX
//{//XXX
//::std::cerr << "Node: " << this->node() << " -- Rescheduling End-of-Service of Customer ID: " << cid << " NOW (Clock: " << this->node().network().engine().simulated_time() << ")" << ::std::endl;//XXX
//}//XXX
			this->node().reschedule_service(rt_info.get_customer(), 0);
		}

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Processing QUANTUM-EXPIRY at Node: " << this->node() << " (Clock: " << this->node().network().engine().simulated_time() << ")"); //XXX
	}


	/// The quantum
	private: real_type q_;
	/// The total number of servers.
	private: uint_type ns_;
	/// The servers container. For each server, it maintains the list of customers currently running on it.
	private: server_container servers_;
	/// The service distributions container.
	private: distribution_container distrs_;
	/// The number of current busy severs.
	private: uint_type num_busy_;
	/// The next server used to assign a new customer.
	private: uint_type next_srv_;
	/// Source for QUANTUM-EXCEEDED events.
	private: event_source_pointer ptr_quantum_expiry_evt_src_;
	/// Hold <server-id,quantum-event-pointer> associations
	private: server_event_map srv_evt_map_;
	private: real_type old_share_;//FIXME: experimental
	private: real_type old_multiplier_;//FIXME: experimental

	//@} Data members
};

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_RR_SERVICE_STRATEGY_HPP
