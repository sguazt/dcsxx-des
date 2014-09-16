/**
 * \file dcs/des/replications/analyzable_statistic.hpp
 *
 * \brief Output statistic analyzed according to the Independent Replications
 *  method.
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

#ifndef DCS_DES_REPLICATIONS_ANALYZABLE_STATISTIC_HPP
#define DCS_DES_REPLICATIONS_ANALYZABLE_STATISTIC_HPP


#include <algorithm>
#include <cmath>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/base_analyzable_statistic.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/mean_estimator.hpp>
//#include <dcs/des/replications/engine.hpp>
#include <dcs/des/statistic_categories.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/math/constants.hpp>
#include <dcs/math/traits/float.hpp>


//TODO:
// - Find a way to deregister foreign event handlers (e.g. sys-init and sys-final)
//

namespace dcs { namespace des { namespace replications {

template <typename RealT, typename UIntT>
class engine;


/**
 * \brief Output statistic analyzed according to the Independent Replications
 *  method.
 *
 * \tparam StatisticT The type of the output statistic under analysis.
 * \tparam TransientDetectorT The type of the transient phase detector.
 * \tparam ReplicationSizeDetectorT The type of the replication size detector.
 * \tparam NumReplicationsDetectorT The type of the replication size detector.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <
	typename StatisticT,
	typename TransientDetectorT,
	typename ReplicationSizeDetectorT,
	typename NumReplicationsDetectorT
>
class analyzable_statistic: public base_analyzable_statistic<
										typename StatisticT::value_type,
										typename StatisticT::uint_type
									>
{
	private: typedef analyzable_statistic<StatisticT,TransientDetectorT,ReplicationSizeDetectorT,NumReplicationsDetectorT> self_type;
	public: typedef StatisticT statistic_type;
	public: typedef typename statistic_type::value_type value_type;
	public: typedef typename statistic_type::uint_type uint_type;
	public: typedef TransientDetectorT transient_phase_detector_type;
	public: typedef ReplicationSizeDetectorT replication_size_detector_type;
	public: typedef NumReplicationsDetectorT num_replications_detector_type;
//	public: typedef typename statistic_type::category_type category_type;
	private: typedef base_analyzable_statistic<value_type,uint_type> base_type;
	private: typedef ::dcs::des::replications::engine<value_type,uint_type> engine_type;
	private: typedef typename ::dcs::des::engine_traits<engine_type>::event_type event_type;
	private: typedef typename ::dcs::des::engine_traits<engine_type>::engine_context_type engine_context_type;


//	public: static const value_type default_confidence_level; ///< Default value for the confidence level
	public: static const uint_type default_min_num_repl = 2; ///< Default value for the minimum number of replications
	public: static const uint_type default_max_num_obs; ///< Default value for the maximum number of observations


	/// A constructor.
	public: explicit analyzable_statistic(value_type relative_precision = base_type::default_target_relative_precision,
										  uint_type max_num_obs = default_max_num_obs,
										  uint_type min_num_repl = default_min_num_repl)
		: base_type(relative_precision),
		  stat_(),
		  trans_detector_(),
		  repl_size_detector_(),
		  num_repl_detector_(),
		  min_num_repl_(min_num_repl),
		  max_num_obs_(max_num_obs),
		  trans_detected_(false),
		  trans_len_(0),
		  repl_size_detected_(false),
		  repl_size_(0),
		  num_repl_detected_(false),
		  num_repl_(0),
		  repl_mean_stat_(),
		  steady_start_time_(0)
	{
		// Empty
	}

	/**
	 * \brief A constructor.
	 *
	 * \param stat The statistic to be monitored.
	 * \param transient_detector The transient phase detector
	 *  (model of TransientDetector concept).
	 * \param replication_size_detector The replication size detector
	 *  (model of ReplicationSizeDetector concept).
	 * \param num_replications_detector The number of replications detector
	 *  (model of NumReplicationsDetector concept).
	 * \param relative_precision The target relative precision of the confidence
	 *  interval.
	 * \param min_num_replications The minimum number of replications to collect before
	 *  checking for relative precision.
	 */
	public: template <typename RealT, typename UIntT>
		analyzable_statistic(statistic_type const& stat,
							 transient_phase_detector_type const& transient_detector,
							 replication_size_detector_type const& repl_size_detector,
							 num_replications_detector_type const& num_repl_detector,
							 ::dcs::des::replications::engine<RealT,UIntT>& eng,
							 value_type relative_precision = base_type::default_target_relative_precision,
							 uint_type max_num_obs = default_max_num_obs,
							 uint_type min_num_repl = default_min_num_repl)
		: base_type(relative_precision),
		  stat_(stat),
		  trans_detector_(transient_detector),
		  repl_size_detector_(repl_size_detector),
		  num_repl_detector_(num_repl_detector),
		  min_num_repl_(min_num_repl),
		  max_num_obs_(max_num_obs),
		  trans_detected_(false),
		  trans_len_(0),
		  repl_size_detected_(false),
		  repl_size_(0),
		  num_repl_detected_(false),
		  num_repl_(0),
		  repl_mean_stat_(),
		  steady_start_time_(0)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(eng);

		// pre: min number of replications >= 2
		DCS_ASSERT(
			min_num_repl_ >= 2,
			throw ::std::invalid_argument("[dcs::des::replications::analyzable_statistic::ctor] Number of replications must be >= 2.")
		);
	}

	public: uint_type actual_num_replications() const
	{
		return repl_mean_stat_.num_observations();
	}

	public: uint_type actual_replication_size() const
	{
		return stat_.num_observations();
	}

