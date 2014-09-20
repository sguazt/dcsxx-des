/**
 * \file dcs/des/replications/engine.hpp
 *
 * \brief Discrete-event simulator engine with output analysis based on the
 *  Independent Replications method.
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

#ifndef DCS_DES_REPLICATIONS_ENGINE_HPP
#define DCS_DES_REPLICATIONS_ENGINE_HPP


#include <boost/smart_ptr.hpp>
#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
//#include <dcs/des/replications/pawlikowski1990_batch_size_detector.hpp>
#include <dcs/des/replications/analyzable_statistic.hpp>
#include <dcs/des/replications/dummy_num_replications_detector.hpp>
#include <dcs/des/replications/dummy_replication_size_detector.hpp>
#include <dcs/des/engine.hpp>
#include <dcs/des/null_transient_detector.hpp>
#include <dcs/des/output_analysis.hpp>
#include <dcs/des/output_analysis_categories.hpp>
//#include <dcs/des/spectral/pawlikowski1990_transient_detector.hpp>
#include <dcs/exception.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/math/constants.hpp>
#include <iomanip>
#include <iostream>
#include <stdexcept>


namespace dcs { namespace des {

namespace replications {

template <typename RealT>
class engine: public ::dcs::des::engine<RealT>
{
	private: typedef ::dcs::des::engine<RealT> base_type;
	private: typedef engine<RealT> self_type;
	public: typedef typename base_type::real_type real_type;
	public: typedef typename base_type::event_type event_type;
	public: typedef typename base_type::engine_context_type engine_context_type;
	public: typedef typename base_type::event_source_type event_source_type;
	public: typedef ::dcs::des::independent_replications_analysis_category output_analysis_category;


	//public: static const std::size_t default_min_repl_size = 1000;
	public: static const RealT default_min_repl_duration; // = 1000;
	public: static const std::size_t default_min_num_replications = 5;


	// The default constructor
	public: explicit engine(RealT min_repl_duration=default_min_repl_duration, std::size_t min_num_repl=default_min_num_replications)
		: base_type(),
		  min_repl_duration_(min_repl_duration),
		  min_num_repl_(min_num_repl),
		  end_of_repl_(false),
		  ptr_bor_evt_src_(new event_source_type("Begin of Replication")),
		  ptr_meor_evt_src_(new event_source_type("Maybe End of Replication")),
		  ptr_eor_evt_src_(new event_source_type("End of Replication")),
		  repl_count_(0)
	{
		init();
	}


	/// Copy constructor.
	private: engine(engine const& that)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(that);

		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}


	/// Copy assignment.
	private: engine& operator=(engine const& rhs)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(rhs);

		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-assignment not yet implemented." );
	}


//	public: virtual ~engine()
//	{
//		ptr_bor_evt_src_->disconnect(
//			::dcs::functional::bind(
//				&self_type::process_begin_of_replication,
//				this,
//				::dcs::functional::placeholders::_1,
//				::dcs::functional::placeholders::_2
//			)
//		);
//		ptr_eor_evt_src_->disconnect(
//			::dcs::functional::bind(
//				&self_type::process_end_of_replication,
//				this,
//				::dcs::functional::placeholders::_1,
//				::dcs::functional::placeholders::_2
//			)
//		);
//	}


	public: void min_replication_duration(RealT time)
	{
		min_repl_duration_ = time;
	}


	public: RealT min_replication_duration() const
	{
		return min_repl_duration_;
	}


	public: void min_num_replications(std::size_t n)
	{
		min_num_repl_ = n;
	}


	public: std::size_t min_num_replications() const
	{
		return min_num_repl_;
	}


	public: event_source_type const& begin_of_replication_event_source() const
	{
		return *ptr_bor_evt_src_;
	}


	public: event_source_type& begin_of_replication_event_source()
	{
		return *ptr_bor_evt_src_;
	}


	public: event_source_type const& end_of_replication_event_source() const
	{
		return *ptr_eor_evt_src_;
	}


	public: event_source_type& end_of_replication_event_source()
	{
		return *ptr_eor_evt_src_;
	}


//	public: void run()
//	{
//		engine_context_type ctx(this);
//
//		while (repl_count_ < min_num_repl_)
//		{
//			++repl_count_;
//
//			DCS_DEBUG_TRACE_L(1, ">> Begin REPLICATION #" << repl_count_);
//
//			// Clear the simulation state
//			this->reset();
//
//			// Schedule the begin_of_replication event now
//			schedule_event(ptr_bor_evt_src_, this->simulated_time(), repl_count_);
//			fire_next_event(ctx);
//
//			base_type::run();
//
//			// Schedule the end_of_replication event now
//			schedule_event(ptr_eor_evt_src_, this->simulated_time(), repl_count_);
//			fire_next_event(ctx);
//
//			DCS_DEBUG_TRACE_L(1, ">> End REPLICATION #" << repl_count_);
//		}
//	}


	public: std::size_t num_replications() const
	{
		return repl_count_;
	}


	protected: bool is_internal_event(event_type const& evt) const
	{
		return base_type::is_internal_event(evt)
			   || evt.source() == *ptr_bor_evt_src_
			   || evt.source() == *ptr_eor_evt_src_;
	}


	protected: void monitor_statistics_in_replication()
	{
		//typedef typename std::map<boost::shared_ptr< base_analyzable_statistic<real_type> >,bool>::const_iterator stat_iterator;
		typedef typename std::map< base_analyzable_statistic<real_type>*, std::pair<boost::shared_ptr< base_analyzable_statistic<real_type> >,bool> >::const_iterator stat_iterator;

		DCS_DEBUG_TRACE_L(1, "(" << this << ") BEGIN Monitoring statistics in replication."); //XXX

		if (this->monitored_statistics().empty())
		{
			return;
		}

		// NOTE: Current replication is done only when *all* of the monitored stats
		//       are "complete".

		bool replication_done = true;
		const stat_iterator end_it = this->monitored_statistics().end();
		for (stat_iterator it = this->monitored_statistics().begin();
			 it != end_it;
			 ++it)
		{
			//boost::shared_ptr< base_analyzable_statistic<real_type> > ptr_stat(it->first);
			boost::shared_ptr< base_analyzable_statistic<real_type> > ptr_stat(it->second.first);

			ptr_stat->refresh();

			if (ptr_stat->enabled() && !ptr_stat->observation_complete())
			{
				replication_done = false;
				break;
			}
		}

		if (replication_done)
		{
			end_of_repl_ = true;
		}

		DCS_DEBUG_TRACE_L(1, "(" << this << ") END Monitoring statistics in replication."); //XXX
	}


	protected: void prepare_replication(engine_context_type& ctx)
	{
		end_of_repl_ = false;

		// Reset simulation clock
		this->simulated_time(0);

//		// Schedule the BEGIN-OF-REPLICATION event now...
//		this->schedule_event(ptr_bor_evt_src_, this->simulated_time(), repl_count_);
//		// ... and immediately fire it
//		this->fire_next_event(ctx);
		// Immediately (schedule and) fire the BEGIN-OF-REPLICATION event
		this->fire_immediate_event(ptr_bor_evt_src_, ctx, repl_count_);

//		if (this->monitored_statistics().empty())
//		{
//			// Schedule the END-OF-REPLICATION event after the minimum replication duration
//			this->schedule_event(ptr_eor_evt_src_, this->simulated_time()+min_repl_duration_, repl_count_);
//		}
//
//		this->initialize_simulated_system(ctx);
	}


	protected: void finalize_replication(engine_context_type& ctx)
	{
		this->finalize_simulated_system(ctx);

//		// Schedule the end_of_replication event now...
//		schedule_event(ptr_eor_evt_src_, this->simulated_time(), repl_count_);
//		// ... and immediately fire it
//		fire_next_event(ctx);
		// Immediately (schedule and) fire the END-OF-REPLICATION event
		this->fire_immediate_event(ptr_eor_evt_src_, ctx, repl_count_);
//		end_of_repl_ = true;
//
//		// Clear the replication state
//		this->reset();
	}


	protected: void do_stop_now()
	{
		if (!end_of_repl_)
		{
			end_of_repl_ = true;
		}
	}


	protected: void print(::std::ostream& os) const
	{
		os << "<(independent replications)"
		   << " # Replications: " << this->num_replications()
		   << ", Simulated time: " << this->simulated_time()
		   << ", Simulation done? " << ::std::boolalpha << this->end_of_simulation()
		   << ">";
	}


	private: void init()
	{
		ptr_bor_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_begin_of_replication,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ptr_meor_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_maybe_end_of_replication,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ptr_eor_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_end_of_replication,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


	private: void do_run()
	{
		DCS_DEBUG_TRACE( "Begin SIMULATION" );

		engine_context_type ctx(this);

		this->prepare_simulation(ctx);
		end_of_repl_ = false;
		repl_count_ = 0;

		//while (repl_count_ < min_num_repl_)
		while (!this->end_of_simulation())
		{
			++repl_count_;

			DCS_DEBUG_TRACE(">> Begin REPLICATION #" << repl_count_ << " - Simulation time: " << this->simulated_time() << " - Min Duration: " << min_repl_duration_);
std::cerr << ">> BEGIN REPLICATION #" << repl_count_ << " - Simulation time: " << this->simulated_time() << " - Min Duration: " << min_repl_duration_ << std::endl;//XXX

			prepare_replication(ctx);

			while (!end_of_repl_ && !this->future_event_list().empty())
			{
				DCS_DEBUG_TRACE_L(1,  "Simulation time: " << this->simulated_time() );

				this->fire_next_event(ctx);

				// Monitor statistics
				monitor_statistics_in_replication();

				// Check if simulation has ended (e.g., it may happen if the
				// client calls the stop_now or stop_at_time method).
				if (this->end_of_simulation())
				{
					end_of_repl_ = true;
				}

				//FIXME: What should we do when the simulation is done but, for
				//       instance, the replication is shorter than the specified
				//       duration?
				//       Maybe we should use another flag (e.g.,
				//       forced_end_of_sim_) in order to distinguish the case of
				//       "normal" and "user-requested" end of simulation.
				//       Currently, we give priority to the replication length.

				//if (end_of_repl_ && this->simulated_time() < min_repl_duration_)
				if (end_of_repl_)
				{
					// Make sure to consume all concurrent events (i.e., events that fire now)
					if (!this->future_event_list().empty())
					{
						if (this->future_event_list().top()->fire_time() == this->simulated_time())
						{
							end_of_repl_ = false;
							this->end_of_simulation(false);
						}
					}
					// Make sure that replication lasts the minimum set duration.
 					if (this->simulated_time() < min_repl_duration_)
					{
						end_of_repl_ = false;
						this->end_of_simulation(false);
					}
				}
			}

			if (!end_of_repl_ && this->future_event_list().empty())
			{
				::std::clog << "[Warning] Replication not ended but event list is empty: forcing end of replication." << ::std::endl;
			}

			finalize_replication(ctx);

			this->monitor_statistics();

			// Check for end-of-simulation terminating conditions
			if (this->end_of_simulation())
			{
				// Make sure that simulation lasts the minimum set replication number.

				if (repl_count_ < min_num_repl_)
				{
					this->end_of_simulation(false);
				}
			}
			else
			{
				// Make sure that simulation does not take too long than necessary.

				if (repl_count_ >= min_num_repl_ && this->monitored_statistics().empty())
				{
					this->end_of_simulation(true);
				}
			}

std::cerr << ">> END REPLICATION #" << repl_count_ << " - Simulation time: " << this->simulated_time() << " - Min Duration: " << min_repl_duration_ << std::endl;//XXX
			DCS_DEBUG_TRACE(">> End REPLICATION #" << repl_count_ << " - Simulation time: " << this->simulated_time());
		}

		this->finalize_simulation(ctx);

		DCS_DEBUG_TRACE( "End SIMULATION" );
	}


	private: boost::shared_ptr< base_analyzable_statistic<real_type> > do_make_analyzable_statistic(any_statistic<real_type> const& stat)
	{
		typedef ::dcs::des::null_transient_detector<RealT> transient_detector_type;
		typedef dummy_replication_size_detector<RealT> replication_size_detector_type;
		typedef dummy_num_replications_detector<RealT> num_replications_detector_type;
		typedef analyzable_statistic<any_statistic<real_type>,
									 transient_detector_type,
									 replication_size_detector_type,
									 num_replications_detector_type> analyzable_statistic_impl_type;

//FIXME: don't compile since assume *this as a const reference
//		return ::boost::make_shared<analyzable_statistic_impl_type>(
//					stat,
//					transient_detector_type(),
//					replication_size_detector_type(),
//					num_replications_detector_type(),
//					*this,
//					::dcs::math::constants::infinity<RealT>::value,
//					::dcs::math::constants::infinity<std::size_t>::value
//			);
		return ::boost::shared_ptr<analyzable_statistic_impl_type>(
				new analyzable_statistic_impl_type(
					stat,
					transient_detector_type(),
					replication_size_detector_type(),
					num_replications_detector_type(),
					*this,
					::dcs::math::constants::infinity<RealT>::value,
					::dcs::math::constants::infinity<std::size_t>::value
				)
			);
	}


	private: void process_begin_of_replication(event_type const& evt, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );

		DCS_DEBUG_TRACE_L(1, "(" << this << ") Begin Processing of BEGIN-OF-REPLICATION");

		end_of_repl_ = false;

		// Clear the replication state
		this->reset();

//		// Reset simulation clock
//		this->simulated_time(0);

		// In case there are no monitored statistics, stop the replication
		// according to the minimum replication length.
		// -> Schedule the MAYBE-END-OF-REPLICATION event after the minimum replication duration
		this->schedule_event(ptr_meor_evt_src_, this->simulated_time()+min_repl_duration_, repl_count_);

//		// In case there are no monitored statistics, stop the replication
//		// according to the minimum replication length.
//		if (this->monitored_statistics().empty())
//		{
//			// Schedule the END-OF-REPLICATION event after the minimum replication duration
//			this->schedule_event(ptr_eor_evt_src_, this->simulated_time()+min_repl_duration_, repl_count_);
//		}
//		else
		if (!this->monitored_statistics().empty())
		{
			// Initialize statistics for the new experiment

			//typedef typename std::map<boost::shared_ptr< base_analyzable_statistic<real_type> >,bool>::iterator stat_iterator;
			typedef typename std::map< base_analyzable_statistic<real_type>*, std::pair<boost::shared_ptr< base_analyzable_statistic<real_type> >,bool> >::iterator stat_iterator;

			const stat_iterator end_it = this->monitored_statistics().end();
			for (
				stat_iterator it = this->monitored_statistics().begin();
				it != end_it;
				++it
			) {
				//boost::shared_ptr< base_analyzable_statistic<real_type> > ptr_stat(it->first);
				boost::shared_ptr< base_analyzable_statistic<real_type> > ptr_stat(it->second.first);

				ptr_stat->initialize_for_experiment();
			}
		}

		this->initialize_simulated_system(ctx);

		DCS_DEBUG_TRACE_L(1, "(" << this << ") End Processing of BEGIN-OF-REPLICATION");
	}


	private: void process_maybe_end_of_replication(event_type const& evt, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE_L(1, "(" << this << ") Begin Processing of MAYBE-END-OF-REPLICATION");

		// In case there are no monitored statistics, stop the replication
		// according to the minimum replication length.
		if (this->monitored_statistics().empty())
		{
			// Schedule the END-OF-REPLICATION event now
			this->schedule_event(ptr_eor_evt_src_, this->simulated_time(), repl_count_);
		}

		DCS_DEBUG_TRACE_L(1, "(" << this << ") End Processing of MAYBE-END-OF-REPLICATION");
	}


	private: void process_end_of_replication(event_type const& evt, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE_L(1, "(" << this << ") Begin Processing of END-OF-REPLICATION");

		if (!this->monitored_statistics().empty())
		{
			// Finalize statistics for the current experiment

			//typedef typename std::map<boost::shared_ptr< base_analyzable_statistic<real_type> >,bool>::const_iterator stat_iterator;
			typedef typename std::map< base_analyzable_statistic<real_type>*, std::pair<boost::shared_ptr< base_analyzable_statistic<real_type> >,bool> >::const_iterator stat_iterator;

			const stat_iterator end_it = this->monitored_statistics().end();
			for (
				stat_iterator it = this->monitored_statistics().begin();
				it != end_it;
				++it
			) {
				//boost::shared_ptr< base_analyzable_statistic<real_type> > ptr_stat(it->first);
				boost::shared_ptr< base_analyzable_statistic<real_type> > ptr_stat(it->second.first);

				ptr_stat->finalize_for_experiment();
			}
		}

		end_of_repl_ = true;

//		// Clear the replication state
//		this->reset();

		DCS_DEBUG_TRACE_L(1, "(" << this << ") End Processing of END-OF-REPLICATION");
	}


	private: RealT min_repl_duration_; ///< The minimum length of each replication.
	private: std::size_t min_num_repl_; ///< The minimum number of replication to be performed.
	//private: bool end_of_sim_;
	private: bool end_of_repl_; ///< Boolean flag for indicating the end of the current replication.
	private: boost::shared_ptr< event_source<RealT> > ptr_bor_evt_src_; ///< The Begin-of-Replication event source.
	private: boost::shared_ptr< event_source<RealT> > ptr_meor_evt_src_; ///< The Maybe-End-of-Replication event source.
	private: boost::shared_ptr< event_source<RealT> > ptr_eor_evt_src_; ///< The End-of-Replication event source.
	private: std::size_t repl_count_; ///< The number of performed replications.
//	private: RealT acc_sim_time_; ////< Accumulated value of simulated time.
}; // engine


template <typename RealT>
const RealT engine<RealT>::default_min_repl_duration = 1;


namespace /*<unnamed>*/ { namespace detail {

template <
	typename TransientDetectorT,
	typename ReplicationSizeDetectorT,
	typename NumReplicationsDetectorT
>
struct output_analyzer
{
	typedef TransientDetectorT transient_detector_type;
	typedef ReplicationSizeDetectorT replication_size_detector_type;
	typedef NumReplicationsDetectorT num_replications_detector_type;
};

}} // Namespace <unnamed>::detail

} // Namespace replications


