/**
 * \file dcs/des/model/queue/open_queue.hpp
 *
 * \brief Model for open queue (i.e., with infinite population).
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

#ifndef DCS_DES_MODEL_OPEN_QUEUE_HPP
#define DCS_DES_MODEL_OPEN_QUEUE_HPP


#include <algorithm>
#include <boost/smart_ptr.hpp>
#include <cstddef>
#include <dcs/debug.hpp>
#include <dcs/des/any_statistic.hpp>
#include <dcs/des/engine.hpp>
#include <dcs/des/engine_context.hpp>
#include <dcs/des/event.hpp>
#include <dcs/des/event_source.hpp>
#include <dcs/des/model/queue/fifo_queue_policy.hpp>
#include <dcs/des/model/queue/queue_job.hpp>
#include <dcs/des/model/queue/queue_statistics.hpp>
#include <dcs/des/utility.hpp>
#include <dcs/iterator/any_forward_iterator.hpp>
#include <dcs/iterator/iterator_range.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/math/random/any_generator.hpp>
#include <dcs/math/stats/distribution/any_distribution.hpp>
#include <limits>
#include <map>
#include <sstream>
#include <utility>
#include <vector>


namespace dcs { namespace des { namespace model {

/**
 * \brief Model for open queue (i.e., with infinite population).
 *
 * \tparam RealT The type used for real numbers.
 * \tparam UIntT The type used for unsigned integral numbers.
 * \tparam ArrivalDistributionT The interarrival-time distribution type.
 * \tparam ServiceDistributionT The service-time distribution type.
 * \tparam UniformRandomGeneratorT The uniform random number generator type
 * \tparam QueuePolicyT The policy type used for choosing the next request to serve.
 * \tparam DesEngineT The discrete-event simulation engine type.
 * \tparam OutputStatisticT The base type for output statistics.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <
	typename RealT=double,
	typename UIntT=::std::size_t, 
	typename ArrivalDistributionT=::dcs::math::stats::any_distribution<RealT>,
	typename ServiceDistributionT=::dcs::math::stats::any_distribution<RealT>,
	typename UniformRandomGeneratorT=::dcs::math::random::any_generator<RealT>,
	typename QueuePolicyT=fifo_queue_policy< queue_job<RealT> >,
	typename DesEngineT=::dcs::des::engine<RealT>,
	typename OutputStatisticT=::dcs::des::any_statistic<RealT,UIntT>
>
class open_queue
{
	/// This type
	private: typedef open_queue<RealT,UIntT,ArrivalDistributionT,ServiceDistributionT,UniformRandomGeneratorT,QueuePolicyT,DesEngineT,OutputStatisticT> self_type;
	/// The type used for real numbers.
	public: typedef RealT real_type;
	/// The type used for unsigned integral numbers.
	public: typedef UIntT uint_type;
	/// The interarrival-time distribution type (model of ProbabilityDistribution concept).
	public: typedef ArrivalDistributionT arrival_distribution_type;
	/// The service-time distribution type (model of ProbabilityDistribution concept).
	public: typedef ServiceDistributionT service_distribution_type;
	/// The uniform random number generator type (model of UniformRandomNumberGenerator concept).
	public: typedef UniformRandomGeneratorT random_generator_type;
	/// The queue policy type.
	public: typedef QueuePolicyT queue_policy_type;
	/// The discrete-event simulation engine type.
	public: typedef DesEngineT des_engine_type;
	/// The output statistic type
	private: typedef OutputStatisticT output_statistic_type;
	/// The type of the events fired by the simulation engine.
	private: typedef typename des_engine_type::event_type des_event_type;
	/// The type of the context passed to events fired by the simulation engine.
	private: typedef typename des_engine_type::engine_context_type des_engine_context_type;
	/// The type of the source of the events that the simulation engine uses for firing events.
	private: typedef typename des_event_type::event_source_type des_event_source_type;
	/// The type of the mean estimators.
	private: typedef typename queue_policy_type::value_type queue_job_type;
	private: typedef ::boost::shared_ptr<output_statistic_type> output_statistic_pointer;
	private: typedef ::std::vector<output_statistic_pointer> output_statistic_container;
	private: typedef ::std::map<queue_statistics,output_statistic_container> output_statistic_category_container;


	/// Tags of captured events (only internally used).
	private: enum event_tags
	{
		arrival_event_tag, ///< The arrival event tag.
		departure_event_tag, ///< The departure event tag.
		discard_event_tag ///< The discard event tag.
	};


	/// A constructor.
	public: open_queue(arrival_distribution_type const& iatime_distr, service_distribution_type const& svctime_distr, uint_type num_servers, bool queue_is_finite, uint_type max_queue_length, random_generator_type& rng, ::boost::shared_ptr<des_engine_type> const& ptr_eng)
		: ptr_arrival_evt_src_(new des_event_source_type()),
		  ptr_departure_evt_src_(new des_event_source_type()),
		  ptr_discard_evt_src_(new des_event_source_type()),
		  iatime_distr_(iatime_distr),
		  svctime_distr_(svctime_distr),
		  num_srv_(num_servers),
		  queue_is_finite_(queue_is_finite),
		  max_queue_len_(max_queue_length),
		  rng_(rng),
		  ptr_eng_(ptr_eng),
		  queue_(),
		  svc_share_(1)
	{
		init();
	}


	public: open_queue(open_queue const& that)
	{
		ptr_arrival_evt_src_ = ::boost::shared_ptr<des_event_source_type>(new des_event_source_type(*(that.ptr_arrival_evt_src_)));
		ptr_departure_evt_src_ = ::boost::shared_ptr<des_event_source_type>(new des_event_source_type(*(that.ptr_departure_evt_src_)));
		ptr_discard_evt_src_ = ::boost::shared_ptr<des_event_source_type>(new des_event_source_type(*(that.ptr_discard_evt_src_)));
		iatime_distr_ = that.iatime_distr_;
		svctime_distr_ = that.svctime_distr_;
		num_srv_ = that.num_srv_;
		queue_is_finite_ = that.queue_is_finite_;
		max_queue_len_ = that.max_queue_len_;
		rng_ = that.rng_;
		ptr_eng_ = that.ptr_eng_;
		queue_ = that.queue_;
		svc_share_ = that.svc_share_;
		num_arrivals_ = that.num_arrivals_;
		num_departures_ = that.num_departures_;
		num_discards_ = that.num_discards_;
		cur_num_busy_ = that.cur_num_busy_;
		stats_.clear();
		typedef typename output_statistic_category_container::const_iterator stat_outer_iterator;
		typedef typename output_statistic_container::const_iterator stat_inner_iterator;
		stat_outer_iterator stat_outer_end_it(that.stats_.end());
		for (
			stat_outer_iterator stat_outer_it = that.stats_.begin();
			stat_outer_it != stat_outer_end_it;
			++stat_outer_it
		) {
			queue_statistics category(stat_outer_it->first);
			stat_inner_iterator stat_inner_end_it(stat_outer_it->second.end());
			for (
				stat_inner_iterator stat_inner_it = stat_outer_it->second.begin();
				stat_inner_it != stat_inner_end_it;
				++stat_inner_it
			) {
				stats_[category].push_back(*stat_inner_it);
			}
		}
		cur_evt_tag_ = that.cur_evt_tag_;
		last_evt_time_ = that.last_evt_time_;

		init();
	}


	public: virtual ~open_queue()
	{
//		arrivals_slot_.disconnect();
//		departures_slot_.disconnect();
//		discards_slot_.disconnect();
//		bos_slot_.disconnect();

		ptr_arrival_evt_src_->disconnect(
			::dcs::functional::bind(
				&self_type::process_arrival,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);

		ptr_departure_evt_src_->disconnect(
			::dcs::functional::bind(
				&self_type::process_departure,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);

		ptr_discard_evt_src_->disconnect(
			::dcs::functional::bind(
				&self_type::process_discard,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ptr_eng_->system_initialization_event_source().disconnect(
			::dcs::functional::bind(
				&self_type::process_sys_init,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


	public: open_queue const& operator=(open_queue const& rhs)
	{
		if (this != &rhs)
		{
			ptr_arrival_evt_src_ = ::boost::shared_ptr<des_event_source_type>(new des_event_source_type(*(rhs.ptr_arrival_evt_src_)));
			ptr_departure_evt_src_ = ::boost::shared_ptr<des_event_source_type>(new des_event_source_type(*(rhs.ptr_departure_evt_src_)));
			ptr_discard_evt_src_ = ::boost::shared_ptr<des_event_source_type>(new des_event_source_type(*(rhs.ptr_discard_evt_src_)));
			iatime_distr_ = rhs.iatime_distr_;
			svctime_distr_ = rhs.svctime_distr_;
			num_srv_ = rhs.num_srv_;
			queue_is_finite_ = rhs.queue_is_finite_;
			max_queue_len_ = rhs.max_queue_len_;
			rng_ = rhs.rng_;
			ptr_eng_ = rhs.ptr_eng_;
			queue_ = rhs.queue_;
			svc_share_ = rhs.svc_share_;
			num_arrivals_ = rhs.num_arrivals_;
			num_departures_ = rhs.num_departures_;
			num_discards_ = rhs.num_discards_;
			cur_num_busy_ = rhs.cur_num_busy_;
			stats_.clear();
			typedef typename output_statistic_category_container::const_iterator stat_outer_iterator;
			typedef typename output_statistic_container::const_iterator stat_inner_iterator;
			stat_outer_iterator stat_outer_end_it(rhs.stats_.end());
			for (
				stat_outer_iterator stat_outer_it = rhs.stats_.begin();
				stat_outer_it != stat_outer_end_it;
				++stat_outer_it
			) {
				queue_statistics category(stat_outer_it->first);
				stat_inner_iterator stat_inner_end_it(stat_outer_it->second.end());
				for (
					stat_inner_iterator stat_inner_it = stat_outer_it->second.begin();
					stat_inner_it != stat_inner_end_it;
					++stat_inner_it
				) {
					stats_[category].push_back(*stat_inner_it);
				}
			}
			cur_evt_tag_ = rhs.cur_evt_tag_;
			last_evt_time_ = rhs.last_evt_time_;

			init();
		}

		return *this;
	}


	public: des_event_source_type& arrival_event_source()
	{
		return *ptr_arrival_evt_src_;
	}


	public: des_event_source_type& arrival_event_source() const
	{
		return *ptr_arrival_evt_src_;
	}


	public: des_event_source_type& departure_event_source()
	{
		return *ptr_departure_evt_src_;
	}


	public: des_event_source_type& departure_event_source() const
	{
		return *ptr_departure_evt_src_;
	}


	public: des_event_source_type& discard_event_source()
	{
		return *ptr_discard_evt_src_;
	}


	public: des_event_source_type& discard_event_source() const
	{
		return *ptr_discard_evt_src_;
	}


	public: void service_share(real_type value)
	{
		svc_share_ = value;
	}


	public: real_type service_share() const
	{
		return svc_share_;
	}


//	public: simulation_stats_type simulation_result() const
//	{
//		//TODO: check that the simulation has been started and is really completed
//		return create_stats_result();
//	}


	/**
	 * \brief Compute the total number of client request arrivals.
	 * \return The total number of client request arrivals.
	 */
	public: uint_type num_arrivals() const
	{
		return num_arrivals_;
	}


	/**
	 * \brief Compute the total number of client request departures.
	 * \return The total number of client request departures.
	 */
	public: uint_type num_departures() const
	{
		return num_departures_;
	}


	/**
	 * \brief Compute the total number of client request discards.
	 * \return The total number of client request discards.
	 */
	public: uint_type num_discards() const
	{
		return num_discards_;
	}


	public: void statistic(queue_statistics const& statistic_tag, ::boost::shared_ptr<output_statistic_type> const& ptr_stat)
	{
		stats_[statistic_tag].push_back(ptr_stat);
	}


	//FIXME: the use of const specifier make compilation to fail... probably the problem is in any_forward_iterator
	public: ::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator< ::boost::shared_ptr<output_statistic_type> > > statistic(queue_statistics statistic_tag)// const
	{
		if (stats_.at(statistic_tag).size() == 0)
		{
			::std::ostringstream oss;
			oss << "Statistic " << statistic_tag << " not registered";
			throw ::std::runtime_error(oss.str());
		}

		return ::dcs::iterator::make_iterator_range(
					::dcs::iterator::make_any_forward_iterator(
						stats_.at(statistic_tag).begin()
					),
					::dcs::iterator::make_any_forward_iterator(
						stats_.at(statistic_tag).end()
					)
		);
	}


	public: void reset()
	{
//		queue_ = queue_policy_type();
//
//		num_arrivals_ = num_departures_
//					  = num_discards_
//					  = cur_num_busy_
//					  = uint_type(0);
//
////		area_ = real_type(0);
//		last_evt_time_ = real_type(0);
//
////Don't reset external stats
////		reset_stat(busy_time_queue_statistic);
////		reset_stat(interarrival_time_queue_statistic);
////		reset_stat(num_busy_queue_statistic);
////		reset_stat(num_waiting_queue_statistic);
////		reset_stat(response_time_queue_statistic);
////		reset_stat(service_time_queue_statistic);
////		reset_stat(throughput_queue_statistic);
////		reset_stat(utilization_queue_statistic);
////		reset_stat(waiting_time_queue_statistic);
//
//		schedule_arrival();
	}


