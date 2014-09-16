/**
 * \file dcs/des/batch_means/analyzable_statistic.hpp
 *
 * \brief Output statistic analyzed according to the Batch Means method.
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

#ifndef DCS_DES_BATCH_MEANS_ANALYZABLE_STATISTIC_HPP
#define DCS_DES_BATCH_MEANS_ANALYZABLE_STATISTIC_HPP


#include <cmath>
#include <cstdlib>
#include <dcs/debug.hpp>
#include <dcs/des/base_analyzable_statistic.hpp>
#include <dcs/des/batch_means/pawlikowski1990_batch_size_detector.hpp>
#include <dcs/des/spectral/pawlikowski1990_transient_detector.hpp>
#include <dcs/des/statistic_categories.hpp>
#include <dcs/des/weighted_mean_estimator.hpp>
#include <dcs/math/constants.hpp>
#include <dcs/math/stats/distribution/students_t.hpp>
#include <dcs/math/stats/function/quantile.hpp>
#include <dcs/math/traits/float.hpp>
#include <stdexcept>
#include <vector>


namespace dcs { namespace des { namespace batch_means {

/**
 * \brief Output statistic analyzed according to the Batch Means method.
 *
 * \tparam StatisticT The type of the output statistic under analysis.
 * \tparam TransientDetectorT The type of the transient phase detector.
 * \tparam BatchSizeDetectorT The type of the batch size detector.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <
	typename StatisticT,
	typename TransientDetectorT=::dcs::des::spectral::pawlikowski1990_transient_detector<typename StatisticT::value_type, typename StatisticT::uint_type>,
	typename BatchSizeDetectorT=pawlikowski1990_batch_size_detector<typename StatisticT::value_type, typename StatisticT::uint_type>
>
class analyzable_statistic: public base_analyzable_statistic<
									typename StatisticT::value_type,
									typename StatisticT::uint_type
							>
{
	public: typedef StatisticT statistic_type;
	public: typedef typename statistic_type::value_type value_type;
	public: typedef typename statistic_type::uint_type uint_type;
	public: typedef TransientDetectorT transient_phase_detector_type;
	public: typedef BatchSizeDetectorT batch_size_detector_type;
//	public: typedef typename statistic_type::category_type category_type;
	private: typedef base_analyzable_statistic<value_type,uint_type> base_type;


	public: static const value_type default_confidence_level;// = 0.95;
	public: static const uint_type default_min_num_batches = 10;
	public: static const bool default_use_schmeiser_rule = false;
	public: static const uint_type default_schmeiser_rule_batch_size = 30;
	public: static const uint_type default_max_num_obs;// = base_type::num_observations_infinity;
	public: static const value_type default_half_width;// = ::dcs::math::constants::infinity<value_type>::value;


	/// A constructor.
	public: explicit analyzable_statistic(value_type relative_precision = base_type::default_target_relative_precision,
										  uint_type max_num_obs=default_max_num_obs,
										  uint_type min_num_batches=default_min_num_batches)
	: base_type(relative_precision),
	  stat_(),
	  trans_detector_(),
	  size_detector_(),
	  min_num_batches_(min_num_batches),
	  max_num_obs_(max_num_obs),
	  use_schmeiser_rule_(default_use_schmeiser_rule),
	  k_b0_(default_schmeiser_rule_batch_size),
	  //rel_prec_(default_relative_precision),
	  count_(0),
	  half_width_(default_half_width),
	  trans_detected_(false),
	  trans_len_(0),
	  batch_size_detected_(false),
	  batch_size_(0),
	  steady_start_time_(0)
	{
	}


	/**
	 * \brief A constructor.
	 *
	 * \param stat The statistic to be monitored.
	 * \param transient_detector The transient phase detector
	 *  (model of TransientDetector concept).
	 * \param batch_size_detector The batch size detector
	 *  (model of BatchSizeDetector concept).
	 * \param relative_precision The target relative precision of the confidence
	 *  interval.
	 * \param min_num_batches The minimum number of batches to collect before
	 *  checking for relative precision.
	 */
	public: analyzable_statistic(statistic_type const& stat,
								 transient_phase_detector_type const& transient_detector,
								 batch_size_detector_type const& size_detector,
								 value_type relative_precision = base_type::default_target_relative_precision,
								 uint_type max_num_obs = default_max_num_obs,
								 uint_type min_num_batches =  default_min_num_batches)
	: base_type(relative_precision),
	  stat_(stat),
	  trans_detector_(transient_detector),
	  size_detector_(size_detector),
	  min_num_batches_(min_num_batches),
	  max_num_obs_(max_num_obs),
	  use_schmeiser_rule_(default_use_schmeiser_rule),
	  k_b0_(default_schmeiser_rule_batch_size),
	  //rel_prec_(default_relative_precision),
	  count_(0),
	  half_width_(default_half_width),
	  trans_detected_(false),
	  trans_len_(0),
	  batch_size_detected_(false),
	  batch_size_(0),
	  steady_start_time_(0)
	{
	}


	public: void enable_schmeiser_rule(uint_type schmeiser_rule_batch_size=default_schmeiser_rule_batch_size)
	{
		use_schmeiser_rule_ = true;
		k_b0_ = schmeiser_rule_batch_size;
	}


	public: void disable_schmeiser_rule()
	{
		use_schmeiser_rule_ = false;
	}


	public: bool batch_size_detected() const
	{
		return batch_size_detected_;
	}


	public: uint_type batch_size() const
	{
		return batch_size_;
	}


	public: uint_type actual_batch_size() const
	{
		return batch_size_-(count_ % batch_size_);
	}


	public: uint_type num_batches() const
	{
		return stat_.num_observations();
	}


	public: bool batch_done() const
	{
		return batch_size_detected_ && actual_batch_size() == batch_size();
	}


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
		if (!this->enabled())
		{
			DCS_DEBUG_TRACE("Statistical Analysis Disabled!");

			return;
		}

		++count_;

		DCS_DEBUG_TRACE("[Observation #" << count_ << " -- MAX: " << max_num_obs_ << "] Observation: " << obs << " - Weight: " << weight);

		if (max_num_obs_ != base_type::num_observations_infinity && count_ >= max_num_obs_)
		{
			DCS_DEBUG_TRACE("Reached maximum number of observations: " << count_ << "/" << max_num_obs_);
			this->enable(false);
			return;
		}

		if (batch_size_detected_)
		{
			// Collect another batch 

			if ((count_ % batch_size_) == 0)
			{
				//batch_mean_ /= batch_size_;
				//do_estimate(batch_mean_);
				//batch_mean_ = 0;
				do_estimate(batch_mean_.estimate());
				batch_mean_.reset();
			}
			else
			{
				//batch_mean_ += obs;
				batch_mean_(obs);
			}
		}
		else if (trans_detected_)
		{
			// Transient detected but batch size is still to be detected.

			DCS_DEBUG_TRACE("Detecting batch size...");//XXX

			batch_size_detected_ = size_detector_.detect(obs, weight);
			if (batch_size_detected_)
			{
				batch_size_ = size_detector_.estimated_size();

				DCS_DEBUG_TRACE("Detected batch size. Taking back " << size_detector_.computed_estimators().size() << " batch means computed during batch size detection."); 

				// Batch size just detected.
				// Takes all of the batch means computed during the batch
				// size detection.

				typedef typename batch_size_detector_type::vector_type vector_type;
				typedef typename vector_type::const_iterator vector_iterator_type;

				vector_type means = size_detector_.computed_estimators();
				vector_iterator_type means_end = means.end();
				for (
					vector_iterator_type it = means.begin();
					it != means_end;
					++it
				) {
					do_estimate(*it);
				}

				DCS_DEBUG_TRACE("Already computed batch means took back."); 

				// Reset batch size detector to save memory
				size_detector_.reset();
			}
			else if (size_detector_.aborted())
			{
//				throw ::std::runtime_error("Batch size detection is done but batch size not detected.");
				DCS_DEBUG_TRACE("Batch size detection has been aborted. Disabling statistical analysis.");

				this->enable(false);
			}
		}
		else
		{
			// Detect transient phase

			DCS_DEBUG_TRACE("Detecting transient phase...");//XXX

			trans_detected_ = trans_detector_.detect(obs, weight);

			if (trans_detected_)
			{
				trans_len_ = trans_detector_.estimated_size();

				DCS_DEBUG_TRACE("Detected transient phase. Putting-back " << trans_detector_.steady_state_observations().size() << " safe steady-state observations."); 

				// Transient phase just detected
				// Put back steady-state observations possibly used for
				// transient phase detection
				typedef typename transient_phase_detector_type::sample_container vector_type;
				typedef typename vector_type::const_iterator vector_iterator_type;

				vector_type obs = trans_detector_.steady_state_observations();
				vector_iterator_type obs_end = obs.end();

				// Decrement counter since it is already been incremented
				// during transient detection.
				// This prevent to count twice observations already seen
				// during transient detection.
				count_ -= obs.size();

				for (
					vector_iterator_type it = obs.begin();
					it != obs_end;
					++it
				) {
					// Recursively call this method in order to collect
					// observations for batch size detection or for
					// sample accumulation
					this->operator()(it->first, it->second);
				}

				DCS_DEBUG_TRACE("Safe steady-state observations put back."); 

				// Reset transient detector to save memory
				trans_detector_.reset();
			}
			else if (trans_detector_.aborted())
			{
//				throw ::std::runtime_error("Transient phase is done but transient period not detected.");
				DCS_DEBUG_TRACE("Transient phase detection has been aborted. Disabling statistical analysis.");

				this->enable(false);
			}
		}
	}


	private: void do_reset()
	{
		stat_.reset();

		trans_detector_.reset();

		size_detector_.reset();

		rel_prec_ = ::dcs::math::constants::infinity<value_type>::value;

		this->enable(true);

		trans_detected_ = batch_size_detected_
					    = false;

		count_ = trans_len_
			   = batch_size_
			   = uint_type(0);

//		batch_mean_ = half_width_
//					= value_type(0);
		batch_mean_.reset();
		half_width_ = value_type(0);

		batch_means_.clear();
	}


