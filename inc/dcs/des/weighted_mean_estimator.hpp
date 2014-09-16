/**
 * \file dcs/des/weighted_weighted_mean_estimator.hpp
 *
 * \brief Weighted mean estimator for independent and identically distributed samples.
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

#ifndef DCS_DES_WEIGHTED_MEAN_ESTIMATOR_HPP
#define DCS_DES_WEIGHTED_MEAN_ESTIMATOR_HPP


#include <cmath>
#include <cstdlib>
#include <dcs/debug.hpp>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/statistic_categories.hpp>
#include <dcs/math/constants.hpp>
#include <dcs/math/stats/distribution/students_t.hpp>
#include <dcs/math/stats/function/quantile.hpp>
#include <dcs/math/function/sqr.hpp>
#include <iostream>
#include <string>


namespace dcs { namespace des {

/**
 * \brief Weighted ean estimator for independent and identically distributed
 *  samples.
 *
 * The (sample) weighted mean is defined as:
 * \f[
 *   \bar{x}_w = \frac{\sum w_i x_i}{\sum w_i}
 * \f]
 * For the associated (sample) weighted variance, there are several definitions:
 * -# Directly use the definition Var[x] = Moment[x,2]-Mean[x]^2:
 *  \f[
 *   s^2_w = \frac{\sum w_i x_i^2}{\sum w_i} - \bar{x}_w^2
 *  \f]
 *  This is a biased estimator since it derives from the formula:
 *  \f[
 *    \s^2_w = \frac{\sum w_i \left(x_i-\bar{x}_w\right)^2}{\sum w_i} 
 *  \f]
 * -# Directly use the definition Var[x] = Moment[x,2]-Mean[x]^2:
 *  \f[
 *   s^2_n = \frac{\sum w_i x_i^2}{\sum w_i} - \left(\frac{\sum w_i x_i}{\sum w_i}\right)^2
 *  \f]
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename ValueT, typename UIntT>
class weighted_mean_estimator: public base_statistic<ValueT,UIntT>
{
	private: typedef base_statistic<ValueT,UIntT> base_type;
	public: typedef ValueT value_type;
	public: typedef UIntT uint_type;
	public: typedef mean_statistic_category category_type;


	public: explicit weighted_mean_estimator(value_type ci_level = base_type::default_confidence_level)
	: base_type(ci_level, "Weighted Mean"),
	  count_(0),
	  m_(0),
	  s2_(0),
	  sumw_(0)
	{
		// Empty
	}


	private: statistic_category do_category() const
	{
		return mean_statistic;
	}


	private: void do_collect(value_type obs, value_type weight)
	{

		++count_;

//		// The following algorithm is taken from boost::accumulators
//		ws_ += weight;
//		m1_ = (m1_*(ws_-weight) + obs*weight) / ws_; // Use the new value of ws_!
//		if (count_ > 1)
//		{
//			s2_ = s2_*(ws_-weight)/ws_ + ::dcs::math::sqr(obs-m1_)*weight/(ws_-weight); // Use the new value of m1_!
//		}
		// The following algorithm should be more numerically stable than the above
		value_type q(obs-m_);
		sumw_ += weight;
		m_ += q*weight/sumw_;
		if (count_ > 1)
		{
			s2_ += weight*q*(obs-m_);
		}
	}


	private: value_type do_estimate() const
	{
		return m_;
	}


	private: value_type do_half_width() const
	{
		if (count_ > 1)
		{
			::dcs::math::stats::students_t_distribution<value_type> t_dist(count_-1);
			value_type t = ::dcs::math::stats::quantile(t_dist, (value_type(1)+this->confidence_level())/value_type(2));

			//return ::std::sqrt(this->variance()/count_)*t;
			return t*(this->standard_deviation()/::std::sqrt(count_));
		}

		//return value_type/*zero*/();
		return ::dcs::math::constants::infinity<value_type>::value;
	}


	private: uint_type do_num_observations() const
	{
		return count_;
	}


	private: value_type do_relative_precision() const
	{
		// Note: the requirements that the estimate is different from zero is
		// justified by the fact that any error, however small, is infinitely
		// large compared with zero.
		return (this->estimate() != 0 && count_ > 1)
				? (this->half_width() / ::std::abs(this->estimate()))
				: ::dcs::math::constants::infinity<value_type>::value;
	}


	private: void do_reset()
	{
		m_	= s2_
			= sumw_
			= value_type(0);
		count_ = uint_type(0);
	}


//	private: value_type standard_deviation() const
//	{
//		return ::std::sqrt(variance());
//	}


	private: value_type do_variance() const
	{
		return (count_ > 1)
				? s2_/sumw_
				: ::dcs::math::constants::infinity<value_type>::value;
	}


	private: uint_type count_; ///< Number of observations seen to date.
	private: value_type m_; ///< Accumulator for the weighted mean.
	private: value_type s2_; ///< Accumulator for the weighted variance.
	private: value_type sumw_; ///< Accumulator for the sum of weights
}; // weighted_mean_estimator


}} // Namespace dcs::des


#endif // DCS_DES_WEIGHTED_MEAN_ESTIMATOR_HPP