//	public: ::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator<output_statistic_type> > statistic(queue_statistics const& statistic_tag)
//	{
//		return ::dcs::iterator::make_iterator_range(
//					::dcs::iterator::make_any_forward_iterator(
//						stats_[statistic_tag].begin()
//					),
//					::dcs::iterator::make_any_forward_iterator(
//						stats_[statistic_tag].end()
//					)
//		);
//	}


//	public: template <typename OutputStatisticTag>
//		output_statistic_type const& statistic(queue_statistics const& queue_statistic_tag, OutputStatisticTag const& statistic_tag) const
//	{
//		for (
//			std::vector< ::boost::shared_ptr<output_statistic_type> >::const_iterator it = stats_[statistic_tag].begin();
//			it != stats_[statistic_tag].end();
//			++it
//		) {
//			if (detail::is_type<it::statistic_category,OutputStatisticTag>::value)
//			{
//				return *(*it);
//			}
//	}


	private: void init()
	{
		//arrivals_slot_ = ptr_arrival_evt_src_->connect(
		ptr_arrival_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_arrival,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);

		//departures_slot_ = ptr_departure_evt_src_->connect(
		ptr_departure_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_departure,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);

		//discards_slot_ = ptr_discard_evt_src_->connect(
		ptr_discard_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_discard,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);

		ptr_eng_->system_initialization_event_source().connect(
			::dcs::functional::bind(
				&self_type::process_sys_init,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


//	private: void prepare_simulation(des_event_type const& evt, des_engine_context_type& ctx)
//	{
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(evt);
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ctx);
//
//		queue_ = queue_policy_type();
//
//		num_arrivals_ = num_departures_
//					  = num_discards_
//					  = cur_num_busy_
//					  = uint_type(0);
//
////		area_ = real_type(0);
//		last_evt_time_ = real_type(0);
//
//		reset_stat(busy_time_queue_statistic);
//		reset_stat(interarrival_time_queue_statistic);
//		reset_stat(num_busy_queue_statistic);
//		reset_stat(num_waiting_queue_statistic);
//		reset_stat(response_time_queue_statistic);
//		reset_stat(service_time_queue_statistic);
//		reset_stat(throughput_queue_statistic);
//		reset_stat(utilization_queue_statistic);
//		reset_stat(waiting_time_queue_statistic);
//
//		schedule_arrival();
//	}


	private: void process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(evt);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ctx);

		queue_ = queue_policy_type();

		num_arrivals_ = num_departures_
					  = num_discards_
					  = cur_num_busy_
					  = uint_type(0);