//	public: bool num_replications_detected() const
//	{
//		return num_repl_detected_;
//	}

	public: uint_type num_replications() const
	{
		return num_repl_;
	}

	public: bool replication_done() const
	{
		return repl_size_detected_ && actual_replication_size() >= replication_size();
	}

	public: uint_type replication_size() const
	{
		return repl_size_;
	}

	protected: void do_initialize_for_experiment()
	{
		reset_for_replication();
	}

	protected: void do_finalize_for_experiment()
	{
		do_estimate(stat_.estimate());
	}

	private: void reset_for_replication()
	{
		DCS_DEBUG_TRACE("(" << this << ") BEGIN Reset for Replication");

//[FIXME] 2011-05-22
//		// NOTE: don't reset here the underlying stat since this event-handler
//		//       may be called after another one which add add some value to
//		//       this stat. So the reset must be done at the end of replication.
		stat_.reset();
//[/FIXME] 2011-05-22

		// Calling the 'detected' method instead of setting to 'false' allows
		// 'dummy' (e.g., the one with a zero size value) detector to work
		// properly.
//		repl_size_detector_.reset();
//		repl_size_detected_ = repl_size_detector_.detected();
//::std::cerr << "SIZE DETECTOR: " << repl_size_detector_.print() << ::std::endl;//XXX
//		if (repl_size_detected_)
//		{
//			repl_size_ = ::std::min(max_num_obs_, repl_size_detector_.estimated_size());
//		}
//		trans_detector_.reset();
//		trans_detected_ = trans_detector_.detected();
//		if (trans_detected_)
//		{
//			trans_len_ = trans_detector_.estimated_size();
//		}
		trans_detector_.reset();
		trans_detected_ = false;
		trans_len_ = 0;
		transient_detection();
		repl_size_detector_.reset();
		repl_size_ = 0;
		repl_size_detected_ = false;
		replication_size_detection();

		DCS_DEBUG_TRACE("(" << this << ") END Reset for Replication");
	}

	//@{ Event handlers

//	private: void process_begin_of_sim(event_type const& evt, engine_context_type& ctx)
//	{
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
//
//		DCS_DEBUG_TRACE_L(5, "(" << this << ") BEGIN Processing BEGIN-OF-SIMULATION (Clock: " << ctx.simulated_time() << ")");
//
//		this->reset();
//
//		DCS_DEBUG_TRACE_L(5, "(" << this << ") END Processing BEGIN-OF-SIMULATION (Clock: " << ctx.simulated_time() << ")");
//	}