/**
 * \brief Instantiation of class \c make_analyzable_statistic_type for the
 *  independent replications output analysis.
 */
template <
	typename StatisticT,
//	typename RealT,
	typename TransientPhaseDetectorT,
	typename ReplicationSizeDetectorT,
	typename NumReplicationsDetectorT
>
struct make_analyzable_statistic_type<
			StatisticT,
			replications::detail::output_analyzer<
				TransientPhaseDetectorT,
				ReplicationSizeDetectorT,
				NumReplicationsDetectorT
			>//,
//			replications::engine<RealT>
	>
{
	typedef replications::analyzable_statistic<
				StatisticT,
				TransientPhaseDetectorT,
				ReplicationSizeDetectorT,
				NumReplicationsDetectorT
			> type;
};


/**
 * \brief  Overload of function \c make_analyzable_statistic for the independent
 * replications output analysis.
 */
template <
	typename StatisticT,
	typename RealT,
	typename TransientPhaseDetectorT,
	typename ReplicationSizeDetectorT,
	typename NumReplicationsDetectorT
>
::boost::shared_ptr<
	typename make_analyzable_statistic_type<
		StatisticT,
		replications::detail::output_analyzer<
			TransientPhaseDetectorT,
			ReplicationSizeDetectorT,
			NumReplicationsDetectorT
		>//,
//		replications::engine<
//			RealT,
//		>
	>::type
> make_analyzable_statistic(StatisticT const& stat,
							TransientPhaseDetectorT const& transient_detector,
							ReplicationSizeDetectorT const& repl_size_detector,
							NumReplicationsDetectorT const& num_repl_detector,
							replications::engine<RealT>& des_engine,
							RealT relative_precision,
							std::size_t max_obs)
{
	typedef typename make_analyzable_statistic_type<
						StatisticT,
						replications::detail::output_analyzer<
							TransientPhaseDetectorT,
							ReplicationSizeDetectorT,
							NumReplicationsDetectorT
						>//,
//						replications::engine<RealT>
					>::type statistic_type;

	::boost::shared_ptr<statistic_type> ptr_stat(
			new statistic_type(
				stat,
				transient_detector,
				repl_size_detector,
				num_repl_detector,
				des_engine,
				relative_precision,
				max_obs
			)
	);

	des_engine.analyze_statistic(ptr_stat);

	return ptr_stat;
}


}} // Namespace dcs::des


#endif // DCS_DES_REPLICATIONS_ENGINE_HPP