//		area_ = real_type(0);
		last_evt_time_ = real_type(0);

//Don't reset external stats
//		reset_stat(busy_time_queue_statistic);
//		reset_stat(interarrival_time_queue_statistic);
//		reset_stat(num_busy_queue_statistic);
//		reset_stat(num_waiting_queue_statistic);
//		reset_stat(response_time_queue_statistic);
//		reset_stat(service_time_queue_statistic);
//		reset_stat(throughput_queue_statistic);
//		reset_stat(utilization_queue_statistic);
//		reset_stat(waiting_time_queue_statistic);

		schedule_arrival();
	}


	private: void accumulate_stat(queue_statistics const& statistic_tag, real_type value)
	{
//		::std::for_each(
//			stats_[statistic_tag].begin(),
//			stats_[statistic_tag].end(),
//			::dcs::functional::bind(
//				&output_statistic_type::operator(),
//				::dcs::functional::placeholders::_1,
//				value
//			)
//		);

		if (!stats_.count(statistic_tag))
		{
			return;
		}

		typedef typename output_statistic_container::iterator stat_iterator;
		stat_iterator end_it(stats_.at(statistic_tag).end());
		for (stat_iterator it = stats_.at(statistic_tag).begin(); it != end_it; ++it)
		{
			output_statistic_pointer ptr_stat(*it);

			// paranoid-check: null
			DCS_DEBUG_ASSERT( ptr_stat );

			(*ptr_stat)(value);
		}
	}


	private: void reset_stat(queue_statistics const& statistic_tag)
	{
		if (!stats_.count(statistic_tag))
		{
			return;
		}

		::std::for_each(
			stats_[statistic_tag].begin(),
			stats_[statistic_tag].end(),
			::dcs::functional::bind(
				&output_statistic_type::reset,
				::dcs::functional::placeholders::_1
			)
		);
	}


	/// Update common statistics.
	private: void update_stats(des_engine_context_type& ctx)
	{
		// The time spent in the current state of the system
		//real_type interval = ctx.simulated_time() - ctx.last_event_time(); //NO: ctx.last_event_time() may be the time of an event associated to another queue
		real_type interval = ctx.simulated_time() - last_evt_time_;

		// Update the number of waiting requests.
		uint_type cur_num_wait = queue_.size();
//		if (queue_.size() > 0)
//		{
//			// We subtract 1 to queue size since we don't include the job that
//			// is currently served.
//			cur_num_wait = queue_.size()-1;
//		}

		//uint_type cur_num_sys = cur_num_wait+cur_num_busy_;

//		//area_ += interval*(num_arrivals_-num_departures_-num_discards_);
//		area_ += interval*cur_num_wait;

		if (cur_num_busy_ > 0)
		{
			accumulate_stat(busy_time_queue_statistic, interval);
			accumulate_stat(num_busy_queue_statistic, cur_num_busy_);
			//accumulate_stat(utilization_queue_statistic, interval/(ctx.simulated_time()-last_evt_time_));
			accumulate_stat(utilization_queue_statistic, real_type(1)/cur_num_busy_);
		}
		else
		{
			accumulate_stat(busy_time_queue_statistic, 0);
			accumulate_stat(num_busy_queue_statistic, 0);
			accumulate_stat(utilization_queue_statistic, 0);
		}

		accumulate_stat(num_waiting_queue_statistic, cur_num_wait);
		accumulate_stat(throughput_queue_statistic, num_departures_/ctx.simulated_time());
//		if (cur_evt_tag_ == departure_event_tag)
//		{
//			accumulate_stat(throughput_queue_statistic, real_type(1)/interval);
//		}
//		else
//		{
//			accumulate_stat(throughput_queue_statistic, 0);
//		}

		last_evt_time_ = ctx.simulated_time();
	}


	/**
	 * \brief Callback for handling the arrival event.
	 * \param evt The event fired by the arrival event source.
	 * \param ctx The DES engine context.
	 */
	private: void process_arrival(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_DEBUG_TRACE("Processing ARRIVAL event: " << evt << " for QUEUE object: " << this);

		cur_evt_tag_ = arrival_event_tag;
		++num_arrivals_;

		// Check if there is enough space in the queue
		if (queue_is_finite_ && (queue_.size() == max_queue_len_))
		{
			// Discard current arrival
			//++num_discards_;
			schedule_discard();
		}
		else
		{
			// Queue this arrival
			queue_job_type job;
			job.arrival_time(evt.fire_time());
			//queue_.push(evt.fire_time());
			queue_.push(job);

			// There is some server idle, so serve the next job and schedule a departure
			if (cur_num_busy_ < num_srv_)
			{
				schedule_departure();
			}
		}

		update_stats(ctx);

		schedule_arrival();
	}


	/**
	 * \brief Callback for handling the departure event.
	 * \param evt The event fired by the departure event source.
	 * \param ctx The DES engine context.
	 */
	private: void process_departure(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(evt);

		DCS_DEBUG_TRACE("Processing DEPARTURE event: " << evt << " for QUEUE object: " << this);

		// preconditions
		DCS_ASSERT(
			cur_num_busy_ > 0,
			throw ::std::runtime_error("[open_queue::process_departure] There is no job leaving the system")
		);

		cur_evt_tag_ = departure_event_tag;
		++num_departures_;
		--cur_num_busy_;

		queue_job_type job = evt.template unfolded_state<queue_job_type>();

		accumulate_stat(
			response_time_queue_statistic,
			ctx.simulated_time() - job.arrival_time()
		);

		update_stats(ctx);

		if (!queue_.empty())
		{
			schedule_departure();
		}
	}


	/**
	 * \brief Callback for handling the discard event.
	 * \param evt The event fired by the discard event source.
	 * \param ctx The DES engine context.
	 */
	private: void process_discard(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(evt);

		DCS_DEBUG_TRACE("Processing DISCARD event: " << evt << " for QUEUE object: " << this);

		cur_evt_tag_ = discard_event_tag;
		++num_discards_;

		update_stats(ctx);

//		schedule_arrival();
	}


	/// Schedule a new arrival event.
	private: void schedule_arrival()
	{
		real_type arr_time(0);

		// This loop is needed for coping with distributions
		// that can take non-positive values
		while ((arr_time = iatime_distr_.rand(rng_)) < 0) ;

		DCS_DEBUG_TRACE("Scheduling ARRIVAL event at time: " << (ptr_eng_->simulated_time() + arr_time) << " for QUEUE object: " << this);

		ptr_eng_->schedule_event(
			ptr_arrival_evt_src_,
			ptr_eng_->simulated_time() + arr_time
		);

		// Update the interarrival time stats with the interarrival time of this job
		accumulate_stat(interarrival_time_queue_statistic, arr_time);
	}


	/// Schedule a new departure event.
	private: void schedule_departure()
	{
		// preconditions
		DCS_ASSERT(
			queue_.size() > 0,
			throw ::std::runtime_error("[open_queue::schedule_departure] There is no job to serve")
		);
		DCS_ASSERT(
			cur_num_busy_ < num_srv_,
			throw ::std::runtime_error("[open_queue::schedule_departure] There is no idle server")
		);

		// Choose the next job to be executed
		queue_job_type job = queue_.next();
		queue_.pop();

		real_type wait_time = ptr_eng_->simulated_time() - job.arrival_time();

		// Occupy an idle server
		++cur_num_busy_;

		// Draw a service time and schedule a departure

		real_type svc_time(0);

		// This loop is needed for coping with distributions
		// that can take non-positive values
		while ((svc_time = svctime_distr_.rand(rng_)) < 0) ;

		DCS_DEBUG_TRACE("Service time: " << svc_time << " - Share: " << svc_share_ << " for QUEUE object: " << this);

		svc_time /= svc_share_;

		DCS_DEBUG_TRACE("Scheduling DEPARTURE event at time: " << (ptr_eng_->simulated_time() + svc_time) << " for QUEUE object: " << this);

		job.waiting_time(wait_time);
		job.service_time(svc_time);
		job.departure_time(ptr_eng_->simulated_time() + svc_time);

		ptr_eng_->schedule_event(
			ptr_departure_evt_src_,
			ptr_eng_->simulated_time() + svc_time,
			job
		);

		// Update the waiting time stats with the waiting time of this job
		accumulate_stat(waiting_time_queue_statistic, wait_time);
		// Update the service time stats with the service time of this job
		accumulate_stat(service_time_queue_statistic, svc_time);
	}


	/// Schedule a new discard event.
	private: void schedule_discard()
	{
		DCS_DEBUG_TRACE("Scheduling DISCARD event at time: " << ptr_eng_->simulated_time() << " for QUEUE object: " << this);

//		++num_discards_;

		ptr_eng_->schedule_event(
			ptr_discard_evt_src_,
			ptr_eng_->simulated_time(),
			queue_job_type()
		);
	}


	//@{ Data members

	//private: real_type lambda_;
	private: ::boost::shared_ptr<des_event_source_type> ptr_arrival_evt_src_;
	private: ::boost::shared_ptr<des_event_source_type> ptr_departure_evt_src_;
	private: ::boost::shared_ptr<des_event_source_type> ptr_discard_evt_src_;
	private: arrival_distribution_type iatime_distr_;
	private: service_distribution_type svctime_distr_;
	private: /*const*/ uint_type num_srv_;
	private: /*const*/ bool queue_is_finite_;
	private: /*const*/ uint_type max_queue_len_;
	private: random_generator_type rng_;
	private: ::boost::shared_ptr<des_engine_type> ptr_eng_;
	private: queue_policy_type queue_;
	private: real_type svc_share_;
	private: uint_type num_arrivals_;
	private: uint_type num_departures_;
	private: uint_type num_discards_;
	private: uint_type cur_num_busy_;
	//private: ::std::vector< ::std::vector< ::boost::shared_ptr<output_statistic_type> > > stats_;
	private: output_statistic_category_container stats_;
	private: event_tags cur_evt_tag_;
	private: real_type last_evt_time_;
	//private: typename des_event_source_type::connection_type arrivals_slot_;
	//private: typename des_event_source_type::connection_type departures_slot_;
	//private: typename des_event_source_type::connection_type discards_slot_;
	//private: typename des_event_source_type::connection_type bos_slot_;

	//@} Data members
};

}}} // Namespace dcs::des::model


#endif // DCS_DES_MODEL_OPEN_QUEUE_HPP