//	private: void process_begin_of_replication(event_type const& evt, engine_context_type& ctx)
//	{
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
//
//		DCS_DEBUG_TRACE_L(5, "(" << this << ") BEGIN Processing BEGIN-OF-REPLICATION (Clock: " << ctx.simulated_time() << ")");
//
////		reset_for_replication();
////		stat_.reset();
//
//		DCS_DEBUG_TRACE_L(5, "(" << this << ") END Processing BEGIN-OF-REPLICATION (Clock: " << ctx.simulated_time() << ")");
//	}


//	private: void process_end_of_replication(event_type const& evt, engine_context_type& ctx)
//	{
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
//
//		DCS_DEBUG_TRACE_L(9, "(" << this << ") BEGIN Processing END-OF-REPLICATION (Clock: " << ctx.simulated_time() << ")");
//
//		do_estimate(stat_.estimate());
//
////[FIXME] 2011-05-22
//		// NOTE: The reset operation has been moved to the
//		//       'process_begin_of_replications' function since leaving it here
//		//       may introduce side-effect if one query the statistic object
//		//       at the end of each replication (e.g., one can find that the
//		//       statistic is INCOMPLETED because the state of the
//		//       number-of-replication size detector has been reset.
//		//       Nevertheless, we need to reset here the underlying statistic
//		//       since actually this cannot be done at the beginning of replication
//		//       because the event handler may be called after that some value has
//		//       already been collected (yes, this is a buggy behavior :( )
////		reset_for_replication();
////		stat_.reset();
////[/FIXME] 2011-05-22
//
//		DCS_DEBUG_TRACE_L(9, "(" << this << ") END Processing END-OF-REPLICATION (Clock: " << ctx.simulated_time() << ")");
//	}

	//@} Event handlers


	//@{ Interface methods

	private: statistic_category do_category() const
	{
		return stat_.category();
	}


	/**
	 * \brief Collect a new observation.
	 * \param obs The new observation to be collected.
	 */
	private: void do_collect(value_type obs, value_type weight)
	{
		DCS_DEBUG_TRACE("(" << this << ") BEGIN Collecting observation " << obs << " - weight: " << weight);

//		if (!this->enabled())
//		{
//			DCS_DEBUG_TRACE("(" << this << ") Statistical Analysis Disabled!");
//
//			return;
//		}

		DCS_DEBUG_TRACE("(" << this << ") [Observation #" << stat_.num_observations() << " -- MAX: " << max_num_obs_ << "] Observation: " << obs << " - Weight: " << weight);

		if (max_num_obs_ != base_type::num_observations_infinity && stat_.num_observations() >= max_num_obs_)
		{
			DCS_DEBUG_TRACE("(" << this << ") Reached maximum number of observations: " << stat_.num_observations() << "/" << max_num_obs_);

			::std::clog << "[Warning] Statistic '" << *this << "' will be disabled: collected max number of observations." << ::std::endl;

			this->enable(false);
			return;
		}

		if (repl_size_detected_)
		{
			stat_(obs, weight);
		}
		else if (trans_detected_)
		{
			// Transient detected but replication size is still to be detected.

			DCS_DEBUG_TRACE("(" << this << ") Detecting replication length...");

			repl_size_detected_ = repl_size_detector_.detect(obs, weight);

			this->replication_size_detection();
		}
		else
		{
			// Detect transient phase

			DCS_DEBUG_TRACE("(" << this << ") Detecting transient phase...");

			trans_detected_ = trans_detector_.detect(obs, weight);

			this->transient_detection();
		}

		DCS_DEBUG_TRACE("(" << this << ") END Collecting observation " << obs << " - weight: " << weight);
	}


	private: void transient_detection()
	{
		DCS_DEBUG_TRACE("(" << this << ") Handling detection of transient phase...");

//		if (trans_detected_ || !this->enabled())
		if (trans_detected_)
		{
			return;
		}

		trans_detected_ = trans_detector_.detected();

		if (trans_detected_)
		{
			trans_len_ = trans_detector_.estimated_size();

			DCS_DEBUG_TRACE("(" << this << ") Detected transient phase. Putting-back " << trans_detector_.steady_state_observations().size() << " safe steady-state observations.");

			// Transient phase just detected
			// Put back steady-state observations possibly used for
			// transient phase detection
			typedef typename transient_phase_detector_type::sample_container vector_type;
			typedef typename vector_type::const_iterator vector_iterator_type;

			vector_type obss = trans_detector_.steady_state_observations();
			vector_iterator_type obss_end = obss.end();

			for (
				vector_iterator_type it = obss.begin();
				it != obss_end;
				++it
			) {
				// Recursively call this method in order to collect
				// observations for replication size detection or for
				// sample accumulation
				this->operator()(it->first, it->second);
			}

			DCS_DEBUG_TRACE("(" << this << ") Safe steady-state observations put back.");

			// Reset transient detector to save memory
			trans_detector_.reset();
		}
		else if (trans_detector_.aborted())
		{
//				throw ::std::runtime_error("Transient phase is done but transient period not detected.");
//				DCS_DEBUG_TRACE("(" << this << ") Transient phase detection has been aborted. Disabling statistical analysis.");

			::std::clog << "[Warning] Statistic '" << *this << "' will be disabled: Transient phase detection has been aborted." << ::std::endl;

			this->enable(false);
		}
	}


	private: void replication_size_detection()
	{
		DCS_DEBUG_TRACE("(" << this << ") Handling detection of replication size...");

//		if (repl_size_detected_ || !this->enabled())
		if (repl_size_detected_)
		{
			return;
		}

		repl_size_detected_ = repl_size_detector_.detected();

		if (repl_size_detected_)
		{
			repl_size_ = repl_size_detector_.estimated_size();

			DCS_DEBUG_TRACE("(" << this << ") Detected replication size. Taking back " << repl_size_detector_.consumed_observations().size() << " observations consumed during replication size detection.");

			// Replication size just detected.
			// Takes all of the replicate means computed during the
			// replication size detection.

			typedef typename replication_size_detector_type::vector_type vector_type;
			typedef typename vector_type::const_iterator vector_iterator_type;

			vector_type obss = repl_size_detector_.consumed_observations();
			vector_iterator_type obss_end = obss.end();

			for (vector_iterator_type it = obss.begin();
				it != obss_end;
				++it)
			{
				// Recursively call this method in order to collect
				// observations for replication size detection or for
				// sample accumulation
				this->operator()(it->first, it->second);
			}

			DCS_DEBUG_TRACE("(" << this << ") Already consumed observations means took back.");

			// Reset replication size detector to save memory
			repl_size_detector_.reset();

//					// Adjust max number of observation so that to take the
//					// minimum between the value set by the user and the value
//					// obtained from replication size detector.
//					max_num_obs_ = ::std::min(max_num_obs_, repl_size_);
		}
		else if (repl_size_detector_.aborted())
		{
			DCS_DEBUG_TRACE("(" << this << ") Replication size detection has been aborted. Disabling statistical analysis.");

			::std::clog << "[Warning] Statistic '" << *this << "' will be disabled: replication size detection has been aborted." << ::std::endl;

			this->enable(false);
		}
	}


	private: value_type do_estimate() const
	{
		return repl_mean_stat_.estimate();
	}


	private: void do_estimate(value_type replicate_mean)
	{
		repl_mean_stat_(replicate_mean);

		DCS_DEBUG_TRACE(
			"(" << this << ") [Replication #" << actual_num_replications() << "]"
			<< " Stat: " << this->name()
			<< " -- Replicate Mean: " << replicate_mean
			<< " -- Estimate: " << this->estimate()
			<< " +/- " << this->standard_deviation()
			<< " (r.e. " << this->relative_precision()*100 << "%)"
		);

		bool prec_reached = false;

		if (num_repl_detected_
			&& actual_num_replications() >= num_repl_)
		{
			prec_reached = this->target_precision_reached();
		}

		if (!num_repl_detected_
			|| (actual_num_replications() >= num_repl_ && !prec_reached))
		{
			// Number of replications is still to be detected...
			// ... Or need to redetect since, after having performed the
			//        detected number of replications, precision has not been reached.

			DCS_DEBUG_TRACE("(" << this << ") Detecting number of replications ...");

			num_repl_detected_ = num_repl_detector_.detect(
									this->actual_num_replications(),
									this->estimate(),
									this->standard_deviation()
				);

			if (num_repl_detected_)
			{
				if (num_repl_ < num_repl_detector_.estimated_number())
				{
					num_repl_ = ::std::max(num_repl_detector_.estimated_number(), min_num_repl_);
				}
				else if (num_repl_ <= this->actual_num_replications()
						 && !prec_reached)
				{
					// Ooops! The new detected number of replications is the
					// same of or greater than the one previously detected
					// AND we have already performed this number of replications
					// AND precision has not been reached yet.
					// This means that we are unable to reach the target
					// precision. So disable this statistic.

					::std::clog << "[Warning] Statistic '" << *this << "' will be disabled: unable to reach the wanted precision." << ::std::endl;

					this->enable(false);
				}

				DCS_DEBUG_TRACE("(" << this << ") Detected number of replications: " << num_repl_ << " (already performed: " << this->actual_num_replications() << ")");

			}
			else if (num_repl_detector_.aborted())
			{
				::std::clog << "[Warning] Statistic '" << *this << "' will be disabled: number of replications detection has been aborted." << ::std::endl;

				this->enable(false);
			}
		}

#ifdef DCS_DEBUG
		//if (this->relative_precision() <= this->target_relative_precision())
		if (prec_reached)
		{
			DCS_DEBUG_TRACE("(" << this << ") [Replication #" << this->actual_num_replications() << "] Detected precision: mean = " << this->estimate() << " - reached precision = " << this->relative_precision() << " - target precision: " << this->target_relative_precision()); 
		}
		else
		{
			if (this->actual_num_replications() >= num_repl_)
			{
				DCS_DEBUG_TRACE("(" << this << ") [Replication #" << this->actual_num_replications() << "] Failed to detect precision: mean = " << this->estimate() << " - reached precision = " << this->relative_precision() << " - target precision: " << this->target_relative_precision()); 
			}
			else
			{
				DCS_DEBUG_TRACE("(" << this << ") [Replication #" << this->actual_num_replications() << "] Failed to detect precision: not enough replications (done: " << this->actual_num_replications() << " - needed: " << num_repl_ << ")"); 
			}
		}
#endif // DCS_DEBUG
	}


	private: uint_type do_max_num_observations() const
	{
		return max_num_obs_;
	}


	private: value_type do_half_width() const
	{
		return repl_mean_stat_.half_width();
	}


	private: uint_type do_num_observations() const
	{
		return repl_mean_stat_.num_observations();
	}


	private: bool do_observation_complete() const
	{
		return replication_done();
	}


	private: value_type do_relative_precision() const
	{
		return repl_mean_stat_.relative_precision();
	}


	private: void do_reset()
	{
		reset_for_replication();

//[FIXME] 2011-05-22
		// NOTE: Currently the underlying statistic is not explicitly reset
		//       inside 'reset_for_replication' since this method is also used
		//       by the 'end-of-replication' event handler (see there for more
		//       information).
		stat_.reset();
//[/FIXME] 2011-05-22
		repl_mean_stat_.reset();

		// Calling the 'detected' method instead of setting to 'false' allows
		// 'dummy' (e.g., the one with a zero size value) detector to work
		// properly.
		//num_repl_detected_ = false;
		num_repl_detector_.reset();
		num_repl_detected_ = num_repl_detector_.detected();
		if (num_repl_detected_)
		{
			num_repl_ = ::std::max(min_num_repl_, num_repl_detector_.estimated_number());
		}
		this->enable(true);
	}


	private: bool do_steady_state_entered() const
	{
		return trans_detected_;
	}


	private: value_type do_steady_state_enter_time() const
	{
		return steady_start_time_;
	}


	private: void do_steady_state_enter_time(value_type value)
	{
		steady_start_time_ = value;
	}

	private: bool do_transient_phase_detected() const
	{
		return trans_detected_;
	}


	private: uint_type do_transient_phase_length() const
	{
		return trans_len_;
	}


	private: value_type do_variance() const
	{
		return repl_mean_stat_.variance();
	}


	private: ::std::string do_name() const
	{
		return stat_.name();
	}


	private: void do_refresh()
	{
//::std::cerr << "[replications::analyzable_statistic] (" << this << ") BEGIN REFRESH - repl-size-detector: <" << repl_size_detector_.print() << "> - repl_size_detected_: " << repl_size_detected_ << " - repl_size_: " << repl_size_ << ::std::endl;///XXX
//		if (!trans_detected_)
//		{
//			trans_detected_ = trans_detector_.detected();
//			if (trans_detected_)
//			{
//				trans_len_ = trans_detector_.estimated_size();
//			}
			transient_detection();
//		}
//		if (!repl_size_detected_)
//		{
//			repl_size_detected_ = repl_size_detector_.detected();
//			if (repl_size_detected_)
//			{
//				repl_size_ = ::std::min(max_num_obs_, repl_size_detector_.estimated_size());
//			}
			replication_size_detection();
//		}
//FIXME
//		if (!num_repl_detected_)
//		{
//			num_repl_detected_ = num_repl_detector_.detected();
//			if (num_repl_detected_)
//			{
//				num_repl_ = ::std::max(min_num_repl_, num_repl_detector_.estimated_number());
//			}
//		}
//::std::cerr << "[replications::analyzable_statistic] (" << this << ") END REFRESH - repl-size-detector: <" << repl_size_detector_.print() << "> - repl_size_detected_: " << repl_size_detected_ << " - repl_size_: " << repl_size_ << ::std::endl;///XXX
	}


	//@} Interface methods


	//@{ Data members

	private: statistic_type stat_;
	private: transient_phase_detector_type trans_detector_;
	private: replication_size_detector_type repl_size_detector_;
	private: num_replications_detector_type num_repl_detector_;
	/// The target relative precision
	private: /*const*/ uint_type min_num_repl_;
	private: /*const*/ uint_type max_num_obs_;