//	public: transient_phase_detector_type const& transient_phase_detector() const
//	{
//		return trans_detector_;
//	}


//	public: batch_size_detector_type const& batch_size_detector() const
//	{
//		return size_detector_;
//	}


	private: uint_type do_num_observations() const
	{
		return count_;
	}


	private: value_type do_estimate() const
	{
		return stat_.estimate();
	}


	private: value_type do_variance() const
	{
		return stat_.variance()/value_type(num_batches());
	}


//	private: value_type do_standard_deviation() const
//	{
//		return ::std::sqrt(variance());
//	}


	private: value_type do_half_width() const
	{
		return half_width_;
	}


	private: value_type do_relative_precision() const
	{
		return rel_prec_;
	}


	private: uint_type do_max_num_observations() const
	{
		return max_num_obs_;
	}


//	private: bool do_transient_phase_detected() const
//	{
//		return trans_detected_;
//	}


	private: uint_type do_transient_phase_length() const
	{
		return trans_len_;
	}


	private: bool do_steady_state_entered() const
	{
		//return trans_detected_ && batch_size_detected_;
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


	private: bool do_observation_complete() const
	{
		return batch_done();
	}

	/**
	 * \brief Estimate the grand-mean given the new batch mean.
	 * \param batch_mean The new batch mean.
	 */
	private: void do_estimate(value_type batch_mean)
	{
		stat_(batch_mean);

		DCS_DEBUG_TRACE("[Batch #" << num_batches() << "] Batch Mean: " << batch_mean);


		if (num_batches() > 1 && num_batches() >= min_num_batches_)
		{
			::dcs::math::stats::students_t_distribution<value_type> t_dist(num_batches()-1);

			half_width_ = this->standard_deviation()
						* ::dcs::math::stats::quantile(t_dist, (value_type(1)+this->confidence_level())/value_type(2));

			// Compute the relative precision
			// Note: the requirements that the estimate is different from zero
			// is for the computation of relative precision and is justified by
			// the fact that any error, however small, is infinitely large
			// compared with zero.
			if (this->estimate() != 0)
			{
				// Note: ABS(estimate) is for making sure the precision is
				// positive even for negative estimates.
				rel_prec_ = half_width_ / ::std::abs(this->estimate());
			}
			else
			{
				// In case of zero estimates, set the relative error to infinity
				rel_prec_ = ::dcs::math::constants::infinity<value_type>::value;
			}
		}

#ifdef DCS_DEBUG
		if (rel_prec_ <= this->target_relative_precision())
		{
			DCS_DEBUG_TRACE("[Batch #" << num_batches() << "] Detected precision: mean = " << this->estimate() << " - reached precision = " << rel_prec_ << " - target precision: " << this->target_relative_precision()); 
		}
		else
		{
			DCS_DEBUG_TRACE("[Batch #" << num_batches() << "] Failed to detect precision: mean = " << this->estimate() << " - reached precision = " << rel_prec_ << " - target precision: " << this->target_relative_precision()); 
		}
#endif // DCS_DEBUG

		bool prec_reached = this->target_precision_reached();

		/// This applies the recommendations found in (Schmeiser,1982)
		if (!prec_reached
			&& use_schmeiser_rule_
			&& (k_b0_ > 0))
		{
			batch_means_.push_back(batch_mean);

			if ((num_batches() % k_b0_) == 0)
			{
				// Additional test for precision by consolidating k_be 
				// observations into k_b0 means of longer batches
				// (see (Schmeiser, 1982)

				uint_type k_be = num_batches();
				uint_type m = k_be / k_b0_;

				value_type grand_mean = 0;
				value_type grand_sd = 0;

				uint_type i = 0;
				for (uint_type j = 0; j < k_b0_; ++j)
				{
					value_type sum = 0;

					for (uint_type r = 0; r < m; ++r)
					{
						//sum += obs_[j*m+r];
						sum += batch_means_[i];
						++i;
					}

					// Update the grand-mean and the grand-standard-deviation
					value_type mean = sum/value_type(m);
					value_type delta = mean - grand_mean;
					grand_mean += delta/value_type(j+1);
					grand_sd += delta*(mean-grand_mean);
				}
				if (k_b0_ > 1)
				{
					grand_sd = ::std::sqrt(grand_sd/value_type(k_b0_-1));

					::dcs::math::stats::students_t_distribution<value_type> t_dist(k_b0_-1);
					half_width_ = grand_sd
								* ::dcs::math::stats::quantile(t_dist, (value_type(1)+this->confidence_level())/value_type(2));
					// Recompute precision and check again
					if (grand_mean != 0)
					{
						rel_prec_ = half_width_ / ::std::abs(grand_mean); 
					}
					else
					{
						// In case of zero estimates, set the relative error to infinity
						rel_prec_ = ::dcs::math::constants::infinity<value_type>::value;
					}
				}

				bool prec_reached = this->target_precision_reached();
#ifdef DCS_DEBUG
				if (prec_reached)
				{
					DCS_DEBUG_TRACE("[Batch #" << k_be << "] Detected precision through Schmeiser trick: grand mean = " << grand_mean << " - reached precision = " << rel_prec_ << " - target precision: " << this->target_relative_precision()); 
				}
				else
				{
					DCS_DEBUG_TRACE("[Batch #" << k_be << "] Failed to detect precision through Schmeiser trick: grand mean = " << grand_mean << " - reached precision = " << rel_prec_ << " - target precision: " << this->target_relative_precision()); 
				}
#endif // DCS_DEBUG
			}
		}
	}


	private: virtual ::std::string do_name() const
	{
		return stat_.name();
	}


	private: statistic_type stat_;
	private: transient_phase_detector_type trans_detector_;
	private: batch_size_detector_type size_detector_;
	/// The target relative precision
	private: /*const*/ uint_type min_num_batches_;
	private: /*const*/ uint_type max_num_obs_;
	private: /*const*/ bool use_schmeiser_rule_;
	private: /*const*/ uint_type k_b0_;
	/// The reached relative precision
	private: value_type rel_prec_;
	/// The total number of observations (this is different from the number of
	/// batch means).
	private: uint_type count_;
	/// The half-wdith of the confidence interval at ci_level_ confidence level.
	private: value_type half_width_;
	private: bool trans_detected_;
	private: uint_type trans_len_;
	private: bool batch_size_detected_;
	private: uint_type batch_size_;
	//private: value_type batch_mean_;
	private: weighted_mean_estimator<value_type,uint_type> batch_mean_;
	private: ::std::vector<value_type> batch_means_;
	private: value_type steady_start_time_;
};

template <
	typename StatisticT,
	typename TransientDetectorT,
	typename BatchSizeDetectorT
>
const typename StatisticT::uint_type analyzable_statistic<StatisticT,TransientDetectorT,BatchSizeDetectorT>::default_max_num_obs = base_analyzable_statistic<typename StatisticT::value_type,typename StatisticT::uint_type>::num_observations_infinity;

template <
	typename StatisticT,
	typename TransientDetectorT,
	typename BatchSizeDetectorT
>
const typename StatisticT::value_type analyzable_statistic<StatisticT,TransientDetectorT,BatchSizeDetectorT>::default_half_width = ::dcs::math::constants::infinity<typename StatisticT::value_type>::value;

}}} // Namespace dcs::des::batch_means


#endif // DCS_DES_BATCH_MEANS_ANALYZABLE_STATISTIC_HPP