//	/// The reached relative precision
//	private: value_type rel_prec_;
	private: bool trans_detected_;
	private: uint_type trans_len_;
	private: bool repl_size_detected_;
	private: uint_type repl_size_;
	private: bool num_repl_detected_;
	private: uint_type num_repl_;
	/// If \c true, the statistic is not updated anymore.
//	private: value_type repl_mean_;
	private: mean_estimator<value_type,uint_type> repl_mean_stat_;
	private: value_type steady_start_time_;

	//@} Data members
};


//template <
//	typename StatisticT,
//	typename TransientDetectorT,
//	typename ReplicationSizeDetectorT,
//	typename NumReplicationsDetectorT
//>
//const typename StatisticT::value_type analyzable_statistic<StatisticT,TransientDetectorT,ReplicationSizeDetectorT,NumReplicationsDetectorT>::default_confidence_level = base_statistic<typename StatisticT::value_type, typename StatisticT::uint_type>::default_confidence_level;

template <
	typename StatisticT,
	typename TransientDetectorT,
	typename ReplicationSizeDetectorT,
	typename NumReplicationsDetectorT
>
const typename StatisticT::uint_type analyzable_statistic<StatisticT,TransientDetectorT,ReplicationSizeDetectorT,NumReplicationsDetectorT>::default_max_num_obs = base_analyzable_statistic<typename StatisticT::value_type,typename StatisticT::uint_type>::num_observations_infinity;

}}} // Namespace dcs::des::replications


#endif // DCS_DES_REPLICATIONS_ANALYZABLE_STATISTIC_HPP
